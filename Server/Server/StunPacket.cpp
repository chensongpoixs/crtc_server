﻿//#define MS_CLASS "RTC::StunPacket"
// #define MS_LOG_DEV_LEVEL 3

#include "StunPacket.hpp"
//#include "Logger.hpp"
//#include "Utils.hpp"
#include <cstdio>  // std::snprintf()
#include <cstring> // std::memcmp(), std::memcpy()
#include "crtc_util.h"
#include "ccrypto_random.h"
#include "cuv_ip.h"
#include "clog.h"




namespace RTC
{
	/* Class variables. */

	const uint8_t StunPacket::magicCookie[] = { 0x21, 0x12, 0xA4, 0x42 };

	/* Class methods. */
	using namespace chen;
	StunPacket* StunPacket::Parse(const uint8_t* data, size_t len)
	{
		//MS_TRACE();
		// 21 12 A4 42 固定的魔法数 0x2112A442
		if (!StunPacket::IsStun(data, len))
		{
			return nullptr;
		}

		/*
		  The message type field is decomposed further into the following
		    structure:

		    0                 1
		    2  3  4 5 6 7 8 9 0 1 2 3 4 5
		       +--+--+-+-+-+-+-+-+-+-+-+-+-+-+
		       |M |M |M|M|M|C|M|M|M|C|M|M|M|M|
		       |11|10|9|8|7|1|6|5|4|0|3|2|1|0|
		       +--+--+-+-+-+-+-+-+-+-+-+-+-+-+

		    Figure 3: Format of STUN Message Type Field

		   Here the bits in the message type field are shown as most significant
		   (M11) through least significant (M0).  M11 through M0 represent a 12-
		   bit encoding of the method.  C1 and C0 represent a 2-bit encoding of
		   the class.
		 */

		// Get type field.
		uint16_t msgType = rtc_byte::get2bytes(data, 0);

		// Get length field.
		uint16_t msgLength = rtc_byte::get2bytes(data, 2);

		// length field must be total size minus header's 20 bytes, and must be multiple of 4 Bytes.
		if ((static_cast<size_t>(msgLength) != len - 20) || ((msgLength & 0x03) != 0))
		{
			//using namespace chen;

			WARNING_EX_LOG("ice, length field + 20 does not match total size (or it is not multiple of 4 bytes),  packet discarded");

			return nullptr;
		}

		// Get STUN method.
		uint16_t msgMethod = (msgType & 0x000f) | ((msgType & 0x00e0) >> 1) | ((msgType & 0x3E00) >> 2);

		// Get STUN class.
		uint16_t msgClass = ((data[0] & 0x01) << 1) | ((data[1] & 0x10) >> 4);

		// Create a new StunPacket (data + 8 points to the received TransactionID field).
		auto* packet = new StunPacket(
		  static_cast<Class>(msgClass), static_cast<Method>(msgMethod), data + 8, data, len);

		/*
		    STUN Attributes

		    After the STUN header are zero or more attributes.  Each attribute
		    MUST be TLV encoded, with a 16-bit type, 16-bit length, and value.
		    Each STUN attribute MUST end on a 32-bit boundary.  As mentioned
		    above, all fields in an attribute are transmitted most significant
		    bit first.

		        0                   1                   2                   3
		        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		       |         Type                  |            Length             |
		       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		       |                         Value (variable)                ....
		       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 */

		// Start looking for attributes after STUN header (Byte #20).
		size_t pos{ 20 };
		// Flags (positions) for special MESSAGE-INTEGRITY and FINGERPRINT attributes. 
		// TODO@chensong 2022-05-11 消息完整性验证
		bool hasMessageIntegrity{ false };
		bool hasFingerprint{ false };
		size_t fingerprintAttrPos; // Will point to the beginning of the attribute.
		uint32_t fingerprint;      // Holds the value of the FINGERPRINT attribute.

		// Ensure there are at least 4 remaining bytes (attribute with 0 length).
		while (pos + 4 <= len)
		{
			// Get the attribute type.
			auto attrType = static_cast<Attribute>(rtc_byte::get2bytes(data, pos));

			// Get the attribute length.
			uint16_t attrLength = rtc_byte::get2bytes(data, pos + 2);

			// Ensure the attribute length is not greater than the remaining size.
			if ((pos + 4 + attrLength) > len)
			{
				using namespace chen;

				WARNING_EX_LOG("ice, the attribute length exceeds the remaining size, packet discarded");

				delete packet;
				return nullptr;
			}

			// FINGERPRINT must be the last attribute.
			if (hasFingerprint)
			{
				WARNING_EX_LOG("ice, attribute after FINGERPRINT is not allowed, packet discarded");

				delete packet;
				return nullptr;
			}

			// After a MESSAGE-INTEGRITY attribute just FINGERPRINT is allowed.
			if (hasMessageIntegrity && attrType != Attribute::FINGERPRINT)
			{
				WARNING_EX_LOG("ice, attribute after MESSAGE-INTEGRITY other than FINGERPRINT is not allowed, packet discarded");

				delete packet;
				return nullptr;
			}

			const uint8_t* attrValuePos = data + pos + 4;

			switch (attrType)
			{
				case Attribute::USERNAME:
				{
					packet->SetUsername( reinterpret_cast<const char*>(attrValuePos), static_cast<size_t>(attrLength));
					DEBUG_EX_LOG("[pos = %u][username = %s]", pos,  packet->GetUsername().c_str());
					break;
				}

				case Attribute::PRIORITY:
				{
					// Ensure attribute length is 4 bytes.
					if (attrLength != 4)
					{
						WARNING_EX_LOG("ice, attribute PRIORITY must be 4 bytes length, packet discarded");

						delete packet;
						return nullptr;
					}
					
					packet->SetPriority(rtc_byte::get4bytes(attrValuePos, 0));
					DEBUG_EX_LOG("[pos = %u][priority =  %u]", pos, packet->GetPriority());
					break;
				}

				case Attribute::ICE_CONTROLLING:
				{
					// Ensure attribute length is 8 bytes.
					if (attrLength != 8)
					{
						WARNING_EX_LOG("ice, attribute ICE-CONTROLLING must be 8 bytes length, packet discarded");

						delete packet;
						return nullptr;
					}
					
					packet->SetIceControlling(rtc_byte::get8bytes(attrValuePos, 0));
					DEBUG_EX_LOG("[pos = %u][IceControlling = %u]", pos, packet->GetIceControlling());
					break;
				}

				case Attribute::ICE_CONTROLLED:
				{
					// Ensure attribute length is 8 bytes.
					if (attrLength != 8)
					{
						WARNING_EX_LOG("ice, attribute ICE-CONTROLLED must be 8 bytes length, packet discarded");

						delete packet;
						return nullptr;
					}
					
					packet->SetIceControlled(rtc_byte::get8bytes(attrValuePos, 0));
					DEBUG_EX_LOG("[pos = %u][iceControlled = %u]", pos,  packet->GetIceControlled());
					break;
				}

				case Attribute::USE_CANDIDATE:
				{
					// Ensure attribute length is 0 bytes.
					if (attrLength != 0)
					{
						WARNING_EX_LOG("ice, attribute USE-CANDIDATE must be 0 bytes length, packet discarded");

						delete packet;
						return nullptr;
					}
					
					packet->SetUseCandidate();
					DEBUG_EX_LOG("[pos = %u][UseCandidate = %u]", pos,  packet->HasUseCandidate());
					break;
				}

				case Attribute::MESSAGE_INTEGRITY:
				{
					// Ensure attribute length is 20 bytes.
					if (attrLength != 20)
					{
						WARNING_EX_LOG("ice, attribute MESSAGE-INTEGRITY must be 20 bytes length, packet discarded");

						delete packet;
						return nullptr;
					}
					
					hasMessageIntegrity = true;
					packet->SetMessageIntegrity(attrValuePos);
					DEBUG_EX_LOG("[pos = %u][MessageIntegrity = %u]", pos, packet->HasMessageIntegrity());
					break;
				}

				case Attribute::FINGERPRINT:
				{
					// Ensure attribute length is 4 bytes.
					if (attrLength != 4)
					{
						WARNING_EX_LOG("ice, attribute FINGERPRINT must be 4 bytes length, packet discarded");

						delete packet;
						return nullptr;
					}
					
					hasFingerprint     = true;
					fingerprintAttrPos = pos;
					fingerprint        = rtc_byte::get4bytes(attrValuePos, 0);
					packet->SetFingerprint();
					DEBUG_EX_LOG("[pos = %u][fingerprint = %u]", pos, fingerprint);
					break;
				}

				case Attribute::ERROR_CODE:
				{
					// Ensure attribute length >= 4bytes.
					if (attrLength < 4)
					{
						WARNING_EX_LOG("ice, attribute ERROR-CODE must be >= 4bytes length, packet discarded");

						delete packet;
						return nullptr;
					}

					uint8_t errorClass  = rtc_byte::get1byte(attrValuePos, 2);
					uint8_t errorNumber = rtc_byte::get1byte(attrValuePos, 3);
					auto errorCode      = static_cast<uint16_t>(errorClass * 100 + errorNumber);
					
					packet->SetErrorCode(errorCode);
					DEBUG_EX_LOG("[pos = %u][errorClass = %u][errorNumber = %u]", pos, errorClass, errorNumber);
					break;
				}

				default:;
			}

			// Set next attribute position.
			pos =
			  static_cast<size_t>(rtc_byte::padto4bytes(static_cast<uint16_t>(pos + 4 + attrLength)));
		}

		// Ensure current position matches the total length.
		if (pos != len)
		{
			WARNING_EX_LOG("ice, computed packet size does not match total size, packet discarded");

			delete packet;
			return nullptr;
		}

		// If it has FINGERPRINT attribute then verify it.
		if (hasFingerprint)
		{
			// Compute the CRC32 of the received packet up to (but excluding) the
			// FINGERPRINT attribute and XOR it with 0x5354554e.
			uint32_t computedFingerprint = s_crypto_random. GetCRC32(data, fingerprintAttrPos) ^ 0x5354554e;

			// Compare with the FINGERPRINT value in the packet.
			if (fingerprint != computedFingerprint)
			{
				WARNING_EX_LOG("ice, computed FINGERPRINT value does not match the value in the packet,  packet discarded");

				delete packet;
				return nullptr;
			}
		}

		return packet;
	}

	/* Instance methods. */

	StunPacket::StunPacket(
	  Class klass, Method method, const uint8_t* transactionId, const uint8_t* data, size_t size)
	  : klass(klass), method(method), transactionId(transactionId), data(const_cast<uint8_t*>(data)),
	    size(size)
	{
		//MS_TRACE();
	}

	StunPacket::~StunPacket()
	{
		//MS_TRACE();
	}

	void StunPacket::Dump() const
	{
		//MS_TRACE();

		DEBUG_LOG("<StunPacket>");

		std::string klass;
		switch (this->klass)
		{
			case Class::REQUEST:
				klass = "Request";
				break;
			case Class::INDICATION:
				klass = "Indication";
				break;
			case Class::SUCCESS_RESPONSE:
				klass = "SuccessResponse";
				break;
			case Class::ERROR_RESPONSE:
				klass = "ErrorResponse";
				break;
		}
		if (this->method == Method::BINDING)
		{
			DEBUG_LOG("  Binding %s", klass.c_str());
		}
		else
		{
			// This prints the unknown method number. Example: TURN Allocate => 0x003.
			DEBUG_LOG("  %s with unknown method %#.3x", klass.c_str(), static_cast<uint16_t>(this->method));
		}
		DEBUG_LOG("  size: %zu bytes", this->size);

		char transactionId[25];

		for (int i{ 0 }; i < 12; ++i)
		{
			// NOTE: n must be 3 because snprintf adds a \0 after printed chars.
			std::snprintf(transactionId + (i * 2), 3, "%.2x", this->transactionId[i]);
		}
		DEBUG_LOG("  transactionId: %s", transactionId);
		if (this->errorCode != 0u)
			DEBUG_LOG("  errorCode: %hu"  , this->errorCode);
		if (!this->username.empty())
			DEBUG_LOG("  username: %s", this->username.c_str());
		if (this->priority != 0u)
			DEBUG_LOG("  priority: %u"  , this->priority);
		if (this->iceControlling != 0u)
			DEBUG_LOG("  iceControlling: %llu"  , this->iceControlling);
		if (this->iceControlled != 0u)
			DEBUG_LOG("  iceControlled: %llu"  , this->iceControlled);
		if (this->hasUseCandidate)
			DEBUG_LOG("  useCandidate");
		if (this->xorMappedAddress != nullptr)
		{
			int family;
			uint16_t port;
			std::string ip;

			uv_ip::GetAddressInfo(this->xorMappedAddress, family, ip, port);

			DEBUG_LOG("  xorMappedAddress: %s : %llu"  , ip.c_str(), port);
		}
		if (this->messageIntegrity != nullptr)
		{
			char messageIntegrity[41];

			for (int i{ 0 }; i < 20; ++i)
			{
				std::snprintf(messageIntegrity + (i * 2), 3, "%.2x", this->messageIntegrity[i]);
			}

			DEBUG_LOG("  messageIntegrity: %s", messageIntegrity);
		}
		if (this->hasFingerprint)
			DEBUG_LOG("  has fingerprint");

		DEBUG_LOG("</StunPacket>");
	}

	StunPacket::Authentication StunPacket::CheckAuthentication(
	  const std::string& localUsername, const std::string& localPassword)
	{
		//MS_TRACE();

		switch (this->klass)
		{
			case Class::REQUEST:
			case Class::INDICATION:
			{
				// Both USERNAME and MESSAGE-INTEGRITY must be present.
				if (!this->messageIntegrity || this->username.empty())
					return Authentication::BAD_REQUEST;

				// Check that USERNAME attribute begins with our local username plus ":".
				size_t localUsernameLen = localUsername.length();

				if (
				  this->username.length() <= localUsernameLen || this->username.at(localUsernameLen) != ':' ||
				  (this->username.compare(0, localUsernameLen, localUsername) != 0))
				{
					return Authentication::UNAUTHORIZED;
				}

				break;
			}
			// This method cannot check authentication in received responses (as we
			// are ICE-Lite and don't generate requests).
			case Class::SUCCESS_RESPONSE:
			case Class::ERROR_RESPONSE:
			{
				ERROR_EX_LOG("cannot check authentication for a STUN response");

				return Authentication::BAD_REQUEST;
			}
		}

		// If there is FINGERPRINT it must be discarded for MESSAGE-INTEGRITY calculation,
		// so the header length field must be modified (and later restored).
		if (this->hasFingerprint)
		{
			// Set the header length field: full size - header length (20) - FINGERPRINT length (8).
			rtc_byte::set2bytes(this->data, 2, static_cast<uint16_t>(this->size - 20 - 8));
		}

		// Calculate the HMAC-SHA1 of the message according to MESSAGE-INTEGRITY rules.
		const uint8_t* computedMessageIntegrity = s_crypto_random. GetHmacSha1(
		  localPassword, this->data, (this->messageIntegrity - 4) - this->data);

		Authentication result;

		// Compare the computed HMAC-SHA1 with the MESSAGE-INTEGRITY in the packet.
		if (std::memcmp(this->messageIntegrity, computedMessageIntegrity, 20) == 0)
		{
			result = Authentication::OK;
		}
		else
		{
			result = Authentication::UNAUTHORIZED;
		}

		// Restore the header length field.
		if (this->hasFingerprint)
		{
			rtc_byte::set2bytes(this->data, 2, static_cast<uint16_t>(this->size - 20));
		}

		return result;
	}

	StunPacket* StunPacket::CreateSuccessResponse()
	{
		//MS_TRACE();

		/*MS_ASSERT(
		  this->klass == Class::REQUEST,
		  "attempt to create a success response for a non Request STUN packet");*/

		return new StunPacket(Class::SUCCESS_RESPONSE, this->method, this->transactionId, nullptr, 0);
	}

	StunPacket* StunPacket::CreateErrorResponse(uint16_t errorCode)
	{
		//MS_TRACE();

		///*MS_ASSERT(
		//  this->klass == Class::REQUEST,
		//  "attempt to create an error response for a non Request STUN packet");*/

		auto* response =
		  new StunPacket(Class::ERROR_RESPONSE, this->method, this->transactionId, nullptr, 0);

		response->SetErrorCode(errorCode);

		return response;
	}

	void StunPacket::Authenticate(const std::string& password)
	{
		// Just for Request, Indication and SuccessResponse messages.
		if (this->klass == Class::ERROR_RESPONSE)
		{
			ERROR_EX_LOG("cannot set password for ErrorResponse messages");

			return;
		}

		this->password = password;
	}

	void StunPacket::Serialize(uint8_t* buffer)
	{
		//MS_TRACE();

		// Some useful variables.
		uint16_t usernamePaddedLen{ 0 };
		uint16_t xorMappedAddressPaddedLen{ 0 };
		bool addXorMappedAddress =
		  ((this->xorMappedAddress != nullptr) && this->method == StunPacket::Method::BINDING &&
		   this->klass == Class::SUCCESS_RESPONSE);
		bool addErrorCode        = ((this->errorCode != 0u) && this->klass == Class::ERROR_RESPONSE);
		bool addMessageIntegrity = (this->klass != Class::ERROR_RESPONSE && !this->password.empty());
		bool addFingerprint{ true }; // Do always.

		// Update data pointer.
		this->data = buffer;

		// First calculate the total required size for the entire packet.
		this->size = 20; // Header.

		if (!this->username.empty())
		{
			usernamePaddedLen = rtc_byte::padto4bytes(static_cast<uint16_t>(this->username.length()));
			this->size += 4 + usernamePaddedLen;
		}

		if (this->priority != 0u)
			this->size += 4 + 4;

		if (this->iceControlling != 0u)
			this->size += 4 + 8;

		if (this->iceControlled != 0u)
			this->size += 4 + 8;

		if (this->hasUseCandidate)
			this->size += 4;

		if (addXorMappedAddress)
		{
			switch (this->xorMappedAddress->sa_family)
			{
				case AF_INET:
				{
					xorMappedAddressPaddedLen = 8;
					this->size += 4 + 8;

					break;
				}

				case AF_INET6:
				{
					xorMappedAddressPaddedLen = 20;
					this->size += 4 + 20;

					break;
				}

				default:
				{
					ERROR_EX_LOG("invalid inet family in XOR-MAPPED-ADDRESS attribute");

					addXorMappedAddress = false;
				}
			}
		}

		if (addErrorCode)
			this->size += 4 + 4;

		if (addMessageIntegrity)
			this->size += 4 + 20;

		if (addFingerprint)
			this->size += 4 + 4;

		// Merge class and method fields into type.
		uint16_t typeField = (static_cast<uint16_t>(this->method) & 0x0f80) << 2;

		typeField |= (static_cast<uint16_t>(this->method) & 0x0070) << 1;
		typeField |= (static_cast<uint16_t>(this->method) & 0x000f);
		typeField |= (static_cast<uint16_t>(this->klass) & 0x02) << 7;
		typeField |= (static_cast<uint16_t>(this->klass) & 0x01) << 4;

		// Set type field.
		rtc_byte::set2bytes(buffer, 0, typeField);
		// Set length field.
		rtc_byte::set2bytes(buffer, 2, static_cast<uint16_t>(this->size) - 20);
		// Set magic cookie.
		std::memcpy(buffer + 4, StunPacket::magicCookie, 4);
		// Set TransactionId field.
		std::memcpy(buffer + 8, this->transactionId, 12);
		// Update the transaction ID pointer.
		this->transactionId = buffer + 8;
		// Add atributes.
		size_t pos{ 20 };

		// Add USERNAME.
		if (usernamePaddedLen != 0u)
		{
			rtc_byte::set2bytes(buffer, pos, static_cast<uint16_t>(Attribute::USERNAME));
			rtc_byte::set2bytes(buffer, pos + 2, static_cast<uint16_t>(this->username.length()));
			std::memcpy(buffer + pos + 4, this->username.c_str(), this->username.length());
			pos += 4 + usernamePaddedLen;
		}

		// Add PRIORITY.
		if (this->priority != 0u)
		{
			rtc_byte::set2bytes(buffer, pos, static_cast<uint16_t>(Attribute::PRIORITY));
			rtc_byte::set2bytes(buffer, pos + 2, 4);
			rtc_byte::set4bytes(buffer, pos + 4, this->priority);
			pos += 4 + 4;
		}

		// Add ICE-CONTROLLING.
		if (this->iceControlling != 0u)
		{
			rtc_byte::set2bytes(buffer, pos, static_cast<uint16_t>(Attribute::ICE_CONTROLLING));
			rtc_byte::set2bytes(buffer, pos + 2, 8);
			rtc_byte::set8bytes(buffer, pos + 4, this->iceControlling);
			pos += 4 + 8;
		}

		// Add ICE-CONTROLLED.
		if (this->iceControlled != 0u)
		{
			rtc_byte::set2bytes(buffer, pos, static_cast<uint16_t>(Attribute::ICE_CONTROLLED));
			rtc_byte::set2bytes(buffer, pos + 2, 8);
			rtc_byte::set8bytes(buffer, pos + 4, this->iceControlled);
			pos += 4 + 8;
		}

		// Add USE-CANDIDATE.
		if (this->hasUseCandidate)
		{
			rtc_byte::set2bytes(buffer, pos, static_cast<uint16_t>(Attribute::USE_CANDIDATE));
			rtc_byte::set2bytes(buffer, pos + 2, 0);
			pos += 4;
		}

		// Add XOR-MAPPED-ADDRESS
		if (addXorMappedAddress)
		{
			rtc_byte::set2bytes(buffer, pos, static_cast<uint16_t>(Attribute::XOR_MAPPED_ADDRESS));
			rtc_byte::set2bytes(buffer, pos + 2, xorMappedAddressPaddedLen);

			uint8_t* attrValue = buffer + pos + 4;

			switch (this->xorMappedAddress->sa_family)
			{
				case AF_INET:
				{
					// Set first byte to 0.
					attrValue[0] = 0;
					// Set inet family.
					attrValue[1] = 0x01;
					// Set port and XOR it.
					std::memcpy(
					  attrValue + 2,
					  &(reinterpret_cast<const sockaddr_in*>(this->xorMappedAddress))->sin_port,
					  2);
					attrValue[2] ^= StunPacket::magicCookie[0];
					attrValue[3] ^= StunPacket::magicCookie[1];
					// Set address and XOR it.
					std::memcpy(
					  attrValue + 4,
					  &(reinterpret_cast<const sockaddr_in*>(this->xorMappedAddress))->sin_addr.s_addr,
					  4);
					attrValue[4] ^= StunPacket::magicCookie[0];
					attrValue[5] ^= StunPacket::magicCookie[1];
					attrValue[6] ^= StunPacket::magicCookie[2];
					attrValue[7] ^= StunPacket::magicCookie[3];

					pos += 4 + 8;

					break;
				}

				case AF_INET6:
				{
					// Set first byte to 0.
					attrValue[0] = 0;
					// Set inet family.
					attrValue[1] = 0x02;
					// Set port and XOR it.
					std::memcpy(
					  attrValue + 2,
					  &(reinterpret_cast<const sockaddr_in6*>(this->xorMappedAddress))->sin6_port,
					  2);
					attrValue[2] ^= StunPacket::magicCookie[0];
					attrValue[3] ^= StunPacket::magicCookie[1];
					// Set address and XOR it.
					std::memcpy(
					  attrValue + 4,
					  &(reinterpret_cast<const sockaddr_in6*>(this->xorMappedAddress))->sin6_addr.s6_addr,
					  16);
					attrValue[4] ^= StunPacket::magicCookie[0];
					attrValue[5] ^= StunPacket::magicCookie[1];
					attrValue[6] ^= StunPacket::magicCookie[2];
					attrValue[7] ^= StunPacket::magicCookie[3];
					attrValue[8] ^= this->transactionId[0];
					attrValue[9] ^= this->transactionId[1];
					attrValue[10] ^= this->transactionId[2];
					attrValue[11] ^= this->transactionId[3];
					attrValue[12] ^= this->transactionId[4];
					attrValue[13] ^= this->transactionId[5];
					attrValue[14] ^= this->transactionId[6];
					attrValue[15] ^= this->transactionId[7];
					attrValue[16] ^= this->transactionId[8];
					attrValue[17] ^= this->transactionId[9];
					attrValue[18] ^= this->transactionId[10];
					attrValue[19] ^= this->transactionId[11];

					pos += 4 + 20;

					break;
				}
			}
		}

		// Add ERROR-CODE.
		if (addErrorCode)
		{
			rtc_byte::set2bytes(buffer, pos, static_cast<uint16_t>(Attribute::ERROR_CODE));
			rtc_byte::set2bytes(buffer, pos + 2, 4);

			auto codeClass     = static_cast<uint8_t>(this->errorCode / 100);
			uint8_t codeNumber = static_cast<uint8_t>(this->errorCode) - (codeClass * 100);

			rtc_byte::set2bytes(buffer, pos + 4, 0);
			rtc_byte::set1byte(buffer, pos + 6, codeClass);
			rtc_byte::set1byte(buffer, pos + 7, codeNumber);
			pos += 4 + 4;
		}

		// Add MESSAGE-INTEGRITY.
		if (addMessageIntegrity)
		{
			// Ignore FINGERPRINT.
			if (addFingerprint)
			{
				rtc_byte::set2bytes(buffer, 2, static_cast<uint16_t>(this->size - 20 - 8));
			}

			// Calculate the HMAC-SHA1 of the packet according to MESSAGE-INTEGRITY rules.
			const uint8_t* computedMessageIntegrity =
			 s_crypto_random. GetHmacSha1(this->password, buffer, pos);

			rtc_byte::set2bytes(buffer, pos, static_cast<uint16_t>(Attribute::MESSAGE_INTEGRITY));
			rtc_byte::set2bytes(buffer, pos + 2, 20);
			std::memcpy(buffer + pos + 4, computedMessageIntegrity, 20);

			// Update the pointer.
			this->messageIntegrity = buffer + pos + 4;
			pos += 4 + 20;

			// Restore length field.
			if (addFingerprint)
			{
				rtc_byte::set2bytes(buffer, 2, static_cast<uint16_t>(this->size - 20));
			}
		}
		else
		{
			// Unset the pointer (if it was set).
			this->messageIntegrity = nullptr;
		}

		// Add FINGERPRINT.
		if (addFingerprint)
		{
			// Compute the CRC32 of the packet up to (but excluding) the FINGERPRINT
			// attribute and XOR it with 0x5354554e.
			uint32_t computedFingerprint = s_crypto_random. GetCRC32(buffer, pos) ^ 0x5354554e;

			rtc_byte::set2bytes(buffer, pos, static_cast<uint16_t>(Attribute::FINGERPRINT));
			rtc_byte::set2bytes(buffer, pos + 2, 4);
			rtc_byte::set4bytes(buffer, pos + 4, computedFingerprint);
			pos += 4 + 4;

			// Set flag.
			this->hasFingerprint = true;
		}
		else
		{
			this->hasFingerprint = false;
		}

		//MS_ASSERT(pos == this->size, "pos != this->size");
	}
} // namespace RTC

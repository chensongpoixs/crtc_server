﻿#ifndef MS_RTC_RTCP_PACKET_HPP
#define MS_RTC_RTCP_PACKET_HPP

//#include "common.hpp"
#include <map>
#include <string>
#include "cnet_type.h"
namespace RTC
{
	namespace RTCP
	{
		// Internal buffer for RTCP serialization.
		constexpr size_t BufferSize{ 65536 };
		extern uint8_t Buffer[BufferSize];

		// Maximum interval for regular RTCP mode.
		constexpr uint16_t MaxAudioIntervalMs{ 5000 };
		constexpr uint16_t MaxVideoIntervalMs{ 1000 };

		enum class Type : uint8_t
		{
			SR    = 200,
			RR    = 201,
			SDES  = 202,
			BYE   = 203,
			APP   = 204,
			RTPFB = 205,
			PSFB  = 206,
			XR    = 207
		};

		class Packet
		{
		public:
			/* Struct for RTCP common header. */
			struct CommonHeader
			{
#if defined(MS_LITTLE_ENDIAN) // TODO@chensong 202208011 win 平台
				uint8_t count : 5;  // 一个包中Report Block个数
				uint8_t padding : 1;// 填充标识， 最后一个填充字节是（）个数
				uint8_t version : 2;
#elif defined(MS_BIG_ENDIAN)
				uint8_t version : 2;
				uint8_t padding : 1;
				uint8_t count : 5;
#endif
				uint8_t packetType : 8; // 不同RTCP包的类型
				uint16_t length : 16;   // 16位，包长度（包括头）。[数值为（N-1）个4字节]
			};

		public:
			static bool IsRtcp(const uint8_t* data, size_t len)
			{
				auto header = const_cast<CommonHeader*>(reinterpret_cast<const CommonHeader*>(data));

				// clang-format off
				return (
					(len >= sizeof(CommonHeader)) &&
					// DOC: https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
					(data[0] > 127 && data[0] < 192) &&
					// RTP Version must be 2.
					(header->version == 2) &&
					// RTCP packet types defined by IANA:
					// http://www.iana.org/assignments/rtp-parameters/rtp-parameters.xhtml#rtp-parameters-4
					// RFC 5761 (RTCP-mux) states this range for secure RTCP/RTP detection.
					(header->packetType > 191/*>= 192*/ && header->packetType <224/*<= 223*/)
				);
				// clang-format on
			}
			// RTCP INFO 数据分析 网络评估 的数据的
			static Packet* Parse(const uint8_t* data, size_t len);
			static const std::string& Type2String(Type type);

		private:
			static std::map<Type, std::string> type2String;

		public:
			explicit Packet(Type type) : type(type)
			{
			}
			explicit Packet(CommonHeader* commonHeader)
			{
				this->type   = RTCP::Type(commonHeader->packetType);
				this->header = commonHeader;
			}
			virtual ~Packet() = default;

			void SetNext(Packet* packet)
			{
				this->next = packet;
			}
			Packet* GetNext() const
			{
				return this->next;
			}
			const uint8_t* GetData() const
			{
				return reinterpret_cast<uint8_t*>(this->header);
			}

		public:
			virtual void Dump() const                 = 0;
			virtual size_t Serialize(uint8_t* buffer) = 0;
			virtual Type GetType() const
			{
				return this->type;
			}
			virtual size_t GetCount() const
			{
				return 0u;
			}
			virtual size_t GetSize() const = 0;

		private:
			Type type;
			Packet* next{ nullptr };
			CommonHeader* header{ nullptr };
		};
	} // namespace RTCP
} // namespace RTC

#endif

﻿#ifndef MS_RTC_ICE_CANDIDATE_HPP
#define MS_RTC_ICE_CANDIDATE_HPP

//#include "common.hpp"
//#include "RTC/TcpServer.hpp"
//#include "RTC/UdpSocket.hpp"
//#include <nlohmann/json.hpp>
//#include <string>

//using json = nlohmann::json;

#include <string>
#include "cudp_socket.h"

#include <json/json.h>

namespace RTC
{
	class IceCandidate
	{
	public:
		enum class Protocol
		{
			UDP = 1,
			TCP
		};

	public:
		enum class CandidateType
		{
			HOST = 1
		};

	public:
		enum class TcpCandidateType
		{
			PASSIVE = 1
		};

	public:
		IceCandidate(chen::cudp_socket* udpSocket, uint32_t priority)
		  : foundation("udpcandidate"), priority(priority), ip(udpSocket->GetLocalIp()),
		    protocol(Protocol::UDP), port(udpSocket->GetLocalPort()), type(CandidateType::HOST)
		{
		}

		IceCandidate(chen::cudp_socket* udpSocket, uint32_t priority, std::string& announcedIp)
		  : foundation("udpcandidate"), priority(priority), ip(announcedIp), protocol(Protocol::UDP),
		    port(udpSocket->GetLocalPort()), type(CandidateType::HOST)
		{
		}

		/*IceCandidate(RTC::TcpServer* tcpServer, uint32_t priority)
		  : foundation("tcpcandidate"), priority(priority), ip(tcpServer->GetLocalIp()),
		    protocol(Protocol::TCP), port(tcpServer->GetLocalPort()), type(CandidateType::HOST),
		    tcpType(TcpCandidateType::PASSIVE)
		{
		}

		IceCandidate(RTC::TcpServer* tcpServer, uint32_t priority, std::string& announcedIp)
		  : foundation("tcpcandidate"), priority(priority), ip(announcedIp), protocol(Protocol::TCP),
		    port(tcpServer->GetLocalPort()), type(CandidateType::HOST),
		    tcpType(TcpCandidateType::PASSIVE)
		{
		}*/

		//void FillJson(json& jsonObject) const;
		std::string candidate( );
		bool reply(Json::Value&value);
		// const  bool reply(Json::Value&value) const;
	private:
		// Others.
		std::string foundation;
		uint32_t priority{ 0u };
		std::string ip;
		Protocol protocol;
		uint16_t port{ 0u };
		CandidateType type{ CandidateType::HOST };
		TcpCandidateType tcpType{ TcpCandidateType::PASSIVE };
	};
} // namespace RTC

#endif

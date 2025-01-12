﻿/***********************************************************************************************
created: 		2023-03-12

author:			chensong

purpose:		rtc_producer

输赢不重要，答案对你们有什么意义才重要。

光阴者，百代之过客也，唯有奋力奔跑，方能生风起时，是时代造英雄，英雄存在于时代。或许世人道你轻狂，可你本就年少啊。 看护好，自己的理想和激情。


我可能会遇到很多的人，听他们讲好2多的故事，我来写成故事或编成歌，用我学来的各种乐器演奏它。
然后还可能在一个国家遇到一个心仪我的姑娘，她可能会被我帅气的外表捕获，又会被我深邃的内涵吸引，在某个下雨的夜晚，她会全身淋透然后要在我狭小的住处换身上的湿衣服。
3小时候后她告诉我她其实是这个国家的公主，她愿意向父皇求婚。我不得已告诉她我是穿越而来的男主角，我始终要回到自己的世界。
然后我的身影慢慢消失，我看到她眼里的泪水，心里却没有任何痛苦，我才知道，原来我的心被丢掉了，我游历全世界的原因，就是要找回自己的本心。
于是我开始有意寻找各种各样失去心的人，我变成一块砖头，一颗树，一滴水，一朵白云，去听大家为什么会失去自己的本心。
我发现，刚出生的宝宝，本心还在，慢慢的，他们的本心就会消失，收到了各种黑暗之光的侵蚀。
从一次争论，到嫉妒和悲愤，还有委屈和痛苦，我看到一只只无形的手，把他们的本心扯碎，蒙蔽，偷走，再也回不到主人都身边。
我叫他本心猎手。他可能是和宇宙同在的级别 但是我并不害怕，我仔细回忆自己平淡的一生 寻找本心猎手的痕迹。
沿着自己的回忆，一个个的场景忽闪而过，最后发现，我的本心，在我写代码的时候，会回来。
安静，淡然，代码就是我的一切，写代码就是我本心回归的最好方式，我还没找到本心猎手，但我相信，顺着这个线索，我一定能顺藤摸瓜，把他揪出来。
************************************************************************************************/
#include "crtc_producer.h"
#include "crtp_header_extensions.h"
#include "ctransport_mgr.h"
#include "crtc_transport.h"
namespace chen {

	crtc_producer::~crtc_producer()
	{

	}

	void crtc_producer::destroy()
	{
		for (std::unordered_map<uint32, crtp_stream_recv*>::iterator iter = m_ssrc_rtp_stream_map.begin(); iter != m_ssrc_rtp_stream_map.end(); ++iter)
		{
			iter->second->destroy();
			delete iter->second;
		}
		m_ssrc_rtp_stream_map.clear();
		m_rtx_ssrc_rtp_stream_map.clear();
		m_server_ssrc_map.clear();
	}

	bool crtc_producer::receive_rtp_packet(RTC::RtpPacket * packet)
	{
		// Reset current packet.
		//this->currentRtpPacket = nullptr;
		//return true;
		// Count number of RTP streams.
	//	auto numRtpStreamsBefore = this->mapSsrcRtpStream.size();
#if RTP_PACKET_MCS
		std::chrono::steady_clock::time_point cur_time_ms;
		std::chrono::steady_clock::time_point pre_time = std::chrono::steady_clock::now();
		std::chrono::steady_clock::duration dur;
		std::chrono::microseconds ms;
#endif
#if RTP_PACKET_FILE_MCS
		std::chrono::steady_clock::time_point cur_time_ms;
		std::chrono::steady_clock::time_point pre_time = std::chrono::steady_clock::now();
		std::chrono::steady_clock::duration dur;
		std::chrono::microseconds ms;
#endif
		crtp_stream_recv* rtpStream = _get_rtp_stream(packet);

		if (!rtpStream)
		{
			WARNING_EX_LOG(  "rtp no stream found for received packet [ssrc:%" PRIu32 "]", packet->GetSsrc());

			return false;
		}

		// Pre-process the packet.
		//PreProcessRtpPacket(packet);

		//ReceiveRtpPacketResult result;
		bool isRtx{ false };
		/*	int32 biff_ms = uv_util::GetTimeMs() - packet->GetTimestamp();
			if (biff_ms > 150)
			{
				NORMAL_EX_LOG("[ssrc = %u][seq = %u][timestamp = %u][biff_ms = %u]", packet->GetSsrc(), packet->GetSequenceNumber(), packet->GetTimestamp(),   biff_ms);
			}*/
		//NORMAL_EX_LOG("[ssrc = %u][seq = %u][timestamp = %u] ", packet->GetSsrc(), packet->GetSequenceNumber(), packet->GetTimestamp() );
		// Media packet.
		if (packet->GetSsrc() == rtpStream->get_ssrc())
		{
			//result = ReceiveRtpPacketResult::MEDIA;

			// Process the packet.
			if (!rtpStream->receive_packet(packet))
			{
				// May have to announce a new RTP stream to the listener.
				/*if (this->mapSsrcRtpStream.size() > numRtpStreamsBefore)
				{
					NotifyNewRtpStream(rtpStream);
				}
*/
				WARNING_EX_LOG("[ssrc = %u][seq = %u][timestamp = %u] ", packet->GetSsrc(), packet->GetSequenceNumber());
				return false;
			}
			 
		}
		// RTX packet.
		else if (packet->GetSsrc() == rtpStream->get_rtx_ssrc())
		{
			//result = ReceiveRtpPacketResult::RETRANSMISSION;
			isRtx = true;
			 
			// Process the packet.
			if (!rtpStream->receive_rtx_packet(packet))
			{
				WARNING_EX_LOG("[ssrc = %u][seq = %u] ", packet->GetSsrc(), packet->GetSequenceNumber());
				return false;
			}
		}
		// Should not happen.
		else
		{
			WARNING_EX_LOG("[ssrc = %u][seq = %u][timestamp = %u] ", packet->GetSsrc(), packet->GetSequenceNumber());
			cassert("found stream does not match received packet");
		}
		//return true;
		if (packet->IsKeyFrame())
		{
			DEBUG_EX_LOG( "rtp key frame received [ssrc:%" PRIu32 ", seq:%" PRIu16 "]",
				packet->GetSsrc(),
				packet->GetSequenceNumber());

			// Tell the keyFrameRequestManager.
			//if (this->keyFrameRequestManager)
			{
				//this->keyFrameRequestManager->KeyFrameReceived(packet->GetSsrc());
			}
		}

		// May have to announce a new RTP stream to the listener.
		//if (this->mapSsrcRtpStream.size() > numRtpStreamsBefore)
		//{
		//	// Request a key frame for this stream since we may have lost the first packets
		//	// (do not do it if this is a key frame).
		//	if (this->keyFrameRequestManager && !this->paused && !packet->IsKeyFrame())
		//	{
		//		this->keyFrameRequestManager->ForceKeyFrameNeeded(packet->GetSsrc());
		//	}

		//	// Update current packet.
		//	this->currentRtpPacket = packet;

		//	NotifyNewRtpStream(rtpStream);

		//	// Reset current packet.
		//	this->currentRtpPacket = nullptr;
		//}

		//// If paused stop here.
		//if (m_paused)
		//{
		//	return result;
		//}

		// May emit 'trace' event.
		//EmitTraceEventRtpAndKeyFrameTypes(packet, isRtx);

		// Mangle the packet before providing the listener with it.
		// 在向侦听器提供数据包之前，先将其弄乱。
		/*if (!MangleRtpPacket(packet, rtpStream))
		{
			return ReceiveRtpPacketResult::DISCARDED;
		}*/
#if 0
		cur_time_ms = std::chrono::steady_clock::now();
		dur = cur_time_ms - pre_time;
		ms = std::chrono::duration_cast<std::chrono::microseconds>(dur);
		if (ms.count() > 5)
		{
			WARNING_EX_LOG("==>udp recv rtp packet seq #########  --> [microseconds = %" PRIi64 "]", ms.count());
		}
		pre_time = cur_time_ms;
#endif 
		  mangle_rtp_packet(packet, m_params.params);
#if RTP_PACKET_MCS
		  cur_time_ms = std::chrono::steady_clock::now();
		    dur = cur_time_ms - pre_time;
		    ms = std::chrono::duration_cast<std::chrono::microseconds>(dur);

			static FILE * out_file_recv_ptr = fopen("./rtp_recv_packet.csv", "wb+");
			if (out_file_recv_ptr)
			{
				fprintf(out_file_recv_ptr, "%" PRIi64 ", %" PRIi32 "\n", ms.count(), packet->GetSize());
				fflush(out_file_recv_ptr);
			}
			if (ms.count() > 5)
			{
				WARNING_EX_LOG("==>udp recv rtp packet mangle_rtp_packet #########  --> [microseconds = %" PRIi64 "]", ms.count());
		   }
			pre_time = cur_time_ms;
#endif 
#if RTP_PACKET_FILE_MCS
			cur_time_ms = std::chrono::steady_clock::now();
			dur = cur_time_ms - pre_time;
			ms = std::chrono::duration_cast<std::chrono::microseconds>(dur);

			static FILE * out_file_recv_ptr = fopen("./rtp_recv_packet.csv", "wb+");
			if (out_file_recv_ptr)
			{
				fprintf(out_file_recv_ptr, "%" PRIi64 ", %" PRIi32 "\n", ms.count(), packet->GetSize());
				fflush(out_file_recv_ptr);
			}
			 
			pre_time = cur_time_ms;
#endif 
		  //return true;
		  if (m_rtc_ptr)
		  {
			  m_rtc_ptr->broadcast_consumers(packet);

#if RTP_PACKET_MCS
			   cur_time_ms = std::chrono::steady_clock::now();
			  dur = cur_time_ms - pre_time;
			  ms = std::chrono::duration_cast<std::chrono::microseconds>(dur);
			  if (ms.count() > 20)
			  {
				  WARNING_EX_LOG("==>udp recv rtp packet  send consumer  ######## packet  --> [microseconds = %" PRIi64 "]", ms.count());
			  } 
#endif 
		  }
		 // for (crtc_transport* transport: m)
		  return true;
		 // return true;
		// Post-process the packet.
		// 处理视频 转的角度
		//PostProcessRtpPacket(packet);
		// 所有的订阅的客户端媒体信息
		// 数据保持在每个接受客户端缓存中中了RtpSend
		//this->listener->OnProducerRtpPacketReceived(this, packet);
		///*  cur_time_ms = std::chrono::steady_clock::now();
		//  dur = cur_time_ms - pre_time;
		//  ms = std::chrono::duration_cast<std::chrono::microseconds>(dur);
		//  NORMAL_EX_LOG("udp recv rtp packet  recv packet  --> [microseconds = %" PRIi64 "]", ms.count());*/
		  std::unordered_set <crtc_transport*> & consumer_all =  g_transport_mgr.m_all_consumer_map[m_rtc_ptr->get_rtp_sdp().m_msids[0]];
		  if (consumer_all.empty())
		  {
			  return true;
		  }
		for (crtc_transport* rtc_ptr : consumer_all)
		{
			//crtc_transport* rtc_ptr = g_transport_mgr.find_stream_name(stream_name);
			if (!rtc_ptr)
			{
				 WARNING_EX_LOG("not find stream_name = %s", m_rtc_ptr->get_rtp_sdp().m_msids[0].c_str());
				continue;
			}
			if (!rtc_ptr->get_dtls_connected_ok())
			{
				WARNING_EX_LOG("  stream_name = %s  ICE dtls connected not ok !!!", m_rtc_ptr->get_rtp_sdp().m_msids[0].c_str());
				continue;
			}
			rtc_ptr->send_consumer(packet);
			//if (/*params.type*/  get_kind() == "audio")
			//{
			//	rtc_ptr->send_rtp_audio_data(packet);
			//}
			//else
			//{
			//	if (/*m_all_rtx_video_ssrc*/  get_rtcp_params().params.rtx_ssrc == packet->GetSsrc())
			//	{
			//		rtc_ptr->send_rtp_rtx_video_data(packet);
			//	}
			//	else if (/*m_all_video_ssrc*/  get_rtcp_params().params.ssrc == packet->GetSsrc())
			//	{
			//		m_rtc_ptr->get_remote_estimator()->on_packet_arrival(packet->GetSequenceNumber(), packet->GetSsrc(), packet->GetTimestamp());
			//		//NORMAL_EX_LOG("[video][rtp ][ssrc = %u][size = %u][GetSequenceNumber = %u][GetPayloadType = %u][timestamp = %u][marker = %u]", packet->GetSsrc(), len, packet->GetSequenceNumber(), packet->GetPayloadType(), packet->GetTimestamp(), packet->HasMarker());
			//		//RTC::Codecs::H264::ProcessRtpPacket(packet);
			//		rtc_ptr->send_rtp_video_data(packet);
			//	}
			//	else
			//	{
			//		WARNING_EX_LOG("[rtc type = %s][video rtx ][not find ssrc = %u][]", m_rtc_ptr->get_rtc_type() == ERtcClientPlayer ? "rtc_player" : "rtc_publisher", packet->GetSsrc());
			//	}

			//}

		}
		return true;
	}

	void crtc_producer::receive_rtcp_sender_report(RTC::RTCP::SenderReport* report)
	{
		auto it = m_ssrc_rtp_stream_map.find(report->GetSsrc());

		if (it != m_ssrc_rtp_stream_map.end())
		{
			auto* rtpStream = it->second;
			bool first = rtpStream->get_sender_report_ntp_ms() == 0;

			rtpStream->receive_rtcp_sender_report(report);

			//this->listener->OnProducerRtcpSenderReport(this, rtpStream, first);

			return;
		}

		// If not found, check with RTX.
		auto it2 = m_rtx_ssrc_rtp_stream_map.find(report->GetSsrc());

		if (it2 != this->m_rtx_ssrc_rtp_stream_map.end())
		{
			auto* rtpStream = it2->second;

			rtpStream->receive_rtx_rtcp_sender_report(report);

			return;
		}
	}

	void crtc_producer::get_rtcp(RTC::RTCP::CompoundPacket * packet, uint64_t nowMs)
	{
		if (static_cast<float>((nowMs - this->m_last_rtcp_send_time) * 1.15) < 1000u)
		{
			return;
		}

		 for (auto& kv : this->m_ssrc_rtp_stream_map)
		{
			 
			RTC::RTCP::ReceiverReport*	report = kv.second->get_rtcp_receiver_report();

			packet->AddReceiverReport(report);

			RTC::RTCP::ReceiverReport*	rtxReport = kv.second->get_rtx_rtcp_receiver_report();

			if (rtxReport)
			{
				packet->AddReceiverReport(rtxReport);
			}
		}

		// Add a receiver reference time report if no present in the packet.
		if (!packet->HasReceiverReferenceTime())
		{
			rtc_time::Ntp				    ntp = rtc_time::TimeMs2Ntp(nowMs);
			RTC::RTCP::ReceiverReferenceTime* report = new RTC::RTCP::ReceiverReferenceTime();

			report->SetNtpSec(ntp.seconds);
			report->SetNtpFrac(ntp.fractions);
			packet->AddReceiverReferenceTime(report);
		}

		this->m_last_rtcp_send_time = nowMs;
	}

	void crtc_producer::request_key_frame(uint32 mapped_ssrc)
	{
	}

	crtp_stream_recv * crtc_producer::_get_rtp_stream(RTC::RtpPacket * packet)
	{
		uint32  ssrc = packet->GetSsrc();
		uint8  payloadType = packet->GetPayloadType();

		// If stream found in media ssrcs map, return it.
		{
			std::unordered_map<uint32, crtp_stream_recv*>::iterator it = this->m_ssrc_rtp_stream_map.find(ssrc);

			if (it != this->m_ssrc_rtp_stream_map.end())
			{
				crtp_stream_recv* rtpStream = it->second;

				return rtpStream;
			}
		}

		// If stream found in RTX ssrcs map, return it.
		// 这边会看map中是否有ssrc中RTX中重传数据ssrc
		{
			std::unordered_map<uint32, crtp_stream_recv*>::iterator  it = this->m_rtx_ssrc_rtp_stream_map.find(ssrc);

			if (it != this->m_rtx_ssrc_rtp_stream_map.end())
			{
				crtp_stream_recv* rtpStream = it->second;

				return rtpStream;
			}
		}

		// Otherwise check our encodings and, if appropriate, create a new stream.
		crtp_stream_recv* rtpStream = _create_rtp_stream(packet);
		return rtpStream;
		/*if (m_params.params.ssrc == ssrc && payloadType == m_params.params.payload_type)
		{

		}*/
		// First, look for an encoding with matching media or RTX ssrc value.
		//for (size_t i{ 0 }; i < this->rtpParameters.encodings.size(); ++i)
		//{
		//	auto& encoding = this->rtpParameters.encodings[i];
		//	const auto* mediaCodec = this->rtpParameters.GetCodecForEncoding(encoding);
		//	const auto* rtxCodec = this->rtpParameters.GetRtxCodecForEncoding(encoding);
		//	bool isMediaPacket = (mediaCodec->payloadType == payloadType);
		//	bool isRtxPacket = (rtxCodec && rtxCodec->payloadType == payloadType);

		//	if (isMediaPacket && encoding.ssrc == ssrc)
		//	{
		//		auto* rtpStream = CreateRtpStream(packet, *mediaCodec, i);

		//		return rtpStream;
		//	}
		//	else if (isRtxPacket && encoding.hasRtx && encoding.rtx.ssrc == ssrc)
		//	{
		//		////////////////////////////////////RTX/////////////////////////////////////////////////////
		//		//一、什么是 RTX
		//		//	RTX 就是重传 Retransmission, 将丢失的包重新由发送方传给接收方。
		//		//
		//		//	Webrtc 默认开启 RTX (重传)，它一般采用不同的 SSRC 进行传输，即原始的 RTP 包和重传的 RTP 包的 SSRC 是不同的，这样不会干扰原始 RTP 包的度量。
		//		//
		//		//	RTX 包的 Payload 在 RFC4588 中有详细描述，一般 NACK 导致的重传包和 Bandwidth Probe 导致的探测包也可能走 RTX 通道。
		//		//
		//		//
		//		//	为什么用 RTX
		//		//	媒体流多使用 RTP 通过 UDP 进行传输，由于是不可靠传输，丢包是不可避免，也是可以容忍的，但是对于一些关键数据是不能丢失的，这时候就需要重传(RTX)。
		//		//
		//		//二、在 WebRTC 中常用的 QoS 策略有
		//		//
		//		//	反馈：例如 PLI , NACK
		//		//	冗余， 例如 FEC, RTX
		//		//	调整：例如码率，分辨率及帧率的调整
		//		//	缓存: 例如 Receive Adaptive Jitter Buffer, Send Buffer
		//		//	这些措施一般需要结合基于拥塞控制(congestion control) 及带宽估计(bandwidth estimation)技术, 不同的网络条件下需要采用不同的措施。
		//		//
		//		//	FEC 用作丢包恢复需要占用更多的带宽，即使 5% 的丢包需要几乎一倍的带宽，在带宽有限的情况下可能会使情况更糟。
		//		//
		//		//	RTX 不会占用太多的带宽，接收方发送 NACK 指明哪些包丢失了，发送方通过单独的 RTP 流（不同的 SSRC）来重传丢失的包，但是 RTX 至少需要一个 RTT 来修复丢失的包。
		//		//
		//		//	音频流对于延迟很敏感，而且占用带宽不多，所以用 FEC 更好。WebRTC 默认并不为 audio 开启 RTX
		//		//	视频流对于延迟没那么敏感，而且占用带宽很多，所以用 RTX 更好。
		//		/////////////////////////////////////////////////////////////////////////////////////////
		//		auto it = this->mapSsrcRtpStream.find(encoding.ssrc);

		//		// Ignore if no stream has been created yet for the corresponding encoding.
		//		if (it == this->mapSsrcRtpStream.end())
		//		{
		//			MS_DEBUG_2TAGS(rtp, rtx, "ignoring RTX packet for not yet created RtpStream (ssrc lookup)");

		//			return nullptr;
		//		}

		//		auto* rtpStream = it->second;

		//		// Ensure no RTX ssrc was previously detected.
		//		// 确保之前未检测到RTX ssrc
		//		if (rtpStream->HasRtx())
		//		{
		//			MS_DEBUG_2TAGS(rtp, rtx, "ignoring RTX packet with new ssrc (ssrc lookup)");

		//			return nullptr;
		//		}

		//		// Update the stream RTX data.
		//		rtpStream->SetRtx(payloadType, ssrc);

		//		// Insert the new RTX ssrc into the map.
		//		// TODO@chensong 20220909 怎么处理RTX的视频包重传数据呢
		//		this->mapRtxSsrcRtpStream[ssrc] = rtpStream;

		//		return rtpStream;
		//	}
		//}


		return nullptr;
	}

	chen::crtp_stream_recv* crtc_producer::_create_rtp_stream(RTC::RtpPacket* packet)
	{
		
		if (m_params.params.payload_type == packet->GetPayloadType() && m_params.params.ssrc == packet->GetSsrc())
		{
			crtp_stream_recv * rtp_stream = new crtp_stream_recv(this, m_params.params);
			if (!m_ssrc_rtp_stream_map.insert(std::make_pair(m_params.params.ssrc, rtp_stream)).second)
			{ 
				WARNING_EX_LOG("rtp stream insert (ssrc = %u)  failed ", m_params.params.ssrc);
				delete rtp_stream;
				rtp_stream = NULL;
				return NULL;
			}
			return rtp_stream;
		}
		else if (m_params.params.rtx_payload_type == packet->GetPayloadType() && m_params.params.rtx_ssrc == packet->GetSsrc())
		{
			std::unordered_map<uint32, crtp_stream_recv*>::iterator iter =  m_ssrc_rtp_stream_map.find(m_params.params.ssrc);
			if (iter == m_ssrc_rtp_stream_map.end())
			{
				WARNING_EX_LOG("find (ssrc = %u) failed  create rtp stream (packet_ssrc = %u)(rtx_ssrc = %u) failed !!!", m_params.params.ssrc, packet->GetSsrc(), m_params.params.rtx_ssrc);
				return NULL;
			}
			iter->second->set_rtx(m_params.params.rtx_payload_type, packet->GetPayloadType());
			if (!m_rtx_ssrc_rtp_stream_map.insert(std::make_pair(m_params.params.rtx_ssrc, iter->second)).second)
			{
				WARNING_EX_LOG("insert rtx rtp stream (rtx_ssrc = %u)failed !!!", m_params.params.rtx_ssrc);
				iter->second->del_rtx();

				return NULL;
			}
			return iter->second;
		}
		return NULL;
	}
	bool  crtc_producer::mangle_rtp_packet(RTC::RtpPacket * packet, const crtp_stream::crtp_stream_params & params)
	{
		// Mangle the payload type.

		// Mangle the SSRC.
		{
			// 2.  修改为服务端ssrc   这个可能是因为不同端可能会冲突的问题吧
			std::unordered_map<uint32, uint32>::const_iterator iter =  m_server_ssrc_map.find(packet->GetSsrc());
			if (iter == m_server_ssrc_map.end())
			{
				WARNING_EX_LOG("not find ssrc = %u failed !!!", packet->GetSsrc());
			}
			else
			{
				packet->SetSsrc(iter->second);
			} 
		}
		//return true;
		// Mangle RTP header extensions.
		{
			thread_local static uint8_t buffer[4096];
			thread_local static std::vector<RTC::RtpPacket::GenericExtension> extensions;

			// This happens just once.
			if (extensions.capacity() != 24)
			{
				extensions.reserve(24);
			}

			extensions.clear();

			uint8_t* extenValue;
			uint8_t extenLen;
			uint8_t* bufferPtr{ buffer };

			// Add urn:ietf:params:rtp-hdrext:sdes:mid.
			{
				extenLen = RTC::MidMaxLength;

				extensions.emplace_back(
					static_cast<uint8_t>(EMID), extenLen, bufferPtr);

				bufferPtr += extenLen;
			}

			if (params.type == EMediaAudio)
			{
				// Proxy urn:ietf:params:rtp-hdrext:ssrc-audio-level.
				extenValue = packet->GetExtension(ESSRC_AUDIO_LEVEL, extenLen);

				if (extenValue)
				{
					std::memcpy(bufferPtr, extenValue, extenLen);

					extensions.emplace_back(
						static_cast<uint8_t>(ESSRC_AUDIO_LEVEL),
						extenLen,
						bufferPtr);

					// Not needed since this is the latest added extension.
					// bufferPtr += extenLen;
				}
			}
			else if (params.type == EMediaVideo)
			{
				// Add http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time.
				{
					extenLen = 3u;

					// NOTE: Add value 0. The sending Transport will update it.
					uint32_t absSendTime{ 0u };

					rtc_byte::set3bytes(bufferPtr, 0, absSendTime);

					extensions.emplace_back(
						static_cast<uint8_t>(EABS_SEND_TIME), extenLen, bufferPtr);

					bufferPtr += extenLen;
				}

				// Add http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01.
				{
					extenLen = 2u;

					// NOTE: Add value 0. The sending Transport will update it.
					uint16_t wideSeqNumber{ 0u };

					rtc_byte::set3bytes(bufferPtr, 0, wideSeqNumber);

					extensions.emplace_back(
						static_cast<uint8_t>(ETRANSPORT_WIDE_CC_01),
						extenLen,
						bufferPtr);

					bufferPtr += extenLen;
				}

				// NOTE: Remove this once framemarking draft becomes RFC.
				// Proxy http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07.
				extenValue = packet->GetExtension(EFRAME_MARKING_07, extenLen);

				if (extenValue)
				{
					std::memcpy(bufferPtr, extenValue, extenLen);

					extensions.emplace_back(
						static_cast<uint8_t>(EFRAME_MARKING_07),
						extenLen,
						bufferPtr);

					bufferPtr += extenLen;
				}

				// Proxy urn:ietf:params:rtp-hdrext:framemarking.
				extenValue = packet->GetExtension(EFRAME_MARKING, extenLen);

				if (extenValue)
				{
					std::memcpy(bufferPtr, extenValue, extenLen);

					extensions.emplace_back(
						static_cast<uint8_t>(EFRAME_MARKING), extenLen, bufferPtr);

					bufferPtr += extenLen;
				}

				// Proxy urn:3gpp:video-orientation.
				extenValue = packet->GetExtension(EVIDEO_ORIENTATION, extenLen);

				if (extenValue)
				{
					std::memcpy(bufferPtr, extenValue, extenLen);

					extensions.emplace_back(
						static_cast<uint8_t>(EVIDEO_ORIENTATION),
						extenLen,
						bufferPtr);

					bufferPtr += extenLen;
				}

				// Proxy urn:ietf:params:rtp-hdrext:toffset.
				extenValue = packet->GetExtension(ETOFFSET, extenLen);

				if (extenValue)
				{
					std::memcpy(bufferPtr, extenValue, extenLen);

					extensions.emplace_back(
						static_cast<uint8_t>(ETOFFSET), extenLen, bufferPtr);

					// Not needed since this is the latest added extension.
					// bufferPtr += extenLen;
				}
			}

			// Set the new extensions into the packet using One-Byte format.
			packet->SetExtensions(1, extensions);

			// Assign mediasoup RTP header extension ids (just those that mediasoup may
			// be interested in after passing it to the Router).
			packet->SetMidExtensionId(static_cast<uint8_t>(EMID));
			packet->SetAbsSendTimeExtensionId(
				static_cast<uint8_t>(EABS_SEND_TIME));
			packet->SetTransportWideCc01ExtensionId(
				static_cast<uint8_t>(ETRANSPORT_WIDE_CC_01));
			// NOTE: Remove this once framemarking draft becomes RFC.
			packet->SetFrameMarking07ExtensionId(
				static_cast<uint8_t>(EFRAME_MARKING_07));
			packet->SetFrameMarkingExtensionId(
				static_cast<uint8_t>(EFRAME_MARKING));
			packet->SetSsrcAudioLevelExtensionId(
				static_cast<uint8_t>(ESSRC_AUDIO_LEVEL));
			packet->SetVideoOrientationExtensionId(
				static_cast<uint8_t>(EVIDEO_ORIENTATION));
		}

		return true;
		return true;
	}

	void crtc_producer::receive_rtcp_xrdelay_since_lastrr(RTC::RTCP::DelaySinceLastRr::SsrcInfo * ssrcInfo)
	{
		auto it = m_ssrc_rtp_stream_map.find(ssrcInfo->GetSsrc());
		if (it == this->m_ssrc_rtp_stream_map.end())
		{
			WARNING_EX_LOG("rtcp, RtpStream not found [ssrc:%" PRIu32 "]", ssrcInfo->GetSsrc());

			return;
		}
		 
		it->second->receive_rtcp_xrdelay_since_lastrr(ssrcInfo);
	}

	void crtc_producer::OnProducerSendRtcpPacket(RTC::RTCP::Packet* packet)
	{
		if (m_rtc_ptr)
		{
			m_rtc_ptr->send_rtcp_packet(packet);
		}
	}

}
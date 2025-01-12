﻿/*
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
*/


//#define MS_CLASS "RTC::SimpleConsumer"
// #define MS_LOG_DEV_LEVEL 3

#include "SimpleConsumer.hpp"
//#include "DepLibUV.hpp"
//#include "Logger.hpp"
//#include "MediaSoupErrors.hpp"
//#include "Channel/ChannelNotifier.hpp"
#include "Tools.hpp"
#include <mutex>
#include <cstdio>
#include <cstring>
#include "clog.h"
namespace RTC
{
	/* Instance methods. */
	using namespace chen;
	SimpleConsumer::SimpleConsumer(
	  const std::string& id, const std::string& producerId, RTC::Consumer::Listener* listener, Json::Value& data)
	  : RTC::Consumer::Consumer(id, producerId, listener, data, RTC::RtpParameters::Type::SIMPLE)
	{
		//MS_TRACE();
	//	DEBUG_EX_LOG("producerId = %s, data = %s", producerId.c_str(), data.dump().c_str());
		// Ensure there is a single encoding.
		if (this->consumableRtpEncodings.size() != 1u)
		{
			ERROR_EX_LOG("invalid consumableRtpEncodings with size != 1");
			return;
		}
		auto& encoding         = this->rtpParameters.encodings[0];
		const auto* mediaCodec = this->rtpParameters.GetCodecForEncoding(encoding);

		this->keyFrameSupported = RTC::Codecs::Tools::CanBeKeyFrame(mediaCodec->mimeType);

		// Create RtpStreamSend instance for sending a single stream to the remote.
		CreateRtpStream();
	}

	SimpleConsumer::~SimpleConsumer()
	{
		//MS_TRACE();
		DEBUG_EX_LOG("");
		delete this->rtpStream;
	}

	//void SimpleConsumer::FillJson(Json::Value& jsonObject) const
	//{
	//	//MS_TRACE();

	//	// Call the parent method.
	//	//RTC::Consumer::FillJson(jsonObject);

	//	// Add rtpStream.
	//	//this->rtpStream->FillJson(jsonObject["rtpStream"]);
	//	//DEBUG_EX_LOG("jsonObject = %s", jsonObject.dump().c_str());
	//}

	void SimpleConsumer::FillJsonStats(Json::Value& jsonArray) const
	{
	//	MS_TRACE();

		// Add stats of our send stream.
		//jsonArray.emplace_back(json::value_t::object);
		//this->rtpStream->FillJsonStats(jsonArray[0]);

		// Add stats of our recv stream.
		if (this->producerRtpStream)
		{
			//jsonArray.emplace_back(json::value_t::object);
		//	this->producerRtpStream->FillJsonStats(jsonArray[1]);
		}
		//DEBUG_EX_LOG("jsonArray = %s", jsonArray.dump().c_str());
	}

	void SimpleConsumer::FillJsonScore(Json::Value& jsonObject) const
	{
		//MS_TRACE();

	//	MS_ASSERT(this->producerRtpStreamScores, "producerRtpStreamScores not set");

		//jsonObject["score"] = this->rtpStream->GetScore();

		//if (this->producerRtpStream)
		//	jsonObject["producerScore"] = this->producerRtpStream->GetScore();
		//else
		//	jsonObject["producerScore"] = 0;

		//jsonObject["producerScores"] = *this->producerRtpStreamScores;
		//DEBUG_EX_LOG("jsonObject = %s", jsonObject.dump().c_str());
	}
	 
	/*
	void SimpleConsumer::HandleRequest(Channel::ChannelRequest* request)
	{
		MS_TRACE();
		DEBUG_EX_LOG("methodid = %d, method = %s, data = %s, internal = %s", request->methodId, request->method.c_str(), request->data.dump().c_str(), request->internal.dump().c_str());
		switch (request->methodId)
		{
			case Channel::ChannelRequest::MethodId::CONSUMER_REQUEST_KEY_FRAME:
			{
				if (IsActive())
					RequestKeyFrame();

				request->Accept();

				break;
			}

			case Channel::ChannelRequest::MethodId::CONSUMER_SET_PREFERRED_LAYERS:
			{
				// Do nothing.

				request->Accept();

				break;
			}

			default:
			{
				// Pass it to the parent class.
				RTC::Consumer::HandleRequest(request);
			}
		}
	}
	
	*/
	void SimpleConsumer::ProducerRtpStream(RTC::RtpStream* rtpStream, uint32_t /*mappedSsrc*/)
	{
		//MS_TRACE();
		DEBUG_EX_LOG("");
		this->producerRtpStream = rtpStream;
	}

	void SimpleConsumer::ProducerNewRtpStream(RTC::RtpStream* rtpStream, uint32_t /*mappedSsrc*/)
	{
	//	MS_TRACE();
		DEBUG_EX_LOG("");
		this->producerRtpStream = rtpStream;

		// Emit the score event.
		EmitScore();
	}

	void SimpleConsumer::ProducerRtpStreamScore(
	  RTC::RtpStream* /*rtpStream*/, uint8_t /*score*/, uint8_t /*previousScore*/)
	{
		//MS_TRACE();
		DEBUG_EX_LOG("");
		// Emit the score event.
		EmitScore();
	}

	void SimpleConsumer::ProducerRtcpSenderReport(RTC::RtpStream* /*rtpStream*/, bool /*first*/)
	{
		//MS_TRACE();
		DEBUG_EX_LOG("");
		// Do nothing.
	}

	uint8_t SimpleConsumer::GetBitratePriority() const
	{
		//MS_TRACE();
		DEBUG_EX_LOG("");
		//MS_ASSERT(this->externallyManagedBitrate, "bitrate is not externally managed");

		// Audio SimpleConsumer does not play the BWE game.
		if (this->kind != RTC::Media::Kind::VIDEO)
			return 0u;

		if (!IsActive())
			return 0u;

		return this->priority;
	}

	uint32_t SimpleConsumer::IncreaseLayer(uint32_t bitrate, bool /*considerLoss*/)
	{
		//MS_TRACE();
		DEBUG_EX_LOG("bitrate = %lu", bitrate);
		//MS_ASSERT(this->externallyManagedBitrate, "bitrate is not externally managed");
		//MS_ASSERT(this->kind == RTC::Media::Kind::VIDEO, "should be video");
		//MS_ASSERT(IsActive(), "should be active");

		// If this is not the first time this method is called within the same iteration,
		// return 0 since a video SimpleConsumer does not keep state about this.
		if (this->managingBitrate)
			return 0u;

		this->managingBitrate = true;

		// Video SimpleConsumer does not really play the BWE game when. However, let's
		// be honest and try to be nice.
		auto nowMs = uv_util::GetTimeMs(); // DepLibUV::GetTimeMs();
		auto desiredBitrate = this->producerRtpStream->GetBitrate(nowMs);

		if (desiredBitrate < bitrate)
			return desiredBitrate;
		else
			return bitrate;
	}

	void SimpleConsumer::ApplyLayers()
	{
		//MS_TRACE();

		//MS_ASSERT(this->externallyManagedBitrate, "bitrate is not externally managed");
		//MS_ASSERT(this->kind == RTC::Media::Kind::VIDEO, "should be video");
		//MS_ASSERT(IsActive(), "should be active");
		DEBUG_EX_LOG("");
		this->managingBitrate = false;

		// SimpleConsumer does not play the BWE game (even if video kind).
	}

	uint32_t SimpleConsumer::GetDesiredBitrate() const
	{
		//MS_TRACE();
		DEBUG_EX_LOG("");
		//MS_ASSERT(this->externallyManagedBitrate, "bitrate is not externally managed");

		// Audio SimpleConsumer does not play the BWE game.
		if (this->kind != RTC::Media::Kind::VIDEO)
			return 0u;

		if (!IsActive())
			return 0u;

		auto nowMs = uv_util::GetTimeMs(); // DepLibUV::GetTimeMs();
		auto desiredBitrate = this->producerRtpStream->GetBitrate(nowMs);

		// If consumer.rtpParameters.encodings[0].maxBitrate was given and it's
		// greater than computed one, then use it.
		auto maxBitrate = this->rtpParameters.encodings[0].maxBitrate;

		if (maxBitrate > desiredBitrate)
			desiredBitrate = maxBitrate;

		return desiredBitrate;
	}

	void SimpleConsumer::SendRtpPacket(RTC::RtpPacket* packet)
	{
		//MS_TRACE();
		
		if (!IsActive())
			return;

		auto payloadType = packet->GetPayloadType();
		DEBUG_EX_LOG("payloadType = %d", payloadType);
		// NOTE: This may happen if this Consumer supports just some codecs of those
		// in the corresponding Producer.
		if (this->supportedCodecPayloadTypes.find(payloadType) == this->supportedCodecPayloadTypes.end())
		{
			//MS_DEBUG_DEV("payload type not supported [payloadType:%" PRIu8 "]", payloadType);

			return;
		}

		// If we need to sync, support key frames and this is not a key frame, ignore
		// the packet.
		if (this->syncRequired && this->keyFrameSupported && !packet->IsKeyFrame())
		{
			return;
		}

		// Whether this is the first packet after re-sync.
		bool isSyncPacket = this->syncRequired;

		// Sync sequence number and timestamp if required.
		if (isSyncPacket)
		{
			if (packet->IsKeyFrame())
			{
				//MS_DEBUG_TAG(rtp, "sync key frame received");
			}

			this->rtpSeqManager.Sync(packet->GetSequenceNumber() - 1);

			this->syncRequired = false;
		}

		// Update RTP seq number and timestamp.
		uint16_t seq;

		this->rtpSeqManager.Input(packet->GetSequenceNumber(), seq);

		// Save original packet fields.
		auto origSsrc = packet->GetSsrc();
		auto origSeq  = packet->GetSequenceNumber();

		// Rewrite packet.
		packet->SetSsrc(this->rtpParameters.encodings[0].ssrc);
		packet->SetSequenceNumber(seq);

		if (isSyncPacket)
		{
			DEBUG_EX_LOG( "rtp, sending sync packet [ssrc:%u, seq:%hu, ts:%u] from original [seq:%hu]",
			  packet->GetSsrc(),
			  packet->GetSequenceNumber(),
			  packet->GetTimestamp(),
			  origSeq);
		}

		// Process the packet.
		if (this->rtpStream->ReceivePacket(packet))
		{
			// Send the packet.
			this->listener->OnConsumerSendRtpPacket(this, packet);

			// May emit 'trace' event.
			EmitTraceEventRtpAndKeyFrameTypes(packet);
		}
		else
		{
			WARNING_EX_LOG("rtp, failed to send packet [ssrc:%u, seq:%hu, ts:%u] from original [seq:%hu]",
			  packet->GetSsrc(),
			  packet->GetSequenceNumber(),
			  packet->GetTimestamp(),
			  origSeq);
		}

		// Restore packet fields.
		packet->SetSsrc(origSsrc);
		packet->SetSequenceNumber(origSeq);
	}

	void SimpleConsumer::GetRtcp(
	  RTC::RTCP::CompoundPacket* packet, RTC::RtpStreamSend* rtpStream, uint64_t nowMs)
	{
		//MS_TRACE();
		DEBUG_EX_LOG("");
		//MS_ASSERT(rtpStream == this->rtpStream, "RTP stream does not match");

		if (static_cast<float>((nowMs - this->lastRtcpSentTime) * 1.15) < this->maxRtcpInterval)
			return;

		auto* report = this->rtpStream->GetRtcpSenderReport(nowMs);

		if (!report)
			return;

		packet->AddSenderReport(report);

		// Build SDES chunk for this sender.
		//auto* sdesChunk = this->rtpStream->GetRtcpSdesChunk();

		//packet->AddSdesChunk(sdesChunk);

		this->lastRtcpSentTime = nowMs;
	}

	void SimpleConsumer::NeedWorstRemoteFractionLost(
	  uint32_t /*mappedSsrc*/, uint8_t& worstRemoteFractionLost)
	{
		//MS_TRACE();
		DEBUG_EX_LOG("");
		if (!IsActive())
			return;

		auto fractionLost = this->rtpStream->GetFractionLost();

		// If our fraction lost is worse than the given one, update it.
		if (fractionLost > worstRemoteFractionLost)
			worstRemoteFractionLost = fractionLost;
	}

	void SimpleConsumer::ReceiveNack(RTC::RTCP::FeedbackRtpNackPacket* nackPacket)
	{
		//MS_TRACE();
		DEBUG_EX_LOG("");
		if (!IsActive())
			return;

		// May emit 'trace' event.
		EmitTraceEventNackType();

		this->rtpStream->ReceiveNack(nackPacket);
	}

	void SimpleConsumer::ReceiveKeyFrameRequest(
	  RTC::RTCP::FeedbackPs::MessageType messageType, uint32_t ssrc)
	{
		//MS_TRACE();
		DEBUG_EX_LOG("messageType = %d, ssrc = %lu", messageType, ssrc);
		switch (messageType)
		{
			case RTC::RTCP::FeedbackPs::MessageType::PLI:
			{
				EmitTraceEventPliType(ssrc);

				break;
			}

			case RTC::RTCP::FeedbackPs::MessageType::FIR:
			{
				EmitTraceEventFirType(ssrc);

				break;
			}

			default:;
		}

		this->rtpStream->ReceiveKeyFrameRequest(messageType);

		if (IsActive())
			RequestKeyFrame();
	}

	void SimpleConsumer::ReceiveRtcpReceiverReport(RTC::RTCP::ReceiverReport* report)
	{
		//MS_TRACE();
		DEBUG_EX_LOG("");
		this->rtpStream->ReceiveRtcpReceiverReport(report);
	}

	uint32_t SimpleConsumer::GetTransmissionRate(uint64_t nowMs)
	{
		//MS_TRACE();
		DEBUG_EX_LOG("");
		if (!IsActive())
			return 0u;

		return this->rtpStream->GetBitrate(nowMs);
	}

	float SimpleConsumer::GetRtt() const
	{
		//MS_TRACE();
		DEBUG_EX_LOG("");
		return this->rtpStream->GetRtt();
	}

	void SimpleConsumer::UserOnTransportConnected()
	{
	//	MS_TRACE();
		DEBUG_EX_LOG("");
		this->syncRequired = true;

		if (IsActive())
			RequestKeyFrame();
	}

	void SimpleConsumer::UserOnTransportDisconnected()
	{
		//MS_TRACE();
		DEBUG_EX_LOG("");
		this->rtpStream->Pause();
	}

	void SimpleConsumer::UserOnPaused()
	{
		//MS_TRACE();
		DEBUG_EX_LOG("");
		this->rtpStream->Pause();

		if (this->externallyManagedBitrate && this->kind == RTC::Media::Kind::VIDEO)
			this->listener->OnConsumerNeedZeroBitrate(this);
	}

	void SimpleConsumer::UserOnResumed()
	{
		//MS_TRACE();
		DEBUG_EX_LOG("");
		this->syncRequired = true;

		if (IsActive())
			RequestKeyFrame();
	}

	void SimpleConsumer::CreateRtpStream()
	{
		//MS_TRACE();
		DEBUG_EX_LOG("");
		RtpEncodingParameters& encoding         = this->rtpParameters.encodings[0];
		const auto* mediaCodec = this->rtpParameters.GetCodecForEncoding(encoding);

		DEBUG_EX_LOG("rtp, [ssrc:%u, payloadType:%hhu]", encoding.ssrc, mediaCodec->payloadType);

		// Set stream params.
		RTC::RtpStream::Params params;

		params.ssrc        = encoding.ssrc;
		params.payloadType = mediaCodec->payloadType;
		params.mimeType    = mediaCodec->mimeType;
		params.clockRate   = mediaCodec->clockRate;
		params.cname       = this->rtpParameters.rtcp.cname;

		// Check in band FEC in codec parameters.
		if (mediaCodec->parameters.HasInteger("useinbandfec") && mediaCodec->parameters.GetInteger("useinbandfec") == 1)
		{
			DEBUG_EX_LOG("rtp, in band FEC enabled");

			params.useInBandFec = true;
		}

		// Check DTX in codec parameters.
		if (mediaCodec->parameters.HasInteger("usedtx") && mediaCodec->parameters.GetInteger("usedtx") == 1)
		{
			DEBUG_EX_LOG("rtp, DTX enabled");

			params.useDtx = true;
		}

		// Check DTX in the encoding.
		if (encoding.dtx)
		{
			DEBUG_EX_LOG("rtp, DTX enabled");

			params.useDtx = true;
		}

		for (const auto& fb : mediaCodec->rtcpFeedbacks)
		{
			if (!params.useNack && fb.type == "nack" && fb.parameter.empty())
			{
				DEBUG_EX_LOG("rtp, rtcp, NACK supported");

				params.useNack = true;
			}
			else if (!params.usePli && fb.type == "nack" && fb.parameter == "pli")
			{
				DEBUG_EX_LOG("rtp, rtcp, PLI supported");

				params.usePli = true;
			}
			else if (!params.useFir && fb.type == "ccm" && fb.parameter == "fir")
			{
				DEBUG_EX_LOG("rtp, rtcp, FIR supported");

				params.useFir = true;
			}
		}

		// Create a RtpStreamSend for sending a single media stream.
		size_t bufferSize = params.useNack ? 600u : 0u;

		this->rtpStream = new RTC::RtpStreamSend(this, params, bufferSize);
		this->rtpStreams.push_back(this->rtpStream);

		// If the Consumer is paused, tell the RtpStreamSend.
		if (IsPaused() || IsProducerPaused())
		{
			this->rtpStream->Pause();
		}

		const auto* rtxCodec = this->rtpParameters.GetRtxCodecForEncoding(encoding);

		if (rtxCodec && encoding.hasRtx)
		{
			this->rtpStream->SetRtx(rtxCodec->payloadType, encoding.rtx.ssrc);
		}
	}

	void SimpleConsumer::RequestKeyFrame()
	{
		//MS_TRACE();
		DEBUG_EX_LOG("");
		if (this->kind != RTC::Media::Kind::VIDEO)
			return;

		auto mappedSsrc = this->consumableRtpEncodings[0].ssrc;

		this->listener->OnConsumerKeyFrameRequested(this, mappedSsrc);
	}

	inline void SimpleConsumer::EmitScore() const
	{
		//MS_TRACE();

		//json data = json::object();

		//FillJsonScore(data);
		//DEBUG_EX_LOG("data = %s", data.dump().c_str());
		//Channel::ChannelNotifier::Emit(this->id, "score", data);
	}

	inline void SimpleConsumer::OnRtpStreamScore(
	  RTC::RtpStream* /*rtpStream*/, uint8_t /*score*/, uint8_t /*previousScore*/)
	{
		//MS_TRACE();
		DEBUG_EX_LOG("");
		// Emit the score event.
		EmitScore();
	}

	inline void SimpleConsumer::OnRtpStreamRetransmitRtpPacket(
	  RTC::RtpStreamSend* /*rtpStream*/, RTC::RtpPacket* packet)
	{
		//MS_TRACE();
		DEBUG_EX_LOG("");
		this->listener->OnConsumerRetransmitRtpPacket(this, packet);

		// May emit 'trace' event.
		EmitTraceEventRtpAndKeyFrameTypes(packet, this->rtpStream->HasRtx());
	}
} // namespace RTC

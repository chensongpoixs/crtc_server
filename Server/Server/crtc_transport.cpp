﻿/***********************************************************************************************
created: 		2023-02-03

author:			chensong

purpose:		crtc_transport


************************************************************************************************/
#include "crtc_transport.h"
#include "clog.h"

#include "ctransport_util.h"
#include "crtc_track_description.h"
#include "ccfg.h"

#include "cdigit2str.h"
#include "crandom.h"
#include "cdtls_certificate.h"
#include <set>
#include "csocket_util.h"
#include  "ctransport_mgr.h"
#include "crtc_stun_packet.h"

namespace chen {
	crtc_transport::~crtc_transport()
	{
		int32  count = 34;
		NORMAL_EX_LOG("%" PRIu32 "", count);
	}
	bool crtc_transport::init(const crtc_sdp & remote_sdp, const crtc_sdp & local_sdp)
	{
		m_remote_sdp = remote_sdp;
		m_local_sdp = local_sdp;
		
		ccandidate candidate =  m_local_sdp.get_candidate();

		m_update_socket_ptr = new cudp_socket(this, candidate.m_ip, candidate.m_port);


		return true;
	}
	void crtc_transport::update(uint32 uDeltaTime)
	{
	}
	void crtc_transport::destroy()
	{
	}
	void crtc_transport::OnPacketReceived(cudp_socket * socket, const uint8_t * data, size_t len, const sockaddr * remoteAddr)
	{
		// Increase receive transmission.
		//RTC::Transport::DataReceived(len);

		// Check if it's STUN.
		if (crtc_stun_packet::is_stun(data, len))
		{
			//<<<<<<< HEAD
			//			DEBUG_EX_LOG("stun");
			//=======
			//DEBUG_EX_ID_LOG("stun");
			//>>>>>>> d40fa1c367378f962a8c8dd093974a106997055a
			//OnStunDataReceived(tuple, data, len);
		}
		// Check if it's RTCP.
		else if (RTC::RTCP::Packet::IsRtcp(data, len))
		{
			//<<<<<<< HEAD
			//			DEBUG_EX_LOG("IsRtcp");
			//=======
			//DEBUG_EX_ID_LOG("IsRtcp");
			//>>>>>>> d40fa1c367378f962a8c8dd093974a106997055a
			//OnRtcpDataReceived(tuple, data, len);
		}
		// Check if it's RTP.
		else if (RTC::RtpPacket::IsRtp(data, len))
		{
			//<<<<<<< HEAD
			//			DEBUG_EX_LOG("IsRtp");
			//=======
			//DEBUG_EX_ID_LOG("IsRtp");
			//>>>>>>> d40fa1c367378f962a8c8dd093974a106997055a
			//OnRtpDataReceived(tuple, data, len);
		}
		// Check if it's DTLS.
		else if (RTC::DtlsTransport::IsDtls(data, len))
		{
			//<<<<<<< HEAD
			//			DEBUG_EX_LOG("IsDtls");
			//=======
			//<<<<<<< HEAD
			//			DEBUG_EX_LOG("IsDtls");
			//=======
			//DEBUG_EX_ID_LOG("IsDtls"); // 这边修改DTLS的状态的哈 ？？
//>>>>>>> 69463cce016535ae4b8531ff725a35bc270954e5
//>>>>>>> d40fa1c367378f962a8c8dd093974a106997055a
			//OnDtlsDataReceived(tuple, data, len);
		}
		else
		{
			//<<<<<<< HEAD
			//			DEBUG_EX_LOG("error type");
			//=======
			//DEBUG_EX_ID_LOG("error type");
			//>>>>>>> d40fa1c367378f962a8c8dd093974a106997055a
			WARNING_EX_LOG("ignoring received packet of unknown type");
		}
	}
	void crtc_transport::OnUdpSocketPacketReceived(cudp_socket * socket, const uint8_t * data, size_t len, const sockaddr * remoteAddr)
	{
		OnPacketReceived(socket, data, len, remoteAddr);
	}
	void crtc_transport::_on_stun_data_received(cudp_socket * socket, const uint8_t * data, size_t len, const sockaddr * remoteAddr)
	{

		crtc_stun_packet stun_packet;
		if (0 != stun_packet.decode((const char *)(data), len) )
		{
			WARNING_EX_LOG("stun decode packet failed !!!");
			return ;
		}
	}
	//bool crtc_transport::_negotiate_publish_capability(crtc_source_description * stream_desc)
	//{
	// 
	//	 
	//	if (!stream_desc) 
	//	{
	//		ERROR_EX_LOG("stream description is NULL");
	//		return false;
	//	//	return srs_error_new(ERROR_RTC_SDP_EXCHANGE, "stream description is NULL");
	//	}

	//	bool nack_enabled = true;
	//	bool twcc_enabled = true;
	//	// TODO: FIME: Should check packetization-mode=1 also.
	//	bool has_42e01f =  sdp_has_h264_profile(m_remote_sdp, "42e01f");

	//	// How many video descriptions we have parsed.
	//	int32 nn_any_video_parsed = 0;

	//	for (int32 i = 0; i < (int32)m_remote_sdp.m_media_descs.size(); ++i) 
	//	{
	//		const cmedia_desc& remote_media_desc = m_remote_sdp.m_media_descs.at(i);

	//		if (remote_media_desc.is_video())
	//		{
	//			nn_any_video_parsed++;
	//		}

	//	/*	SrsRtcTrackDescription* track_desc = new SrsRtcTrackDescription();
	//		SrsAutoFree(SrsRtcTrackDescription, track_desc);*/
	//		crtc_track_description track_desc;
	//		 track_desc.set_direction("recvonly");
	//		 track_desc.set_mid(remote_media_desc.m_mid);
	//		// Whether feature enabled in remote extmap.
	//		int32 remote_twcc_id = 0;
	//		if (true) 
	//		{
	//			std::map<int32, std::string> extmaps = remote_media_desc.get_extmaps();
	//			for (std::map<int32, std::string>::iterator it = extmaps.begin(); it != extmaps.end(); ++it) 
	//			{
	//				if (it->second == kTWCCExt)
	//				{
	//					remote_twcc_id = it->first;
	//					break;
	//				}
	//			}
	//		}

	//		if (twcc_enabled && remote_twcc_id) 
	//		{
	//			track_desc.add_rtp_extension_desc(remote_twcc_id, kTWCCExt);
	//		}

	//		if (remote_media_desc.is_audio())
	//		{
	//			// Update the ruc, which is about user specified configuration.
	//			//ruc->audio_before_video_ = !nn_any_video_parsed;

	//			// TODO: check opus format specific param
	//			std::vector<cmedia_payload_type> payloads = remote_media_desc.find_media_with_encoding_name("opus");
	//			if (payloads.empty())
	//			{
	//				ERROR_EX_LOG("no valid found opus payload type");
	//				return false; 
	//			}

	//			for (int32 j = 0; j < (int32)payloads.size(); j++) 
	//			{
	//				const cmedia_payload_type& payload = payloads.at(j);

	//				// if the payload is opus, and the encoding_param_ is channel
	//				caudio_payload* audio_payload = new caudio_payload(payload.m_payload_type, payload.m_encoding_name, payload.m_clock_rate, ::atol(payload.m_encoding_param.c_str()));
	//				audio_payload->set_opus_param_desc(payload.m_format_specific_param);
	//				// AudioPayload* audio_payload = new SrsAudioPayload(payload.m_payload_type, payload.m_encoding_name, payload.m_clock_rate, ::atol(payload.m_encoding_param.c_str()));
	//				//audio_payload->set_opus_param_desc(payload.format_specific_param_);

	//				// TODO: FIXME: Only support some transport algorithms.
	//				for (int32 k = 0; k < (int32)payload.m_rtcp_fb.size(); ++k)
	//				{
	//					const std::string& rtcp_fb = payload.m_rtcp_fb.at(k);

	//					// 掉包重传的协议
	//					if (nack_enabled) 
	//					{
	//						if (rtcp_fb == "nack" || rtcp_fb == "nack pli") 
	//						{
	//							audio_payload->m_rtcp_fbs.push_back(rtcp_fb);
	//						}
	//					}
	//					// 网络带宽评估的协议
	//					if (twcc_enabled && remote_twcc_id) 
	//					{
	//						if (rtcp_fb == "transport-cc")
	//						{
	//							audio_payload->m_rtcp_fbs.push_back(rtcp_fb);
	//						}
	//					}
	//				}

	//				track_desc.m_type = "audio";
	//				track_desc.set_codec_payload(audio_payload);
	//				// Only choose one match opus codec.
	//				break;
	//			}
	//		}
	//		//else if (remote_media_desc.is_video() && ruc->codec_ == "av1") {
	//		//	std::vector<SrsMediaPayloadType> payloads = remote_media_desc.find_media_with_encoding_name("AV1");
	//		//	if (payloads.empty()) {
	//		//		// Be compatible with the Chrome M96, still check the AV1X encoding name
	//		//		// @see https://bugs.chromium.org/p/webrtc/issues/detail?id=13166
	//		//		payloads = remote_media_desc.find_media_with_encoding_name("AV1X");
	//		//	}
	//		//	if (payloads.empty()) {
	//		//		return srs_error_new(ERROR_RTC_SDP_EXCHANGE, "no found valid AV1 payload type");
	//		//	}

	//		//	for (int j = 0; j < (int)payloads.size(); j++) {
	//		//		const SrsMediaPayloadType& payload = payloads.at(j);

	//		//		// Generate video payload for av1.
	//		//		SrsVideoPayload* video_payload = new SrsVideoPayload(payload.payload_type_, payload.encoding_name_, payload.clock_rate_);

	//		//		// TODO: FIXME: Only support some transport algorithms.
	//		//		for (int k = 0; k < (int)payload.rtcp_fb_.size(); ++k) {
	//		//			const string& rtcp_fb = payload.rtcp_fb_.at(k);

	//		//			if (nack_enabled) {
	//		//				if (rtcp_fb == "nack" || rtcp_fb == "nack pli") {
	//		//					video_payload->rtcp_fbs_.push_back(rtcp_fb);
	//		//				}
	//		//			}
	//		//			if (twcc_enabled && remote_twcc_id) {
	//		//				if (rtcp_fb == "transport-cc") {
	//		//					video_payload->rtcp_fbs_.push_back(rtcp_fb);
	//		//				}
	//		//			}
	//		//		}

	//		//		track_desc->type_ = "video";
	//		//		track_desc->set_codec_payload((SrsCodecPayload*)video_payload);
	//		//		break;
	//		//	}
	//		//}
	//		else if (remote_media_desc.is_video()) 
	//		{
	//			std::vector<cmedia_payload_type> payloads = remote_media_desc.find_media_with_encoding_name("H264");
	//			if (payloads.empty())
	//			{
	//				ERROR_EX_LOG("no found valid H.264 payload type");
	//				return false;
	//				//return srs_error_new(ERROR_RTC_SDP_EXCHANGE, "no found valid H.264 payload type");
	//			}

	//			std::deque<cmedia_payload_type> backup_payloads;
	//			for (int32 j = 0; j < (int32)payloads.size(); j++) 
	//			{
	//				const cmedia_payload_type& payload = payloads.at(j);

	//				if (payload.m_format_specific_param.empty())
	//				{
	//					backup_payloads.push_front(payload);
	//					continue;
	//				}
	//				ch264_specific_param h264_param;
	//				if (( parse_h264_fmtp(payload.m_format_specific_param, h264_param)) != 0) 
	//				{

	//					//srs_error_reset(err); 
	//					WARNING_EX_LOG(" h264 specific param  parse h264 fmtp failed !!!");
	//					continue;
	//				}

	//				// If not exists 42e01f, we pick up any profile such as 42001f.
	//				bool profile_matched = (!has_42e01f || h264_param.m_profile_level_id == "42e01f");

	//				// Try to pick the "best match" H.264 payload type.
	//				if (profile_matched && h264_param.m_packetization_mode == "1" && h264_param.m_level_asymmerty_allow == "1") 
	//				{
	//					// if the playload is opus, and the encoding_param_ is channel

	//					cvideo_payload * video_payload = new cvideo_payload(payload.m_payload_type, payload.m_encoding_name, payload.m_clock_rate);
	//					//SrsVideoPayload* video_payload = new SrsVideoPayload(payload.payload_type_, payload.encoding_name_, payload.clock_rate_);
	//					video_payload->set_h264_param_desc(payload.m_format_specific_param);

	//					// Set the codec parameter for H.264, to make Unity happy.
	//					video_payload->m_h264_param = h264_param;

	//					// TODO: FIXME: Only support some transport algorithms.
	//					for (int32 k = 0; k < (int32)payload.m_rtcp_fb.size(); ++k) 
	//					{
	//						const std::string& rtcp_fb = payload.m_rtcp_fb.at(k);

	//						if (nack_enabled) 
	//						{
	//							if (rtcp_fb == "nack" || rtcp_fb == "nack pli") 
	//							{
	//								video_payload->m_rtcp_fbs.push_back(rtcp_fb);
	//							}
	//						}
	//						if (twcc_enabled && remote_twcc_id) 
	//						{
	//							if (rtcp_fb == "transport-cc") 
	//							{
	//								video_payload->m_rtcp_fbs.push_back(rtcp_fb);
	//							}
	//						}
	//					}

	//					track_desc.m_type = "video";
	//					track_desc.set_codec_payload(video_payload);
	//					// Only choose first match H.264 payload type.
	//					break;
	//				}

	//				backup_payloads.push_back(payload);
	//			}

	//			// Try my best to pick at least one media payload type.
	//			if (!track_desc.m_media_ptr && !backup_payloads.empty())
	//			{
	//				const cmedia_payload_type& payload = backup_payloads.front();

	//				// if the playload is opus, and the encoding_param_ is channel
	//				cvideo_payload *  video_payload = new cvideo_payload(payload.m_payload_type, payload.m_encoding_name, payload.m_clock_rate);

	//				//SrsVideoPayload* video_payload = new SrsVideoPayload(payload.payload_type_, payload.encoding_name_, payload.clock_rate_);

	//				// TODO: FIXME: Only support some transport algorithms.
	//				for (int k = 0; k < (int)payload.m_rtcp_fb.size(); ++k)
	//				{
	//					const std::string& rtcp_fb = payload.m_rtcp_fb.at(k);

	//					if (nack_enabled) 
	//					{
	//						if (rtcp_fb == "nack" || rtcp_fb == "nack pli")
	//						{
	//							video_payload->m_rtcp_fbs.push_back(rtcp_fb);
	//						}
	//					}

	//					if (twcc_enabled && remote_twcc_id) 
	//					{
	//						if (rtcp_fb == "transport-cc")
	//						{
	//							video_payload->m_rtcp_fbs.push_back(rtcp_fb);
	//						}
	//					}
	//				}

	//				track_desc.m_type = "video";
	//				track_desc.set_codec_payload( video_payload);
	//				NORMAL_EX_LOG("choose backup H.264 payload type=%d", payload.m_payload_type);
	//			}

	//			// TODO: FIXME: Support RRTR?
	//			//local_media_desc.payload_types_.back().rtcp_fb_.push_back("rrtr");
	//		}

	//		// Error if track desc is invalid, that is failed to match SDP, for example, we require H264 but no H264 found.
	//		if (track_desc.m_type.empty() || !track_desc.m_media_ptr) 
	//		{
	//			ERROR_EX_LOG("no match for track=%s, mid=%s, tracker=%s", remote_media_desc.m_type.c_str(), remote_media_desc.m_mid.c_str(), remote_media_desc.m_msid_tracker.c_str());
	//			return false;
	//			//return srs_error_new(ERROR_RTC_SDP_EXCHANGE, "no match for track=%s, mid=%s, tracker=%s", remote_media_desc.type_.c_str(), remote_media_desc.mid_.c_str(), remote_media_desc.msid_tracker_.c_str());
	//		}

	//		// TODO: FIXME: use one parse payload from sdp.
	//		track_desc.create_auxiliary_payload(remote_media_desc.find_media_with_encoding_name("red"));
	//		track_desc.create_auxiliary_payload(remote_media_desc.find_media_with_encoding_name("rtx"));
	//		track_desc.create_auxiliary_payload(remote_media_desc.find_media_with_encoding_name("ulpfec"));

	//		std::string track_id;
	//		for (int32 j = 0; j < (int32)remote_media_desc.m_ssrc_infos.size(); ++j) 
	//		{
	//			const cssrc_info& ssrc_info = remote_media_desc.m_ssrc_infos.at(j);

	//			// ssrc have same track id, will be description in the same track description.
	//			if (track_id != ssrc_info.m_msid_tracker) 
	//			{
	//				//SrsRtcTrackDescription* track_desc_copy = track_desc->copy();
	//				crtc_track_description * track_desc_copy = track_desc.copy();
	//				 track_desc_copy->m_ssrc = ssrc_info.m_ssrc;
	//				 track_desc_copy->m_id = ssrc_info.m_msid_tracker;
	//				 track_desc_copy->m_msid = ssrc_info.m_msid;

	//				if (remote_media_desc.is_audio()  && !stream_desc->m_audio_track_desc_ptr )
	//				{
	//					stream_desc->m_audio_track_desc_ptr = track_desc_copy;
	//				}
	//				else if (remote_media_desc.is_video())
	//				{
	//					stream_desc->m_video_track_descs.push_back(track_desc_copy);
	//				}
	//				else
	//				{
	//					delete track_desc_copy;
	//					track_desc_copy = NULL;
	//					NORMAL_EX_LOG("not find track_id  delete  track_desc_copy object [track_id = %s][ssrc_info.m_msid_tracker = %s]", track_id.c_str(), ssrc_info.m_msid_tracker.c_str());
	//				//	srs_freep(track_desc_copy);
	//				}
	//			}
	//			track_id = ssrc_info.m_msid_tracker;
	//		}

	//		// set track fec_ssrc and rtx_ssrc
	//		for (int32 j = 0; j < (int32)remote_media_desc.m_ssrc_groups.size(); ++j) 
	//		{
	//			const cssrc_group& ssrc_group = remote_media_desc.m_ssrc_groups.at(j);
	//			crtc_track_description *track_desc = stream_desc->find_track_description_by_ssrc(ssrc_group.m_ssrcs[0]);
	//			//SrsRtcTrackDescription* track_desc = stream_desc->find_track_description_by_ssrc(ssrc_group.ssrcs_[0]);
	//			if (!track_desc) 
	//			{
	//				continue;
	//			}

	//			if (ssrc_group.m_semantic == "FID") 
	//			{
	//				track_desc->set_rtx_ssrc(ssrc_group.m_ssrcs[1]);
	//			}
	//			else if (ssrc_group.m_semantic == "FEC")
	//			{
	//				track_desc->set_fec_ssrc(ssrc_group.m_ssrcs[1]);
	//			}
	//		}
	//	}

	//	return true;
	//}
	//bool crtc_transport::_generate_publish_local_sdp(crtc_sdp & local_sdp, crtc_source_description * stream_desc, bool unified_plan, bool audio_before_video)
	//{
	//	//srs_error_t err = srs_success;
	//	int32 err = 0;
	//	if (!stream_desc) 
	//	{
	//		ERROR_EX_LOG("stream description is NULL");
	//		return false;
	//		//return srs_error_new(ERROR_RTC_SDP_EXCHANGE, "stream description is NULL");
	//	}

	//	local_sdp.m_version = "0";

	//	local_sdp.m_username = MEDIA_RTC_SERVER;
	//	local_sdp.m_session_id = digit2str_dec((int64_t)this);
	//	local_sdp.m_session_version = "2";
	//	local_sdp.m_nettype = "IN";
	//	local_sdp.m_addrtype = "IP4";
	//	local_sdp.m_unicast_address = "0.0.0.0";

	//	local_sdp.m_session_name = "RTCPublishSession";

	//	local_sdp.m_msid_semantic = "WMS";
	//	// TODO@chensong 2023-03-08   default -> video stream address url  
	//	std::string stream_id = "test";;// req->app + "/" + req->stream;
	//	local_sdp.m_msids.push_back(stream_id);

	//	local_sdp.m_group_policy = "BUNDLE";

	//	if (audio_before_video) 
	//	{
	//		if ((err = _generate_publish_local_sdp_for_audio(local_sdp, stream_desc)) != 0) 
	//		{
	//			ERROR_EX_LOG("_generate_publish_local_sdp_for_audio failed !!!");
	//			return false;
	//		}
	//		if ((err = _generate_publish_local_sdp_for_video(local_sdp, stream_desc, unified_plan)) != 0) 
	//		{
	//			ERROR_EX_LOG("_generate_publish_local_sdp_for_video failed !!!");
	//			return false;
	//		}
	//	}
	//	else 
	//	{
	//		if ((err = _generate_publish_local_sdp_for_video(local_sdp, stream_desc, unified_plan)) != 0) 
	//		{
	//			ERROR_EX_LOG("_generate_publish_local_sdp_for_video failed !!!");
	//			return false;
	//		}
	//		if ((err = _generate_publish_local_sdp_for_audio(local_sdp, stream_desc)) != 0) 
	//		{
	//			ERROR_EX_LOG("_generate_publish_local_sdp_for_audio failed !!!");
	//			return false;
	//			//return srs_error_wrap(err, "audio");
	//		}
	//	}

	//	return true;
	//}
	//bool crtc_transport::_generate_publish_local_sdp_for_audio(crtc_sdp & local_sdp, crtc_source_description * stream_desc)
	//{

	//	//srs_error_t err = srs_success;

	//	// generate audio media desc
	//	if (stream_desc->m_audio_track_desc_ptr) 
	//	{
	//		crtc_track_description* audio_track = stream_desc->m_audio_track_desc_ptr;

	//		local_sdp.m_media_descs.push_back(cmedia_desc("audio"));
	//		cmedia_desc& local_media_desc = local_sdp.m_media_descs.back();

	//		local_media_desc.m_port = 9;
	//		local_media_desc.m_protos = "UDP/TLS/RTP/SAVPF";
	//		local_media_desc.m_rtcp_mux = true;
	//		local_media_desc.m_rtcp_rsize = true;

	//		local_media_desc.m_mid = audio_track->m_mid;
	//		local_sdp.m_groups.push_back(local_media_desc.m_mid);

	//		// anwer not need set stream_id and track_id;
	//		// local_media_desc.msid_ = stream_id;
	//		// local_media_desc.msid_tracker_ = audio_track->id_;
	//		local_media_desc.m_extmaps = audio_track->m_extmaps;

	//		if (audio_track->m_direction == "recvonly") 
	//		{
	//			local_media_desc.m_recvonly = true;
	//		}
	//		else if (audio_track->m_direction == "sendonly") 
	//		{
	//			local_media_desc.m_sendonly = true;
	//		}
	//		else if (audio_track->m_direction == "sendrecv") 
	//		{
	//			local_media_desc.m_sendrecv  = true;
	//		}
	//		else if (audio_track->m_direction == "inactive") 
	//		{
	//			local_media_desc.m_inactive  = true;
	//		}

	//		caudio_payload* payload = dynamic_cast<caudio_payload*>(audio_track->m_media_ptr);
	//		local_media_desc.m_payload_types.push_back(payload->generate_media_payload_type());
	//	}

	//	return true; 
	//	//return false;
	//}
	//bool crtc_transport::_generate_publish_local_sdp_for_video(crtc_sdp & local_sdp, crtc_source_description * stream_desc, bool unified_plan)
	//{
	//	//srs_error_t err = srs_success;

	//	for (int32 i = 0; i < (int32)stream_desc->m_video_track_descs.size(); ++i) {
	//		crtc_track_description* video_track = stream_desc->m_video_track_descs.at(i);

	//		local_sdp.m_media_descs.push_back(cmedia_desc("video"));
	//		cmedia_desc& local_media_desc = local_sdp.m_media_descs.back();

	//		local_media_desc.m_port = 9;
	//		local_media_desc.m_protos = "UDP/TLS/RTP/SAVPF";
	//		local_media_desc.m_rtcp_mux = true;
	//		local_media_desc.m_rtcp_rsize = true;

	//		local_media_desc.m_mid = video_track->m_mid;
	//		local_sdp.m_groups.push_back(local_media_desc.m_mid);

	//		// anwer not need set stream_id and track_id;
	//		//local_media_desc.msid_ = stream_id;
	//		//local_media_desc.msid_tracker_ = video_track->id_;
	//		local_media_desc.m_extmaps = video_track->m_extmaps;

	//		if (video_track->m_direction == "recvonly") 
	//		{
	//			local_media_desc.m_recvonly = true;
	//		}
	//		else if (video_track->m_direction == "sendonly")
	//		{
	//			local_media_desc.m_sendonly = true;
	//		}
	//		else if (video_track->m_direction == "sendrecv")
	//		{
	//			local_media_desc.m_sendrecv = true;
	//		}
	//		else if (video_track->m_direction == "inactive")
	//		{
	//			local_media_desc.m_inactive = true;
	//		}

	//		cvideo_payload* payload =  dynamic_cast<cvideo_payload*>(video_track->m_media_ptr);
	//		local_media_desc.m_payload_types.push_back(payload->generate_media_payload_type());

	//		if (video_track->m_red_ptr) 
	//		{
	//			cred_paylod* payload =  dynamic_cast<cred_paylod*>(video_track->m_red_ptr);
	//			local_media_desc.m_payload_types.push_back(payload->generate_media_payload_type());
	//		}

	//		if (!unified_plan) 
	//		{
	//			// For PlanB, only need media desc info, not ssrc info;
	//			break;
	//		}
	//	}
	//	return true;
	//}





}
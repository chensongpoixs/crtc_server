﻿/***********************************************************************************************
created: 		2023-01-15

author:			chensong

purpose:		api_rtc_publish


************************************************************************************************/
#include "capi_rtc_publish.h"
#include "crtc_user_config.h"
#include "cshare_proto_error.h"
#include "crtc_source_description.h"
#include "crtc_track_description.h"
#include "csdp_util.h"
#include "ccfg.h"
#include "cdigit2str.h"
#include "ctransport_mgr.h"
#include "crandom.h"
#include "cdtls_certificate.h"
namespace chen {
	capi_rtc_publish::~capi_rtc_publish()
	{
	}
	int32 capi_rtc_publish::do_serve_client(const std::string & remote_sdp,  const std::string & roomname, const std::string & peerid, std::string & local_sdp)
	{
		 
		crtc_source_description stream_desc;
		crtc_sdp rtc_remote_sdp;
		crtc_sdp rtc_local_sdp;
		// RFC 4145 - http://tools.ietf.org/html/rfc4145#section-4
		// 'active':  The endpoint will initiate an outgoing connection.
		// 'passive': The endpoint will accept an incoming connection.
		// 'actpass': The endpoint is willing to accept an incoming
		//            connection or to initiate an outgoing connection.
		/*a = setup 主要是表示dtls的协商过程中角色的问题，谁是客户端，谁是服务器
		a = setup:actpass 既可以是客户端，也可以是服务器
		a = setup : active 客户端
		a = setup : passive 服务器
		由客户端先发起client hello*/
		rtc_local_sdp.m_session_config.m_dtls_role = "passive";
		rtc_local_sdp.m_session_config.m_dtls_version = "auto";

		rtc_remote_sdp.parse(remote_sdp);
		_negotiate_publish_capability(rtc_remote_sdp, &stream_desc);

		_generate_publish_local_sdp(roomname, peerid, rtc_local_sdp, &stream_desc, rtc_remote_sdp.is_unified(), true);

		// All tracks default as inactive, so we must enable them.
	//	session->set_all_tracks_status(req->get_stream_url(), ruc->publish_, true);

		std::string local_pwd = c_rand.rand_str(32);
		std::string local_ufrag = "";
		// TODO: FIXME: Rename for a better name, it's not an username.
		std::string username = "";
		while (true)
		{
			local_ufrag = c_rand.rand_str(8);

			username = local_ufrag + ":" + rtc_remote_sdp.get_ice_ufrag();
			if (!g_transport_mgr.find_username(username))
			{
				break;
			}
		}

		rtc_local_sdp.set_ice_ufrag(local_ufrag);
		rtc_local_sdp.set_ice_pwd(local_pwd);
		rtc_local_sdp.set_fingerprint_algo("sha-256");
		rtc_local_sdp.set_fingerprint(g_dtls_certificate.get_fingerprint());

		// We allows to mock the eip of server.
		if (true)
		{
			static  int32 udp_port = 10000;//_srs_config->get_rtc_server_listen();
			int32 tcp_port = 0;//_srs_config->get_rtc_server_tcp_listen();
			std::string protocol = "udp";//_srs_config->get_rtc_server_protocol();

			//std::set<std::string> candidates = {"192.168.1.175"};;// = discover_candidates(ruc);
			//for (std::set<std::string>::iterator it = candidates.begin(); it != candidates.end(); ++it) 
			{
				std::string hostname = "192.168.1.175";
				int32 uport = udp_port++;
				//parse_hostport(*it, hostname, uport);
				//int32 tport = tcp_port; 
				//parse_hostport(*it, hostname, tport);
//
				if (protocol == "udp")
				{
					rtc_local_sdp.add_candidate("udp", hostname, uport, "host");
				}
				/*else if (protocol == "tcp")
				{
					m_local_sdp.add_candidate("tcp", hostname, tport, "host");
				}*/
				else
				{
					WARNING_EX_LOG("[protocol = %s][hostname = %s][uport = %u]", protocol.c_str(), hostname.c_str(), uport);
					/*m_local_sdp.add_candidate("udp", hostname, uport, "host");
					m_local_sdp.add_candidate("tcp", hostname, tport, "host");*/
				}
			}

			//std::vector<std::string> v = std::vector<std::string>(candidates.begin(), candidates.end());
			NORMAL_EX_LOG("RTC: Use candidates  = , protocol=%s", /*srs_join_vector_string(v, ", ").c_str(),*/ protocol.c_str());
		}

		// Setup the negotiate DTLS by config.
		rtc_local_sdp.m_session_negotiate = rtc_local_sdp.m_session_config;

		// Setup the negotiate DTLS role.
		if (rtc_remote_sdp.get_dtls_role() == "active")
		{
			rtc_local_sdp.m_session_negotiate.m_dtls_role = "passive";
		}
		else if (rtc_remote_sdp.get_dtls_role() == "passive")
		{
			rtc_local_sdp.m_session_negotiate.m_dtls_role = "active";
		}
		else if (rtc_remote_sdp.get_dtls_role() == "actpass")
		{
			rtc_local_sdp.m_session_negotiate.m_dtls_role = rtc_local_sdp.m_session_config.m_dtls_role;
		}
		else
		{
			// @see: https://tools.ietf.org/html/rfc4145#section-4.1
			// The default value of the setup attribute in an offer/answer exchange
			// is 'active' in the offer and 'passive' in the answer.
			rtc_local_sdp.m_session_negotiate.m_dtls_role = "passive";
		}
		rtc_local_sdp.set_dtls_role(rtc_local_sdp.m_session_negotiate.m_dtls_role);

		//session->set_remote_sdp(ruc->remote_sdp_);
		// We must setup the local SDP, then initialize the session object.
		//session->set_local_sdp(local_sdp);
	//	session->set_state_as_waiting_stun();

		crtc_transport * transport_ptr = new crtc_transport();

		transport_ptr->init(rtc_remote_sdp, rtc_local_sdp);
		transport_ptr->set_state_as_waiting_stun();
		// Before session initialize, we must setup the local SDP.
		//if ((err = session->initialize(req, ruc->dtls_, ruc->srtp_, username)) != 0) 
		{
			//	return srs_error_wrap(err, "init");
		}

		// We allows username is optional, but it never empty here.
		//_srs_rtc_manager->add_with_name(username, session);
		g_transport_mgr.insert_username(username, transport_ptr);



		std::ostringstream    sdp;
		rtc_local_sdp.encode(sdp);


		local_sdp = sdp.str();



		NORMAL_EX_LOG("sdp info = %s", sdp.str().c_str());


		/*FILE * out_sdp_file = ::fopen("api_rtc_sdp.sdp", "wb+");
		std::ostringstream    sdp;
		rtc_local_sdp.encode(sdp);
		::fprintf(out_sdp_file, "%s=============\n", sdp.str().c_str());
		::fflush(out_sdp_file);
		::fclose(out_sdp_file);
		out_sdp_file = NULL;*/


		return 0;
	}
	bool capi_rtc_publish::_negotiate_publish_capability(crtc_sdp& remote_sdp, crtc_source_description * stream_desc)
	{
		if (!stream_desc)
		{
			ERROR_EX_LOG("stream description is NULL");
			return false;
			//	return srs_error_new(ERROR_RTC_SDP_EXCHANGE, "stream description is NULL");
		}

		bool nack_enabled = true;
		bool twcc_enabled = true;
		// TODO: FIME: Should check packetization-mode=1 also.
		bool has_42e01f = sdp_has_h264_profile(remote_sdp, "42e01f");

		// How many video descriptions we have parsed.
		int32 nn_any_video_parsed = 0;

		for (int32 i = 0; i < (int32)remote_sdp.m_media_descs.size(); ++i)
		{
			const cmedia_desc& remote_media_desc = remote_sdp.m_media_descs.at(i);

			if (remote_media_desc.is_video())
			{
				nn_any_video_parsed++;
			}

			/*	SrsRtcTrackDescription* track_desc = new SrsRtcTrackDescription();
				SrsAutoFree(SrsRtcTrackDescription, track_desc);*/
			crtc_track_description track_desc;
			track_desc.set_direction("recvonly");
			track_desc.set_mid(remote_media_desc.m_mid);
			// Whether feature enabled in remote extmap.
			int32 remote_twcc_id = 0;
			if (true)
			{
				std::map<int32, std::string> extmaps = remote_media_desc.get_extmaps();
				for (std::map<int32, std::string>::iterator it = extmaps.begin(); it != extmaps.end(); ++it)
				{
					if (it->second == kTWCCExt)
					{
						remote_twcc_id = it->first;
						break;
					}
				}
			}

			if (twcc_enabled && remote_twcc_id)
			{
				track_desc.add_rtp_extension_desc(remote_twcc_id, kTWCCExt);
			}

			if (remote_media_desc.is_audio())
			{
				// Update the ruc, which is about user specified configuration.
				//ruc->audio_before_video_ = !nn_any_video_parsed;

				// TODO: check opus format specific param
				std::vector<cmedia_payload_type> payloads = remote_media_desc.find_media_with_encoding_name("opus");
				if (payloads.empty())
				{
					ERROR_EX_LOG("no valid found opus payload type");
					return false;
				}

				for (int32 j = 0; j < (int32)payloads.size(); j++)
				{
					const cmedia_payload_type& payload = payloads.at(j);

					// if the payload is opus, and the encoding_param_ is channel
					caudio_payload* audio_payload = new caudio_payload(payload.m_payload_type, payload.m_encoding_name, payload.m_clock_rate, ::atol(payload.m_encoding_param.c_str()));
					audio_payload->set_opus_param_desc(payload.m_format_specific_param);
					// AudioPayload* audio_payload = new SrsAudioPayload(payload.m_payload_type, payload.m_encoding_name, payload.m_clock_rate, ::atol(payload.m_encoding_param.c_str()));
					//audio_payload->set_opus_param_desc(payload.format_specific_param_);

					// TODO: FIXME: Only support some transport algorithms.
					for (int32 k = 0; k < (int32)payload.m_rtcp_fb.size(); ++k)
					{
						const std::string& rtcp_fb = payload.m_rtcp_fb.at(k);

						// 掉包重传的协议
						if (nack_enabled)
						{
							if (rtcp_fb == "nack" || rtcp_fb == "nack pli")
							{
								audio_payload->m_rtcp_fbs.push_back(rtcp_fb);
							}
						}
						// 网络带宽评估的协议
						if (twcc_enabled && remote_twcc_id)
						{
							if (rtcp_fb == "transport-cc")
							{
								audio_payload->m_rtcp_fbs.push_back(rtcp_fb);
							}
						}
					}

					track_desc.m_type = "audio";
					track_desc.set_codec_payload(audio_payload);
					// Only choose one match opus codec.
					break;
				}
			}
			//else if (remote_media_desc.is_video() && ruc->codec_ == "av1") {
			//	std::vector<SrsMediaPayloadType> payloads = remote_media_desc.find_media_with_encoding_name("AV1");
			//	if (payloads.empty()) {
			//		// Be compatible with the Chrome M96, still check the AV1X encoding name
			//		// @see https://bugs.chromium.org/p/webrtc/issues/detail?id=13166
			//		payloads = remote_media_desc.find_media_with_encoding_name("AV1X");
			//	}
			//	if (payloads.empty()) {
			//		return srs_error_new(ERROR_RTC_SDP_EXCHANGE, "no found valid AV1 payload type");
			//	}

			//	for (int j = 0; j < (int)payloads.size(); j++) {
			//		const SrsMediaPayloadType& payload = payloads.at(j);

			//		// Generate video payload for av1.
			//		SrsVideoPayload* video_payload = new SrsVideoPayload(payload.payload_type_, payload.encoding_name_, payload.clock_rate_);

			//		// TODO: FIXME: Only support some transport algorithms.
			//		for (int k = 0; k < (int)payload.rtcp_fb_.size(); ++k) {
			//			const string& rtcp_fb = payload.rtcp_fb_.at(k);

			//			if (nack_enabled) {
			//				if (rtcp_fb == "nack" || rtcp_fb == "nack pli") {
			//					video_payload->rtcp_fbs_.push_back(rtcp_fb);
			//				}
			//			}
			//			if (twcc_enabled && remote_twcc_id) {
			//				if (rtcp_fb == "transport-cc") {
			//					video_payload->rtcp_fbs_.push_back(rtcp_fb);
			//				}
			//			}
			//		}

			//		track_desc->type_ = "video";
			//		track_desc->set_codec_payload((SrsCodecPayload*)video_payload);
			//		break;
			//	}
			//}
			else if (remote_media_desc.is_video())
			{
				std::vector<cmedia_payload_type> payloads = remote_media_desc.find_media_with_encoding_name("H264");
				if (payloads.empty())
				{
					ERROR_EX_LOG("no found valid H.264 payload type");
					return false;
					//return srs_error_new(ERROR_RTC_SDP_EXCHANGE, "no found valid H.264 payload type");
				}

				std::deque<cmedia_payload_type> backup_payloads;
				for (int32 j = 0; j < (int32)payloads.size(); j++)
				{
					const cmedia_payload_type& payload = payloads.at(j);

					if (payload.m_format_specific_param.empty())
					{
						backup_payloads.push_front(payload);
						continue;
					}
					ch264_specific_param h264_param;
					if ((parse_h264_fmtp(payload.m_format_specific_param, h264_param)) != 0)
					{

						//srs_error_reset(err); 
						WARNING_EX_LOG(" h264 specific param  parse h264 fmtp failed !!!");
						continue;
					}

					// If not exists 42e01f, we pick up any profile such as 42001f.
					bool profile_matched = (!has_42e01f || h264_param.m_profile_level_id == "42e01f");

					// Try to pick the "best match" H.264 payload type.
					if (profile_matched && h264_param.m_packetization_mode == "1" && h264_param.m_level_asymmerty_allow == "1")
					{
						// if the playload is opus, and the encoding_param_ is channel

						cvideo_payload * video_payload = new cvideo_payload(payload.m_payload_type, payload.m_encoding_name, payload.m_clock_rate);
						//SrsVideoPayload* video_payload = new SrsVideoPayload(payload.payload_type_, payload.encoding_name_, payload.clock_rate_);
						video_payload->set_h264_param_desc(payload.m_format_specific_param);

						// Set the codec parameter for H.264, to make Unity happy.
						video_payload->m_h264_param = h264_param;

						// TODO: FIXME: Only support some transport algorithms.
						for (int32 k = 0; k < (int32)payload.m_rtcp_fb.size(); ++k)
						{
							const std::string& rtcp_fb = payload.m_rtcp_fb.at(k);

							if (nack_enabled)
							{
								if (rtcp_fb == "nack" || rtcp_fb == "nack pli")
								{
									video_payload->m_rtcp_fbs.push_back(rtcp_fb);
								}
							}
							if (twcc_enabled && remote_twcc_id)
							{
								if (rtcp_fb == "transport-cc")
								{
									video_payload->m_rtcp_fbs.push_back(rtcp_fb);
								}
							}
						}

						track_desc.m_type = "video";
						track_desc.set_codec_payload(video_payload);
						// Only choose first match H.264 payload type.
						break;
					}

					backup_payloads.push_back(payload);
				}

				// Try my best to pick at least one media payload type.
				if (!track_desc.m_media_ptr && !backup_payloads.empty())
				{
					const cmedia_payload_type& payload = backup_payloads.front();

					// if the playload is opus, and the encoding_param_ is channel
					cvideo_payload *  video_payload = new cvideo_payload(payload.m_payload_type, payload.m_encoding_name, payload.m_clock_rate);

					//SrsVideoPayload* video_payload = new SrsVideoPayload(payload.payload_type_, payload.encoding_name_, payload.clock_rate_);

					// TODO: FIXME: Only support some transport algorithms.
					for (int k = 0; k < (int)payload.m_rtcp_fb.size(); ++k)
					{
						const std::string& rtcp_fb = payload.m_rtcp_fb.at(k);

						if (nack_enabled)
						{
							if (rtcp_fb == "nack" || rtcp_fb == "nack pli")
							{
								video_payload->m_rtcp_fbs.push_back(rtcp_fb);
							}
						}

						if (twcc_enabled && remote_twcc_id)
						{
							if (rtcp_fb == "transport-cc")
							{
								video_payload->m_rtcp_fbs.push_back(rtcp_fb);
							}
						}
					}

					track_desc.m_type = "video";
					track_desc.set_codec_payload(video_payload);
					NORMAL_EX_LOG("choose backup H.264 payload type=%d", payload.m_payload_type);
				}

				// TODO: FIXME: Support RRTR?
				//local_media_desc.payload_types_.back().rtcp_fb_.push_back("rrtr");
			}

			// Error if track desc is invalid, that is failed to match SDP, for example, we require H264 but no H264 found.
			if (track_desc.m_type.empty() || !track_desc.m_media_ptr)
			{
				ERROR_EX_LOG("no match for track=%s, mid=%s, tracker=%s", remote_media_desc.m_type.c_str(), remote_media_desc.m_mid.c_str(), remote_media_desc.m_msid_tracker.c_str());
				return false;
				//return srs_error_new(ERROR_RTC_SDP_EXCHANGE, "no match for track=%s, mid=%s, tracker=%s", remote_media_desc.type_.c_str(), remote_media_desc.mid_.c_str(), remote_media_desc.msid_tracker_.c_str());
			}

			// TODO: FIXME: use one parse payload from sdp.
			track_desc.create_auxiliary_payload(remote_media_desc.find_media_with_encoding_name("red"));
			track_desc.create_auxiliary_payload(remote_media_desc.find_media_with_encoding_name("rtx"));
			track_desc.create_auxiliary_payload(remote_media_desc.find_media_with_encoding_name("ulpfec"));

			std::string track_id;
			for (int32 j = 0; j < (int32)remote_media_desc.m_ssrc_infos.size(); ++j)
			{
				const cssrc_info& ssrc_info = remote_media_desc.m_ssrc_infos.at(j);

				// ssrc have same track id, will be description in the same track description.
				if (track_id != ssrc_info.m_msid_tracker)
				{
					//SrsRtcTrackDescription* track_desc_copy = track_desc->copy();
					crtc_track_description * track_desc_copy = track_desc.copy();
					track_desc_copy->m_ssrc = ssrc_info.m_ssrc;
					track_desc_copy->m_id = ssrc_info.m_msid_tracker;
					track_desc_copy->m_msid = ssrc_info.m_msid;

					if (remote_media_desc.is_audio() && !stream_desc->m_audio_track_desc_ptr)
					{
						stream_desc->m_audio_track_desc_ptr = track_desc_copy;
					}
					else if (remote_media_desc.is_video())
					{
						stream_desc->m_video_track_descs.push_back(track_desc_copy);
					}
					else
					{
						delete track_desc_copy;
						track_desc_copy = NULL;
						NORMAL_EX_LOG("not find track_id  delete  track_desc_copy object [track_id = %s][ssrc_info.m_msid_tracker = %s]", track_id.c_str(), ssrc_info.m_msid_tracker.c_str());
						//	srs_freep(track_desc_copy);
					}
				}
				track_id = ssrc_info.m_msid_tracker;
			}

			// set track fec_ssrc and rtx_ssrc
			for (int32 j = 0; j < (int32)remote_media_desc.m_ssrc_groups.size(); ++j)
			{
				const cssrc_group& ssrc_group = remote_media_desc.m_ssrc_groups.at(j);
				crtc_track_description *track_desc = stream_desc->find_track_description_by_ssrc(ssrc_group.m_ssrcs[0]);
				//SrsRtcTrackDescription* track_desc = stream_desc->find_track_description_by_ssrc(ssrc_group.ssrcs_[0]);
				if (!track_desc)
				{
					continue;
				}

				if (ssrc_group.m_semantic == "FID")
				{
					track_desc->set_rtx_ssrc(ssrc_group.m_ssrcs[1]);
				}
				else if (ssrc_group.m_semantic == "FEC")
				{
					track_desc->set_fec_ssrc(ssrc_group.m_ssrcs[1]);
				}
			}
		}

		return true;
	}
	bool capi_rtc_publish::_generate_publish_local_sdp(const std::string & roomname, const std::string & peerid, crtc_sdp & local_sdp, crtc_source_description * stream_desc, bool unified_plan, bool audio_before_video)
	{
		int32 err = 0;
		if (!stream_desc)
		{
			ERROR_EX_LOG("stream description is NULL");
			return false;
			//return srs_error_new(ERROR_RTC_SDP_EXCHANGE, "stream description is NULL");
		}

		local_sdp.m_version = "0";

		local_sdp.m_username = MEDIA_RTC_SERVER;
		local_sdp.m_session_id = digit2str_dec((int64_t)this);
		local_sdp.m_session_version = "2";
		local_sdp.m_nettype = "IN";
		local_sdp.m_addrtype = "IP4";
		local_sdp.m_unicast_address = "0.0.0.0";

		local_sdp.m_session_name = "RTCPublishSession";

		local_sdp.m_msid_semantic = "WMS";
		// TODO@chensong 2023-03-08   default -> video stream address url  
		// url [roomname + username]
		std::string stream_id = roomname + "/" + peerid; //"test";;// req->app + "/" + req->stream;

		local_sdp.m_msids.push_back(stream_id);

		local_sdp.m_group_policy = "BUNDLE";

		if (audio_before_video)
		{
			if ( ! _generate_publish_local_sdp_for_audio(local_sdp, stream_desc)  )
			{
				ERROR_EX_LOG("_generate_publish_local_sdp_for_audio failed !!!");
				return false;
			}
			if ( ! _generate_publish_local_sdp_for_video(local_sdp, stream_desc, unified_plan) )
			{
				ERROR_EX_LOG("_generate_publish_local_sdp_for_video failed !!!");
				return false;
			}
		}
		else
		{
			if (! _generate_publish_local_sdp_for_video(local_sdp, stream_desc, unified_plan) )
			{
				ERROR_EX_LOG("_generate_publish_local_sdp_for_video failed !!!");
				return false;
			}
			if (! _generate_publish_local_sdp_for_audio(local_sdp, stream_desc) )
			{
				ERROR_EX_LOG("_generate_publish_local_sdp_for_audio failed !!!");
				return false;
				//return srs_error_wrap(err, "audio");
			}
		}

		return true;
	}
	bool capi_rtc_publish::_generate_publish_local_sdp_for_audio(crtc_sdp & local_sdp, crtc_source_description * stream_desc)
	{
		// generate audio media desc
		if (stream_desc->m_audio_track_desc_ptr)
		{
			crtc_track_description* audio_track = stream_desc->m_audio_track_desc_ptr;

			local_sdp.m_media_descs.push_back(cmedia_desc("audio"));
			cmedia_desc& local_media_desc = local_sdp.m_media_descs.back();

			local_media_desc.m_port = 9;
			local_media_desc.m_protos = "UDP/TLS/RTP/SAVPF";
			local_media_desc.m_rtcp_mux = true;
			local_media_desc.m_rtcp_rsize = true;

			local_media_desc.m_mid = audio_track->m_mid;
			local_sdp.m_groups.push_back(local_media_desc.m_mid);

			// anwer not need set stream_id and track_id;
			// local_media_desc.msid_ = stream_id;
			// local_media_desc.msid_tracker_ = audio_track->id_;
			local_media_desc.m_extmaps = audio_track->m_extmaps;

			if (audio_track->m_direction == "recvonly")
			{
				local_media_desc.m_recvonly = true;
			}
			else if (audio_track->m_direction == "sendonly")
			{
				local_media_desc.m_sendonly = true;
			}
			else if (audio_track->m_direction == "sendrecv")
			{
				local_media_desc.m_sendrecv = true;
			}
			else if (audio_track->m_direction == "inactive")
			{
				local_media_desc.m_inactive = true;
			}

			caudio_payload* payload = dynamic_cast<caudio_payload*>(audio_track->m_media_ptr);
			local_media_desc.m_payload_types.push_back(payload->generate_media_payload_type());
		}

		return true;
	}
	bool capi_rtc_publish::_generate_publish_local_sdp_for_video(crtc_sdp & local_sdp, crtc_source_description * stream_desc, bool unified_plan)
	{
		for (int32 i = 0; i < (int32)stream_desc->m_video_track_descs.size(); ++i) {
			crtc_track_description* video_track = stream_desc->m_video_track_descs.at(i);

			local_sdp.m_media_descs.push_back(cmedia_desc("video"));
			cmedia_desc& local_media_desc = local_sdp.m_media_descs.back();

			local_media_desc.m_port = 9;
			local_media_desc.m_protos = "UDP/TLS/RTP/SAVPF";
			local_media_desc.m_rtcp_mux = true;
			local_media_desc.m_rtcp_rsize = true;

			local_media_desc.m_mid = video_track->m_mid;
			local_sdp.m_groups.push_back(local_media_desc.m_mid);

			// anwer not need set stream_id and track_id;
			//local_media_desc.msid_ = stream_id;
			//local_media_desc.msid_tracker_ = video_track->id_;
			local_media_desc.m_extmaps = video_track->m_extmaps;

			if (video_track->m_direction == "recvonly")
			{
				local_media_desc.m_recvonly = true;
			}
			else if (video_track->m_direction == "sendonly")
			{
				local_media_desc.m_sendonly = true;
			}
			else if (video_track->m_direction == "sendrecv")
			{
				local_media_desc.m_sendrecv = true;
			}
			else if (video_track->m_direction == "inactive")
			{
				local_media_desc.m_inactive = true;
			}

			cvideo_payload* payload = dynamic_cast<cvideo_payload*>(video_track->m_media_ptr);
			local_media_desc.m_payload_types.push_back(payload->generate_media_payload_type());

			if (video_track->m_red_ptr)
			{
				cred_paylod* payload = dynamic_cast<cred_paylod*>(video_track->m_red_ptr);
				local_media_desc.m_payload_types.push_back(payload->generate_media_payload_type());
			}

			if (!unified_plan)
			{
				// For PlanB, only need media desc info, not ssrc info;
				break;
			}
		}
		return true;
		//return false;
	}
	int32 capi_rtc_publish::_serve_client(crtc_user_config * ruc)
	{

		int32 err = 0;
		if ((err = _check_remote_sdp(ruc->m_remote_sdp)) != 0) 
		{
			WARNING_EX_LOG("remote sdp check failed");
			return err;
			//return error_wrap(err, "remote sdp check failed");
		}

		crtc_sdp local_sdp;


		// TODO: FIXME: move to create_session.
		// Config for SDP and session.
		/*
		a = setup 主要是表示dtls的协商过程中角色的问题，谁是客户端，谁是服务器
		a = setup : actpass 既可以是客户端，也可以是服务器
		a = setup : active 客户端
		a = setup : passive 服务器
		由客户端先发起client hello

		*/
		local_sdp.m_session_config.m_dtls_role =  "passive";
		local_sdp.m_session_config.m_dtls_version = "auto";



		return err;
	}

	int32 capi_rtc_publish::_check_remote_sdp(const crtc_sdp & remote_sdp)
	{
		int32 err = 0;

		if (remote_sdp.m_group_policy != "BUNDLE")
		{
			ERROR_EX_LOG("now only support BUNDLE, group policy=%s", remote_sdp.m_group_policy.c_str());
			return EShareRtcSdpExchange;
			//return error_new(ERROR_RTC_SDP_EXCHANGE, "now only support BUNDLE, group policy=%s", remote_sdp.m_group_policy.c_str());
		}

		if (remote_sdp.m_media_descs.empty()) 
		{
			ERROR_EX_LOG("no media descriptions");
			return EShareRtcSdpExchange;
			//return  error_new(ERROR_RTC_SDP_EXCHANGE, "no media descriptions");
		}

		for (std::vector<cmedia_desc>::const_iterator iter = remote_sdp.m_media_descs.begin(); iter != remote_sdp.m_media_descs.end(); ++iter) 
		{
			if (iter->m_type != "audio" && iter->m_type != "video") 
			{
				ERROR_EX_LOG("unsupport media type=%s", iter->m_type.c_str());
				return EShareRtcSdpExchange;
				//return error_new(ERROR_RTC_SDP_EXCHANGE, "unsupport media type=%s", iter->type_.c_str());
			}

			if (!iter->m_rtcp_mux) 
			{
				ERROR_EX_LOG("now only suppor rtcp-mux");
				return EShareRtcSdpExchange;
				//return  error_new(ERROR_RTC_SDP_EXCHANGE, "now only suppor rtcp-mux");
			}

			if (iter->m_recvonly)
			{
				ERROR_EX_LOG("publish API only support sendrecv/sendonly");
				return EShareRtcSdpExchange;
				//return srs_error_new(ERROR_RTC_SDP_EXCHANGE, "publish API only support sendrecv/sendonly");
			}
		}

		return err;
		 
	}

}
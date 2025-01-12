﻿/***********************************************************************************************
created: 		2023-02-13

author:			chensong

purpose:		api_rtc_publish


************************************************************************************************/

#include "crtc_publisher.h"
#include "cdummysetsession_description_observer.h"
#include "absl/memory/memory.h"
#include "absl/types/optional.h"
#include "api/audio/audio_mixer.h"
#include "api/audio_codecs/audio_decoder_factory.h"
#include "api/audio_codecs/audio_encoder_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/audio_options.h"
#include "api/create_peerconnection_factory.h"
#include "api/rtp_sender_interface.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_encoder_factory.h"
 
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
#include "p2p/base/port_allocator.h"
#include "pc/video_track_source.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/ref_counted_object.h"
#include "rtc_base/rtc_certificate_generator.h"

#include "absl/memory/memory.h"
#include "absl/types/optional.h"
#include "api/audio/audio_mixer.h"
#include "api/audio_codecs/audio_decoder_factory.h"
#include "api/audio_codecs/audio_encoder_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/audio_options.h"
#include "api/create_peerconnection_factory.h"
#include "api/rtp_sender_interface.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_encoder_factory.h"
//#include "examples/peerconnection/desktop/defaults.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
#include "p2p/base/port_allocator.h"
#include "pc/video_track_source.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/ref_counted_object.h"
#include "rtc_base/rtc_certificate_generator.h"
#include "ccapturer_track_source.h"
#include "clog.h"


namespace chen {


	//const char kAudioLabel[] = "";
	
	const char kAudioLabel[] = "audio_label";
	const char kVideoLabel[] = "video_label";
	const char kStreamId[] = "stream_id";

	crtc_publisher::crtc_publisher()
	{
	}

	void crtc_publisher::create_offer()
	{

		if (!InitializePeerConnection())
		{
			printf("[%s][%d] init peer connect failed !!!\n", __FUNCTION__, __LINE__);
			return;
		}

		peer_connection_->CreateOffer(this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());

	}

	bool crtc_publisher::InitializePeerConnection()
	{
		  networkThread = rtc::Thread::CreateWithSocketServer();
		  signalingThread = rtc::Thread::Create();
		  workerThread = rtc::Thread::Create();
		 
			  networkThread->SetName("network_thread", nullptr);
		  signalingThread->SetName("signaling_thread", nullptr);
		  workerThread->SetName("worker_thread", nullptr);

		 if ( ! networkThread->Start()  ||  ! signalingThread->Start()   || ! workerThread->Start() )
		{
			 
		}
		//std::unique_ptr<rtc::Thread>   work_thread = rtc::Thread::Create();
		peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
			networkThread.get() /* network_thread */,  workerThread.get() /* worker_thread */,
			signalingThread.get() /* signaling_thread */,
			nullptr /* default_adm */,
			webrtc::CreateBuiltinAudioEncoderFactory(),
			webrtc::CreateBuiltinAudioDecoderFactory(),
			webrtc::CreateBuiltinVideoEncoderFactory(),
			webrtc::CreateBuiltinVideoDecoderFactory(), nullptr /* audio_mixer */,
			nullptr /* audio_processing */);

		if (!peer_connection_factory_) {
			/*main_wnd_->MessageBox("Error", "Failed to initialize PeerConnectionFactory",
				true);
			DeletePeerConnection();*/
			return false;
		}
		
		// 设置SDP  ->马流是否加密哈DTLS
		if (!CreatePeerConnection(/*dtls=*/false))
		{
			/*main_wnd_->MessageBox("Error", "CreatePeerConnection failed", true);
			DeletePeerConnection();*/
		}

		//AddTracks();
		_add_tracks();

		return peer_connection_ != nullptr;
		//return true;
	}
	bool crtc_publisher::CreatePeerConnection(bool dtls)
	{
		webrtc::PeerConnectionInterface::RTCConfiguration config;
		config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan; //这个 
		//config.enable_dtls_srtp = dtls; //是否加密
		//webrtc::PeerConnectionInterface::IceServer server;
		//server.uri = GetPeerConnectionString();
		//config.servers.push_back(server);
		printf("[%s][%d] config.sdp_semantics = %d\n", __FUNCTION__, __LINE__, config.sdp_semantics);

		peer_connection_ = peer_connection_factory_->CreatePeerConnection(config, nullptr, nullptr, this);
		//RTC_LOG(INFO) << "[%s][%d] fff  config.sdp_semantics = %d\n", __FUNCTION__, __LINE__  config.sdp_semantics;

		return peer_connection_ != nullptr;
	}
	void crtc_publisher::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams)
	{
		     


	}
	void crtc_publisher::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver)
	{
	}
	void crtc_publisher::OnIceCandidate(const webrtc::IceCandidateInterface * candidate)
	{
		NORMAL_EX_LOG(" candidate --->");
	}
	void crtc_publisher::OnSuccess(webrtc::SessionDescriptionInterface * desc)
	{
		// 得到本地视频基本信息 先设置本地 SDP 鸭
		peer_connection_->SetLocalDescription(cdummysetsession_description_observer::Create(), desc);

		std::string sdp;
		desc->ToString(&sdp);
		//RTC_LOG(INFO) << "[" << __FUNCTION__ << "][ offer sdp = " << sdp  << "]";
		//printf("[%s][%d][][]\n", __FUNCTION__, __LINE__);
		NORMAL_EX_LOG("[offer sdp = %s]", sdp.c_str());

	}
	void crtc_publisher::OnFailure(webrtc::RTCError error)
	{

	}
	void crtc_publisher::OnFailure(const std::string & error)
	{

	}

	void crtc_publisher::_add_tracks()
	{
		if (!peer_connection_->GetSenders().empty())
		{
			return;  // Already added tracks.
		}
		///////////////////////////////////////////////AUDIO///////////////////////////////////////////////////////////
		rtc::scoped_refptr<webrtc::AudioSourceInterface> audio_source_ptr = peer_connection_factory_->CreateAudioSource(cricket::AudioOptions());

		rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track_ptr = peer_connection_factory_->CreateAudioTrack(kAudioLabel, audio_source_ptr);

		auto result_or_error = peer_connection_->AddTrack(audio_track_ptr, { kStreamId });
		if (!result_or_error.ok())
		{
			RTC_LOG(LS_ERROR) << "Failed to add audio track to PeerConnection: "
				<< result_or_error.error().message();
		}
		//////////////////////////////////////////VIDEO////////////////////////////////////////////////////////////////
		rtc::scoped_refptr<ccapturer_track_source> video_device = ccapturer_track_source::Create();
		if (video_device)
		{
			rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_proxy_ptr = peer_connection_factory_->CreateVideoTrack(kVideoLabel, video_device);

			//main_wnd_->StartLocalRenderer(video_track_proxy_ptr);

			result_or_error = peer_connection_->AddTrack(video_track_proxy_ptr, { kStreamId });
			if (!result_or_error.ok())
			{
				RTC_LOG(LS_ERROR) << "Failed to add video track to PeerConnection: "
					<< result_or_error.error().message();
			}

		}
		else {
			RTC_LOG(LS_ERROR) << "OpenVideoCaptureDevice failed";
		}
	}

}
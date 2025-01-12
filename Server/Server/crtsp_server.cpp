﻿/***********************************************************************************************
created: 		2023-05-11

author:			chensong

purpose:		_C_DTLS_ _H_
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


#include "crtsp_server.h"
#include "clog.h"
#include "ccfg.h"
#include "ctcp_server.h"
#include "crtsp_session_mgr.h"
#include "cparse_rtsp_request.h"
#include "crtsp_cmd_dispatch.h"
namespace chen {
	crtsp_server g_rtsp_server;
	/*crtsp_server::crtsp_server()
	{
	}*/
	crtsp_server::~crtsp_server()
	{
	}
	bool crtsp_server::init()
	{

		m_stoped = false;
		SYSTEM_LOG("rtsp cmd dis_patch init ...");
		if (!g_rtsp_cmd_dispatch.init())
		{
			return false;
		}


		std::string rtsp_ip = g_cfg.get_string(ECI_RtspWanIp);
		m_tcp_server_ptr = new ctcp_server(this, this, rtsp_ip, g_cfg.get_uint32(ECI_RtspWanPort));
		if (!m_tcp_server_ptr)
		{
			WARNING_EX_LOG(" rtsp server init port = %u, init failed !!!", g_cfg.get_int32(ECI_RtspWanPort));
			return false;
		}
		SYSTEM_LOG(" rtsp server init port = %u, init OK !!!", g_cfg.get_int32(ECI_RtspWanPort));
		return true;
	}
	void crtsp_server::destroy()
	{

		m_stoped = true;
		if (m_tcp_server_ptr)
		{
			delete m_tcp_server_ptr;
			m_tcp_server_ptr = NULL;
		}
		g_rtsp_cmd_dispatch.destroy();
	}
	bool crtsp_server::startup()
	{
		return true;
	}
	void crtsp_server::update(uint32 uDeltaTime)
	{
	}
	void crtsp_server::shutdown()
	{
	}
	void crtsp_server::OnRtcTcpConnectionNew(ctcp_server * tcpServer, ctcp_connection * connection)
	{
		int32 family;
		std::string ip;
		uint16_t port;
		uv_ip::GetAddressInfo(connection->GetPeerAddress(), family, ip, port);
		NORMAL_EX_LOG("New --> connect [ip = %s][port = %u] ", ip.c_str(), port );
		crtsp_session*  session =  g_rtsp_session_mgr.find(connection);
		session->set_session(connection);

	}
	void crtsp_server::OnTcpConnectionPacketReceived(ctcp_connection * connection, const uint8_t * data, size_t len)
	{
		int32 family;
		std::string ip;
		uint16_t port;
		 uv_ip ::GetAddressInfo(connection->GetPeerAddress(), family, ip, port);
		 NORMAL_EX_LOG("[ip = %s][port = %u][len = %u] ", ip.c_str(), port, len);
		 crtsp_session*  session = g_rtsp_session_mgr.find(connection);
		 if (!session)
		 {
			 WARNING_EX_LOG("[ip = %s][port = %u][len = %u]", ip.c_str(), port, len);
			 return;
		}
		 session->set_session(connection);
		 session->on_received((uint8*)data, len);
		 /*crtsp_request  request;
		 if (!request.parse((const char *)data, len))
		 {
			 WARNING_EX_LOG("rtsp parse request failed  data = %s", data);
			 return;
		 }
		 crtsp_cmd_handler * cmd_handler =  g_rtsp_cmd_dispatch.get_msg_handler(request.get_cmd_name());
		 if (!cmd_handler)
		 {
			 WARNING_EX_LOG("not find rtsp [cmd name = %s] failed !!!", request.get_cmd_name().c_str());
			 return;
		 }
		 ++cmd_handler->handle_count;
		 (session->*(cmd_handler->handler))(&request);*/


	}
	void crtsp_server::OnRtcTcpConnectionClosed(ctcp_server * tcpServer, ctcp_connection * connection)
	{
		int family;
		std::string ip;
		uint16_t port;
		uv_ip::GetAddressInfo(connection->GetPeerAddress(), family, ip, port);
		NORMAL_EX_LOG("desconnect[ip = %s][port = %u]", ip.c_str(), port);
		  g_rtsp_session_mgr.erase(connection);
	}

	uint32 crtsp_server::add_session(cmedia_session*session)
	{
		if (m_rtsp_suffix_map.find(session->get_rtsp_url_suffix()) != m_rtsp_suffix_map.end()) {
			return 0;
		}

		//std::shared_ptr<MediaSession> media_session(session);
		uint32 sessionId = session->get_media_session_id();
		m_rtsp_suffix_map.emplace(std::move(session->get_rtsp_url_suffix()), sessionId);
		m_media_sessions.emplace(sessionId, std::move(session));

		return sessionId;
	}

	void crtsp_server::remove_session(uint32 session_id)
	{
		auto iter = m_media_sessions.find(session_id);
		if (iter != m_media_sessions.end()) 
		{
			m_rtsp_suffix_map.erase(iter->second->get_rtsp_url_suffix());
			m_media_sessions.erase(session_id);
		}
	}

	bool crtsp_server::push_frame(uint32 session_id, MediaChannelId channel_id, AVFrame frame)
	{
		//std::shared_ptr<MediaSession> sessionPtr = nullptr;
		cmedia_session * session_ptr = NULL;
		{
			//std::lock_guard<std::mutex> locker(mutex_);
			auto iter = m_media_sessions.find(session_id);
			if (iter != m_media_sessions.end()) {
				session_ptr = iter->second;
			}
			else {
				return false;
			}
		}

		if (session_ptr != nullptr && session_ptr->get_num_client() != 0) {
			return session_ptr->handler_frame(channel_id, frame);
		}

		return false;
	}

	cmedia_session* crtsp_server::find_media_session(const std::string & suffix)
	{

		auto iter = m_rtsp_suffix_map.find(suffix);
		if (iter != m_rtsp_suffix_map.end()) 
		{
			return find_media_session(iter->second);
			//MediaSessionId id = iter->second;
			//return media_sessions_[id];
		}
		return NULL;
	}

	cmedia_session* crtsp_server::find_media_session(uint32 session_id)
	{
		auto iter = m_media_sessions.find(session_id);
		if (iter != m_media_sessions.end()) 
		{
			return iter->second;
		}

		return nullptr;
	}

}
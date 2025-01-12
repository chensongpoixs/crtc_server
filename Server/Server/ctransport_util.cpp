﻿/***********************************************************************************************
created: 		2023-03-08

author:			chensong

purpose:		ctransport_util

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
#include "ctransport_util.h"
#include "clog.h"
#include "csocket_util.h"
namespace chen {


	//bool  sdp_has_h264_profile(const cmedia_payload_type& payload_type, const std::string& profile)
	//{
	//	int32 err = 0;

	//	if (payload_type.m_format_specific_param.empty()) 
	//	{
	//		return false;
	//	}

	//	ch264_specific_param h264_param;
	//	if ((err =  parse_h264_fmtp(payload_type.m_format_specific_param, h264_param)) != 0)
	//	{
	//		WARNING_EX_LOG(" parse h264 fmtp failed !!! ");
	//		return false;
	//	}

	//	if (h264_param.m_profile_level_id == profile) 
	//	{
	//		return true;
	//	}

	//	return false;
	//}

	//// For example, 42001f 42e01f, see https://blog.csdn.net/epubcn/article/details/102802108
	//bool  sdp_has_h264_profile(const crtc_sdp& sdp, const std::string& profile)
	//{
	//	for (size_t i = 0; i < sdp.m_media_descs.size(); ++i) 
	//	{
	//		const cmedia_desc& desc = sdp.m_media_descs[i];
	//		if (!desc.is_video()) 
	//		{
	//			continue;
	//		}

	//		std::vector<cmedia_payload_type> payloads = desc.find_media_with_encoding_name("H264");
	//		if (payloads.empty())
	//		{
	//			continue;
	//		}

	//		for (std::vector<cmedia_payload_type>::iterator it = payloads.begin(); it != payloads.end(); ++it) 
	//		{
	//			const cmedia_payload_type& payload_type = *it;
	//			if (sdp_has_h264_profile(payload_type, profile)) 
	//			{
	//				return true;
	//			}
	//		}
	//	}

	//	return false;
	//}

	bool api_server_as_candidates(std::string api, std::set<std::string>& candidate_ips)
	{
		//srs_error_t err = srs_success;

		if (api.empty() /*|| !_srs_config->get_api_as_candidates()*/) 
		{
			return false;
		}

		std::string hostname = api;
		if (hostname.empty() || hostname ==  "localhost"/*CONSTS_LOCALHOST_NAME*/) 
		{
			return false;
		}
		if (hostname ==  "127.0.0.1"/*CONSTS_LOCALHOST*/ || hostname ==  "0.0.0.0"/*CONSTS_LOOPBACK*/ || hostname == "::" /*CONSTS_LOOPBACK6*/) 
		{
			return false;
		}

		// Whether add domain name.
		if (!is_ipv4(hostname) /*&& _srs_config->get_keep_api_domain()*/) 
		{
			candidate_ips.insert(hostname);
		}

		// Try to parse the domain name if not IP.
		if (! is_ipv4(hostname) /*&& _srs_config->get_resolve_api_domain()*/) 
		{
			int family = 0;
			std::string ip =  dns_resolve(hostname, family);
			if (ip.empty() || ip == "127.0.0.1" /*CONSTS_LOCALHOST*/ || ip ==  "0.0.0.0"/*CONSTS_LOOPBACK*/ || ip == "::" /*CONSTS_LOOPBACK6*/) 
			{
				return false;
			}

			// Try to add the API server ip as candidates.
			candidate_ips.insert(ip);
		}

		// If hostname is IP, use it.
		if ( is_ipv4(hostname)) 
		{
			candidate_ips.insert(hostname);
		}

		return true;
	}

}
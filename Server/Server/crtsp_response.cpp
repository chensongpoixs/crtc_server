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

#include "crtsp_response.h"
namespace chen {
	bool crtsp_response2::ParseResponse(cbuffer_reader * buffer)
	{
		if (strstr(buffer->Peek(), "\r\n\r\n") != NULL) {
			if (strstr(buffer->Peek(), "OK") == NULL) {
				return false;
			}

			char* ptr = strstr(buffer->Peek(), "Session");
			if (ptr != NULL) {
				char session_id[50] = { 0 };
				if (sscanf(ptr, "%*[^:]: %s", session_id) == 1)
					session_ = session_id;
			}

			cseq_++;
			buffer->RetrieveUntil("\r\n\r\n");
		}

		return true;
	}
	int crtsp_response2::BuildOptionReq(const char * buf, int buf_size)
	{
		memset((void*)buf, 0, buf_size);
		snprintf((char*)buf, buf_size,
			"OPTIONS %s RTSP/1.0\r\n"
			"CSeq: %u\r\n"
			"User-Agent: %s\r\n"
			"\r\n",
			rtsp_url_.c_str(),
			this->GetCSeq() + 1,
			user_agent_.c_str());

		method_ = OPTIONS;
		return (int)strlen(buf);
	}
	int crtsp_response2::BuildDescribeReq(const char * buf, int buf_size)
	{
		memset((void*)buf, 0, buf_size);
		snprintf((char*)buf, buf_size,
			"DESCRIBE %s RTSP/1.0\r\n"
			"CSeq: %u\r\n"
			"Accept: application/sdp\r\n"
			"User-Agent: %s\r\n"
			"\r\n",
			rtsp_url_.c_str(),
			this->GetCSeq() + 1,
			user_agent_.c_str());

		method_ = DESCRIBE;
		return (int)strlen(buf);
	}
	int crtsp_response2::BuildAnnounceReq(const char * buf, int buf_size, const char * sdp)
	{
		memset((void*)buf, 0, buf_size);
		snprintf((char*)buf, buf_size,
			"ANNOUNCE %s RTSP/1.0\r\n"
			"Content-Type: application/sdp\r\n"
			"CSeq: %u\r\n"
			"User-Agent: %s\r\n"
			"Session: %s\r\n"
			"Content-Length: %d\r\n"
			"\r\n"
			"%s",
			rtsp_url_.c_str(),
			this->GetCSeq() + 1,
			user_agent_.c_str(),
			this->GetSession().c_str(),
			(int)strlen(sdp),
			sdp);

		method_ = ANNOUNCE;
		return (int)strlen(buf);
	}
	int crtsp_response2::BuildSetupTcpReq(const char * buf, int buf_size, int channel)
	{
		int interleaved[2] = { 0, 1 };
		if (channel == 1) {
			interleaved[0] = 2;
			interleaved[1] = 3;
		}

		memset((void*)buf, 0, buf_size);
		snprintf((char*)buf, buf_size,
			"SETUP %s/track%d RTSP/1.0\r\n"
			"Transport: RTP/AVP/TCP;unicast;mode=record;interleaved=%d-%d\r\n"
			"CSeq: %u\r\n"
			"User-Agent: %s\r\n"
			"Session: %s\r\n"
			"\r\n",
			rtsp_url_.c_str(),
			channel,
			interleaved[0],
			interleaved[1],
			this->GetCSeq() + 1,
			user_agent_.c_str(),
			this->GetSession().c_str());

		method_ = SETUP;
		return (int)strlen(buf);
	}
	int crtsp_response2::BuildRecordReq(const char * buf, int buf_size)
	{
		memset((void*)buf, 0, buf_size);
		snprintf((char*)buf, buf_size,
			"RECORD %s RTSP/1.0\r\n"
			"Range: npt=0.000-\r\n"
			"CSeq: %u\r\n"
			"User-Agent: %s\r\n"
			"Session: %s\r\n"
			"\r\n",
			rtsp_url_.c_str(),
			this->GetCSeq() + 1,
			user_agent_.c_str(),
			this->GetSession().c_str());

		method_ = RECORD;
		return (int)strlen(buf);
	}
}

 
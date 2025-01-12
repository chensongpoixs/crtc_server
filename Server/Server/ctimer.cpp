﻿/***********************************************************************************************
created: 		2022-08-11

author:			chensong

purpose:		tiemr
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


#include "ctimer.h"
#include "cuv_util.h"

namespace chen {

	/* Static methods for UV callbacks. */

	inline static void onTimer(uv_timer_t* handle)
	{
		static_cast<ctimer*>(handle->data)->OnUvTimer();
	}

	inline static void onClose(uv_handle_t* handle)
	{
		delete handle;
	}


	ctimer::ctimer(Listener *listener)
		: m_listener(listener)
		, m_uvHandle(NULL)
		, m_closed(false)
		, m_timeout(0u)
		, m_repeat(0u)
	{
		m_uvHandle = new uv_timer_t;
		if (!m_uvHandle)
		{
			WARNING_EX_LOG(" uv timer alloc filed !!!");
			return  ;
		}

		m_uvHandle->data = static_cast<void*>(this);

		int32 err = uv_timer_init(uv_util::get_loop(), m_uvHandle);

		if (0 != err)
		{
			delete m_uvHandle;
			m_uvHandle = NULL;

			ERROR_EX_LOG("uv_timer_init() failed: %s", uv_strerror(err));
			return  ;
		}

	}
	ctimer::~ctimer()
	{
		if (!this->m_closed)
		{
			Close();
		}
	}

	void ctimer::Start(uint64_t timeout, uint64_t repeat)
	{
		if (m_closed)
		{
			ERROR_EX_LOG("timer closeed ");
			return;
		}

		m_timeout = timeout;
		m_repeat = repeat;

		if (0 != uv_is_active(reinterpret_cast<uv_handle_t*>(m_uvHandle)))
		{
			Stop();
		}


		int32 err = uv_timer_start(m_uvHandle, static_cast<uv_timer_cb>(onTimer), timeout, repeat);

		if (0 != err)
		{
			ERROR_EX_LOG("uv_timer_start() failed: %s", uv_strerror(err));
		}
	}
	void ctimer::Stop()
	{
		if (m_closed)
		{
			ERROR_EX_LOG("closed");
			return;
		}
		
		int32 err = uv_timer_stop(m_uvHandle);
		if (0 != err)
		{
			ERROR_EX_LOG("uv_timer_stop() failed: %s", uv_strerror(err));
		}
	}
	void ctimer::Close()
	{
		if (m_closed)
		{
			return;
		}
		m_closed = true;
		uv_close(reinterpret_cast<uv_handle_t*>(m_uvHandle), static_cast<uv_close_cb>(onClose));
	}
	void ctimer::Reset()
	{
		if (m_closed)
		{
			ERROR_EX_LOG("closed");
			return;
		}

		if (uv_is_active(reinterpret_cast<uv_handle_t*>(m_uvHandle)) == 0)
		{
			return;
		}

		if (m_repeat == 0u)
		{
			return;
		}

		int32 err = uv_timer_start(m_uvHandle, static_cast<uv_timer_cb>(onTimer), m_repeat, m_repeat);

		if (err != 0)
		{
			ERROR_EX_LOG("uv_timer_start() failed: %s", uv_strerror(err));
		}
	}
	void ctimer::Restart()
	{
		if (m_closed)
		{
			ERROR_EX_LOG("closed");
		}

		if (uv_is_active(reinterpret_cast<uv_handle_t*>(m_uvHandle)) != 0)
		{
			Stop();
		}

		int32 err = uv_timer_start(m_uvHandle, static_cast<uv_timer_cb>(onTimer), m_timeout, m_repeat);

		if (0 != err)
		{
			ERROR_EX_LOG("uv_timer_start() failed: %s", uv_strerror(err));
		}
	}
	void ctimer::OnUvTimer()
	{
		// Callback TODO@chensong 20220811 
		//OnTimer(this);
		m_listener->OnTimer(this);
	}
}
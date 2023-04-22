﻿/***********************************************************************************************
created: 		2023-02-01

author:			chensong

purpose:		 rtc——stun define 
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


#ifndef _C_RTC_STUN_DEFINE_H_
#define _C_RTC_STUN_DEFINE_H_
#include "cnet_type.h"

namespace chen {


	// STUN headers are 20 bytes.
	//(len >= 20) &&
		// DOC: https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
	//	(data[0] < 3) &&
		// Magic cookie must match.
	// @see: https://tools.ietf.org/html/rfc5389
	// The magic cookie field MUST contain the fixed value 0x2112A442 in network byte order
	// 固定的 魔法数
	const uint8   kStunMagicCookie[] = { 0x21, 0x12, 0xA4, 0x42 } /*0x2112A442*/;
	const uint32  KStunMagicCookie = 0x2112A442;


	enum EStunMessageType
	{
		// see @ https://tools.ietf.org/html/rfc3489#section-11.1	
		EBindingRequest = 0x0001,
		EBindingResponse = 0x0101,
		EBindingErrorResponse = 0x0111,
		ESharedSecretRequest = 0x0002,
		ESharedSecretResponse = 0x0102,
		ESharedSecretErrorResponse = 0x0112,
	};


	enum EStunMessageAttribute
	{
		// see @ https://tools.ietf.org/html/rfc3489#section-11.2
		EMappedAddress		= 0x0001,
		EResponseAddress		= 0x0002,
		EChangeRequest		= 0x0003,
		ESourceAddress		= 0x0004,
		EChangedAddress		= 0x0005,
		EUsername			= 0x0006,
		EPassword			= 0x0007,
		EMessageIntegrity	= 0x0008,
		EErrorCode			= 0x0009,
		EUnknownAttributes	= 0x000A,
		EReflectedFrom		= 0x000B,

		// see @ https://tools.ietf.org/html/rfc5389#section-18.2
		ERealm				= 0x0014,
		ENonce				= 0x0015,
		EXorMappedAddress	= 0x0020,
		ESoftware			= 0x8022,
		EAlternateServer		= 0x8023,
		EFingerprint			= 0x8028,

		EPriority				= 0x0024,
		EUseCandidate			= 0x0025,
		EIceControlled			= 0x8029,
		EIceControlling			= 0x802A,
	};
}

#endif // _C_RTC_STUN_DEFINE_H_
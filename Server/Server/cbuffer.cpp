﻿/***********************************************************************************************
created: 		2023-02-02

author:			chensong

purpose:		 buffer

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
#include "cbuffer.h"
#include "clog.h"
namespace chen {
	cbuffer::~cbuffer()
	{
	}
	char * cbuffer::data()
	{
		return m_bytes_ptr;
	}
	char * cbuffer::head()
	{
		return m_position_ptr;
	}
	int32 cbuffer::size()
	{
		return m_nb_bytes;
	}
	void cbuffer::set_size(int32 v)
	{
		m_nb_bytes = v;
	}
	int32 cbuffer::pos()
	{
		return (int32(m_position_ptr - m_bytes_ptr));
	}
	int32 cbuffer::left()
	{
		return m_nb_bytes - (int32)(m_position_ptr - m_bytes_ptr);
	}
	bool cbuffer::empty()
	{
		return !m_bytes_ptr || (m_position_ptr>= m_bytes_ptr + m_nb_bytes);
	}
	bool cbuffer::require(int32 required_size)
	{
		if (required_size < 0)
		{
			return false;
		}
		 
		return required_size <= m_nb_bytes - (m_position_ptr - m_bytes_ptr);;
	}
	void cbuffer::skip(int32 size)
	{
		cassert_desc(m_position_ptr, "m_position_ptr");
		cassert_desc(m_position_ptr + size >= m_bytes_ptr, "m_position_ptr + size >= m_bytes_ptr");
		cassert_desc(m_position_ptr + size <= m_bytes_ptr + m_nb_bytes, "m_position_ptr + size <= m_bytes_ptr + m_nb_bytes");

		m_position_ptr += size;
	}
	int8 cbuffer::read_1bytes()
	{

		cassert(require(1));

		return (int8)*m_position_ptr++;
	}
	int16 cbuffer::read_2bytes()
	{
		cassert(require(2));
		int16  value;

		char * pp = (char *)&value;
		pp[1] = *m_position_ptr++;
		pp[0] = *m_position_ptr++;

		return value;
	}
	int16 cbuffer::read_le2bytes()
	{
		cassert(require(2));
		int16  value;

		char * pp = (char *)&value;
		pp[0] = *m_position_ptr++;
		pp[1] = *m_position_ptr++;

		return value;
	}
	int32 cbuffer::read_3bytes()
	{
		cassert(require(3));

		int32  value = 0x00;
		char* pp = (char*)&value;
		pp[2] = *m_position_ptr++;
		pp[1] = *m_position_ptr++;
		pp[0] = *m_position_ptr++;

		return value;
	}
	int32 cbuffer::read_le3bytes()
	{
		cassert(require(3));

		int32  value = 0x00;
		char* pp = (char*)&value;
		pp[0] = *m_position_ptr++;
		pp[1] = *m_position_ptr++;
		pp[2] = *m_position_ptr++;

		return value;
	}
	int32 cbuffer::read_4bytes()
	{
		cassert(require(4));

		int32  value;
		char* pp = (char*)&value;
		pp[3] = *m_position_ptr++;
		pp[2] = *m_position_ptr++;
		pp[1] = *m_position_ptr++;
		pp[0] = *m_position_ptr++;

		return value;
	}
	int32 cbuffer::read_le4bytes()
	{
		cassert(require(4));

		int32 value;
		char* pp = (char*)&value;
		pp[0] = *m_position_ptr++;
		pp[1] = *m_position_ptr++;
		pp[2] = *m_position_ptr++;
		pp[3] = *m_position_ptr++;

		return value;
	}
	int64 cbuffer::read_8bytes()
	{
		cassert(require(8));

		int64  value;
		char* pp = (char*)&value;
		pp[7] = *m_position_ptr++;
		pp[6] = *m_position_ptr++;
		pp[5] = *m_position_ptr++;
		pp[4] = *m_position_ptr++;
		pp[3] = *m_position_ptr++;
		pp[2] = *m_position_ptr++;
		pp[1] = *m_position_ptr++;
		pp[0] = *m_position_ptr++;

		return value;
	}
	int64 cbuffer::read_le8bytes()
	{
		cassert(require(8));

		int64  value;
		char* pp = (char*)&value;
		pp[0] = *m_position_ptr++;
		pp[1] = *m_position_ptr++;
		pp[2] = *m_position_ptr++;
		pp[3] = *m_position_ptr++;
		pp[4] = *m_position_ptr++;
		pp[5] = *m_position_ptr++;
		pp[6] = *m_position_ptr++;
		pp[7] = *m_position_ptr++;

		return value;
	}
	std::string cbuffer::read_string(int32 len)
	{
		cassert(require(len));

		std::string value;
		value.append(m_position_ptr, len);

		m_position_ptr += len;

		return value;
	}
	void cbuffer::read_bytes(char * data, int32 size)
	{
		cassert_desc(require(size), "require(size) size = %u", size);

		memcpy(data, m_position_ptr, size);

		m_position_ptr += size;
	}
	void cbuffer::write_1bytes(int8 value)
	{
		cassert(require(1));

		*m_position_ptr++ = value;
	}
	void cbuffer::write_2bytes(int16 value)
	{
		cassert(require(2));

		char* pp = (char*)&value;
		*m_position_ptr++ = pp[1];
		*m_position_ptr++ = pp[0];
	}
	void cbuffer::write_le2bytes(int16 value)
	{
		cassert_desc(require(2), "require(2)");

		char* pp = (char*)&value;
		*m_position_ptr++ = pp[0];
		*m_position_ptr++ = pp[1];
	}
	void cbuffer::write_4bytes(int32 value)
	{
		cassert_desc(require(4), "require(4)");

		char* pp = (char*)&value;
		*m_position_ptr++ = pp[3];
		*m_position_ptr++ = pp[2];
		*m_position_ptr++ = pp[1];
		*m_position_ptr++ = pp[0];
	}
	void cbuffer::write_le4bytes(int32 value)
	{
		cassert_desc(require(4), "require(4)");

		char* pp = (char*)&value;
		*m_position_ptr++ = pp[0];
		*m_position_ptr++ = pp[1];
		*m_position_ptr++ = pp[2];
		*m_position_ptr++ = pp[3];
	}
	void cbuffer::write_3bytes(int32 value)
	{
		cassert_desc(require(3), "require(3)");

		char* pp = (char*)&value;
		*m_position_ptr++ = pp[2];
		*m_position_ptr++ = pp[1];
		*m_position_ptr++ = pp[0];
	}
	void cbuffer::write_le3bytes(int32 value)
	{
		cassert_desc(require(3), "require(3)");

		char* pp = (char*)&value;
		*m_position_ptr++ = pp[0];
		*m_position_ptr++ = pp[1];
		*m_position_ptr++ = pp[2];
	}
	void cbuffer::write_8bytes(int64 value)
	{
		cassert_desc(require(8), "require(8)");

		char* pp = (char*)&value;
		*m_position_ptr++ = pp[7];
		*m_position_ptr++ = pp[6];
		*m_position_ptr++ = pp[5];
		*m_position_ptr++ = pp[4];
		*m_position_ptr++ = pp[3];
		*m_position_ptr++ = pp[2];
		*m_position_ptr++ = pp[1];
		*m_position_ptr++ = pp[0];
	}
	void cbuffer::write_le8bytes(int64 value)
	{
		cassert_desc(require(8), "require(8)");

		char* pp = (char*)&value;
		*m_position_ptr++ = pp[0];
		*m_position_ptr++ = pp[1];
		*m_position_ptr++ = pp[2];
		*m_position_ptr++ = pp[3];
		*m_position_ptr++ = pp[4];
		*m_position_ptr++ = pp[5];
		*m_position_ptr++ = pp[6];
		*m_position_ptr++ = pp[7];
	}
	void cbuffer::write_string(std::string value)
	{
		if (value.empty()) 
		{
			return;
		}

		cassert(require((int32)value.length()));

		memcpy(m_position_ptr, value.data(), value.length());
		m_position_ptr += value.length();
	}
	void cbuffer::write_bytes(char * data, int32 size)
	{
		if (size <= 0) {
			return;
		}

		cassert(require(size));

		memcpy(m_position_ptr, data, size);
		m_position_ptr += size;
	}
}
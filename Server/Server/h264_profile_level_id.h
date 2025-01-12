﻿/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
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

#ifndef MEDIA_BASE_H264_PROFILE_LEVEL_ID_H_
#define MEDIA_BASE_H264_PROFILE_LEVEL_ID_H_

#include <map>
#include <string>

#include "absl/types/optional.h"
#include "common_types.h"  // NOLINT(build/include)
//#include "rtc_base/system/rtc_export.h"

namespace webrtc {
namespace H264 {

// Map containting SDP codec parameters.
typedef std::map<std::string, std::string> CodecParameterMap;

// All values are equal to ten times the level number, except level 1b which is
// special.
enum Level {
  kLevel1_b = 0,
  kLevel1 = 10,
  kLevel1_1 = 11,
  kLevel1_2 = 12,
  kLevel1_3 = 13,
  kLevel2 = 20,
  kLevel2_1 = 21,
  kLevel2_2 = 22,
  kLevel3 = 30,
  kLevel3_1 = 31,
  kLevel3_2 = 32,
  kLevel4 = 40,
  kLevel4_1 = 41,
  kLevel4_2 = 42,
  kLevel5 = 50,
  kLevel5_1 = 51,
  kLevel5_2 = 52
};

struct ProfileLevelId {
  constexpr ProfileLevelId(Profile profile, Level level)
      : profile(profile), level(level) {}
  Profile profile;
  Level level;
};

// Parse profile level id that is represented as a string of 3 hex bytes.
// Nothing will be returned if the string is not a recognized H264
// profile level id.
absl::optional<ProfileLevelId> ParseProfileLevelId(const char* str);

// Parse profile level id that is represented as a string of 3 hex bytes
// contained in an SDP key-value map. A default profile level id will be
// returned if the profile-level-id key is missing. Nothing will be returned if
// the key is present but the string is invalid.
RTC_EXPORT absl::optional<ProfileLevelId> ParseSdpProfileLevelId(
    const CodecParameterMap& params);

// Given that a decoder supports up to a given frame size (in pixels) at up to a
// given number of frames per second, return the highest H.264 level where it
// can guarantee that it will be able to support all valid encoded streams that
// are within that level.
RTC_EXPORT absl::optional<Level> SupportedLevel(int max_frame_pixel_count,
                                                float max_fps);

// Returns canonical string representation as three hex bytes of the profile
// level id, or returns nothing for invalid profile level ids.
RTC_EXPORT absl::optional<std::string> ProfileLevelIdToString(
    const ProfileLevelId& profile_level_id);

// Generate codec parameters that will be used as answer in an SDP negotiation
// based on local supported parameters and remote offered parameters. Both
// |local_supported_params|, |remote_offered_params|, and |answer_params|
// represent sendrecv media descriptions, i.e they are a mix of both encode and
// decode capabilities. In theory, when the profile in |local_supported_params|
// represent a strict superset of the profile in |remote_offered_params|, we
// could limit the profile in |answer_params| to the profile in
// |remote_offered_params|. However, to simplify the code, each supported H264
// profile should be listed explicitly in the list of local supported codecs,
// even if they are redundant. Then each local codec in the list should be
// tested one at a time against the remote codec, and only when the profiles are
// equal should this function be called. Therefore, this function does not need
// to handle profile intersection, and the profile of |local_supported_params|
// and |remote_offered_params| must be equal before calling this function. The
// parameters that are used when negotiating are the level part of
// profile-level-id and level-asymmetry-allowed.
void GenerateProfileLevelIdForAnswer(
    const CodecParameterMap& local_supported_params,
    const CodecParameterMap& remote_offered_params,
    CodecParameterMap* answer_params);

// Returns true if the parameters have the same H264 profile, i.e. the same
// H264::Profile (Baseline, High, etc).
bool IsSameH264Profile(const CodecParameterMap& params1,
                       const CodecParameterMap& params2);

}  // namespace H264
}  // namespace webrtc

#endif  // MEDIA_BASE_H264_PROFILE_LEVEL_ID_H_

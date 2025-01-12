# 问题


```
// TODO@chensong 202208011  
// .cpp(55): error C4146: 一元负运算符应用于无符号类型，结果仍为无符号类型  --> 解决办法——关闭SDL检查
```

# openssl 1.1.0 vs上编译指令


x64

```
#release版本命令

perl Configure  VC-WIN64A no-asm  shared  no-shared --prefix=D:/dep/openssl_1.1.0

# debug版本命令

perl Configure debug-VC-WIN64A no-asm  shared  no-shared --prefix=D:/dep/openssl_1.1.0




////
```


# 流程

1. Create_WebRTC

C2S --> forceTcp, producing, consuming, sctpCapabilities 


返回消息

	1. iceParameters ->用户名, 密码、 icelite
	2. iceCandidates -> foundation、 priority、ip、protocol， port、type(HOST)、tcpType
    3. iceState
	4. iceSelectedTuple ->localIp、localPort、remoteIp、remotePort、protocol
	5. dtlsParameters  -> fingerprints[{algorithm, value}, {algorithm, value}]、role
	6. sctpParameters  -> MIS、OS、isDataChannel、maxMessageSize、port、sctpBufferedAmount、sendBufferSize



2. WebRTC_connect

C2S -> TRansportId、dtlsParameters{role, fingerprints[algorithm, value ]} ==> 发送自己本地使用fingerprints加密密钥


S2C->


3. WebRTC_produce


# SDP

a： 属性
c: 连接
v： 版本
s： 一个会话
o： 谁拥有这个会话




```
v=0
o=chensong 10000 1 IN IP4 0.0.0.0
s=-
t=0 0
a=ice-lite
a=fingerprint:sha-512 3D:9A:9C:39:15:B1:1F:B4:43:F8:F0:95:D1:F6:DE:C1:3C:09:C1:B9:83:14:A0:E8:31:11:92:69:56:7F:6D:DD:9E:76:C9:9B:CD:82:10:DF:FB:64:B5:0B:7C:FB:DF:E0:83:54:5C:2F:0E:D7:1A:A7:E6:0C:AC:6D:FD:9B:AE:75
a=msid-semantic: WMS *
a=group:BUNDLE 0
m=video 7 UDP/TLS/RTP/SAVPF 100 101
c=IN IP4 127.0.0.1
a=rtpmap:100 H264/90000
a=rtpmap:101 rtx/90000
a=fmtp:100 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42e01f;x-google-max-bitrate=28000;x-google-min-bitrate=5500;x-google-start-bitrate=1000
a=fmtp:101 apt=100
a=rtcp-fb:100 transport-cc
a=rtcp-fb:100 ccm fir
a=rtcp-fb:100 nack
a=rtcp-fb:100 nack pli
a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
a=extmap:4 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id
a=extmap:5 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id
a=extmap:13 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
a=extmap:2 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
a=extmap:8 http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07
a=extmap:12 urn:3gpp:video-orientation
a=extmap:14 urn:ietf:params:rtp-hdrext:toffset
a=setup:active
a=mid:0
a=recvonly
a=ice-ufrag:xjg2o2h2v8bfzqkv
a=ice-pwd:z5ywy0b4le11t42tcqzpz5kdmr22i9rm
a=candidate:udpcandidate 1 udp 1076302079 192.168.1.73 43717 typ host
a=end-of-candidates
a=ice-options:renomination
a=rtcp-mux
a=rtcp-rsize
```

## 1、什么是 RTX

RTX 就是重传 Retransmission, 将丢失的包重新由发送方传给接收方。

Webrtc 默认开启 RTX (重传)，它一般采用不同的 SSRC 进行传输，即原始的 RTP 包和重传的 RTP 包的 SSRC 是不同的，这样不会干扰原始 RTP 包的度量。

RTX 包的 Payload 在 RFC4588 中有详细描述，一般 NACK 导致的重传包和 Bandwidth Probe 导致的探测包也可能走 RTX 通道。


为什么用 RTX
媒体流多使用 RTP 通过 UDP 进行传输，由于是不可靠传输，丢包是不可避免，也是可以容忍的，但是对于一些关键数据是不能丢失的，这时候就需要重传(RTX)。

## 2、在 WebRTC 中常用的 QoS 策略有
   
反馈：例如 PLI , NACK

冗余， 例如 FEC, RTX

调整：例如码率，分辨率及帧率的调整

缓存: 例如 Receive Adaptive Jitter Buffer, Send Buffer

这些措施一般需要结合基于拥塞控制(congestion control) 及带宽估计(bandwidth estimation)技术, 不同的网络条件下需要采用不同的措施。

FEC 用作丢包恢复需要占用更多的带宽，即使 5% 的丢包需要几乎一倍的带宽，在带宽有限的情况下可能会使情况更糟。

RTX 不会占用太多的带宽，接收方发送 NACK 指明哪些包丢失了，发送方通过单独的 RTP 流（不同的 SSRC）来重传丢失的包，但是 RTX 至少需要一个 RTT 来修复丢失的包。

音频流对于延迟很敏感，而且占用带宽不多，所以用 FEC 更好。WebRTC 默认并不为 audio 开启 RTX

视频流对于延迟没那么敏感，而且占用带宽很多，所以用 RTX 更好。

## 3、FEC - WebRTC 中 ulpfec 前向纠错？


```
// FEC Level 0 Header, 10 bytes.
//    0                   1                   2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |E|L|P|X|  CC   |M| PT recovery |            SN base            |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                          TS recovery                          |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |        length recovery        |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

```

OXR运算

|a|b|a[oxr]b|
|:---:|:---:|:---:|
|0|0|0|
|0|1|1|
|1|0|1|
|1|1|0|



E: 保留的扩展标志位，必须设置为0

L: 长掩码标志位。当L设置为0时， ulp header的mask长度为16bits，当L设置为1时，ulp header的mask长度为48bits。

P、X、CC、PT的值由此fec包所保护的RTP包的对应值通过XOR运算得到

SN base: 设置为此包所保护的RTP包中的最小序列号

TS recovery : 由此fec包所保护的RTP包的timestamps通过XOR运算得到

Length recovery: 由此fec包所保护的RTP包的长度通过XOR运算得到




```
//
// FEC Level 1 Header, 4 bytes (L = 0) or 8 bytes (L = 1).
//    0                   1                   2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |       Protection Length       |             mask              |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |              mask cont. (present only when L = 1)             |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

Protection Length:此级别所保护的数据的长度

mask: 保护掩码，长度为2给字节或者6个字节（由fec header的L标志位决定）




## openssl win上静态库编译报错问题


```
libcrypto.lib(e_capi.obj) : error LNK2019: 无法解析的外部符号 __imp_CertOpenStore，该符号在函数 capi_open_store 中被引用
2>libcrypto.lib(e_capi.obj) : error LNK2019: 无法解析的外部符号 __imp_CertCloseStore，该符号在函数 capi_find_key 中被引用
2>libcrypto.lib(e_capi.obj) : error LNK2019: 无法解析的外部符号 __imp_CertEnumCertificatesInStore，该符号在函数 capi_find_cert 中被引用
2>libcrypto.lib(e_capi.obj) : error LNK2019: 无法解析的外部符号 __imp_CertFindCertificateInStore，该符号在函数 capi_find_cert 中被引用
2>libcrypto.lib(e_capi.obj) : error LNK2019: 无法解析的外部符号 __imp_CertDuplicateCertificateContext，该符号在函数 capi_load_ssl_client_cert 中被引用
2>libcrypto.lib(e_capi.obj) : error LNK2019: 无法解析的外部符号 __imp_CertFreeCertificateContext，该符号在函数 capi_find_key 中被引用
2>libcrypto.lib(e_capi.obj) : error LNK2019: 无法解析的外部符号 __imp_CertGetCertificateContextProperty，该符号在函数 capi_cert_get_fname 中被引用 

 
```

解决方法： 添加依赖 Crypt32.lib、User32.lib、Advapi32.lib
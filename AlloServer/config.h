#pragma once

// RTP/RTSP params
#define FACE0_RTP_PORT_NUM      18888
#define BINOCULARS_RTP_PORT_NUM 18988
#define TTL                     255

// Encoder params
#define CODEC_NAME              "libx264rgb"
#define CODEC_FORMAT            AV_PIX_FMT_RGB24
#define DEFAULT_AVG_BIT_RATE    15000000
#define PRESET_VAL				"ultrafast"
#define TUNE_VAL				"zerolatency:fastdecode"
#define FPS						30

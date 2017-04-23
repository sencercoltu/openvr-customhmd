#pragma once

#include "Common.h"

extern "C" {
#include <libavcodec\avcodec.h>
#include <libavformat\avformat.h>
#include <libavutil\avutil.h>
#include <libavutil\opt.h>
#include <libavutil\time.h>
#include <libswscale\swscale.h>
#include <libavutil\imgutils.h>
#include <libavformat\avio.h>
}

#define MESSAGE_MAX_BYTES 8192
#define RING_BUFFER_BYTES (1024 * 256 + MESSAGE_MAX_BYTES)
#define STREAM_PIX_FMT	AV_PIX_FMT_YUV420P 
#define VIDEO_CODEC_ID	AV_CODEC_ID_H264

using namespace vr;

struct InfoPacket {
	char H;
	char M;
	char D1;
	char D2;
	int Width;
	int Height;
};

struct TextureData
{
	uint32_t Index;
	EVREye Eye;

	ID3D11Texture2D *pGPUTexture;
	ID3D11Resource *pGPUResource;

	ID3D11Texture2D *pCPUTexture;
	ID3D11Resource *pCPUResource;

	HANDLE hSharedHandle;
};

struct TextureSet
{
	uint32_t Pid;
	TextureData Data[3];
	bool HasHandle(HANDLE handle)
	{
		for (auto i = 0; i < 3; i++)
		{
			if (Data[i].hSharedHandle == handle)
				return true;
		}
		return false;
	}
};

struct TextureLink
{
	TextureData *pData;
	TextureSet *pSet;
};

struct DirectEyeData
{
	TextureData *pData;
	D3D11_TEXTURE2D_DESC Desc;
	EVREye Eye;
};

typedef struct AVContext {
	AVFormatContext *fmt_ctx;
	AVCodecParameters *video_codecpar;
	AVCodecContext * video_dec_ctx;
	AVStream *video_stream;
	int video_stream_idx;
	AVFrame *frame;
	AVPacket pkt;
	enum AVMediaType type;
	AVDictionary *opts;
	AVCodec *dec;
} AVContext;

class DirectStreamer
{
public:
	DWORD LastFrameTime;
	float FrameTime;

	unsigned long BufferSize;
	unsigned char *pPixelBuffer[3];

	int FPS = 50;

	DWORD LastPacketReceive;

	DirectEyeData Left;
	DirectEyeData Right;

	USBRotationData RotData = {};

	AVCodec *pCodec;
	AVCodecContext *pCodecContext;
	AVFrame *pFrame;
	AVPacket *pPacket;

	int Width;
	int Height;

	int FrameCount;

private:
	int in_linesize[3];
	SwsContext *pSwsContext = nullptr;

	void DirectStreamer::SaveFrame(AVFrame *pFrame, int frameno);
public:
	int ProcessFrame(int pitch);
	

	AVPacket *GetPacket();

	bool Init(int width, int height, float fps, char *url);

	void Destroy();

};

#include "DirectStreamer.h"


int DirectStreamer::ProcessFrame(int pitch)
{
	FrameCount++;

	AVPixelFormat fmt = AV_PIX_FMT_NONE;
	if (Left.Desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
		fmt = AV_PIX_FMT_RGBA;
	else if (Left.Desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)
		fmt = AV_PIX_FMT_BGRA;
	else
	{
		Sleep(1);
		return -1;
	}

	SaveFrameRGB(pPixelBuffer[0], pitch * Left.Desc.Height, FrameCount);

	in_linesize[0] = pitch;
	pSwsContext = sws_getCachedContext(pSwsContext, Left.Desc.Width * 2, Left.Desc.Height, fmt, Width, Height, STREAM_PIX_FMT, 0, 0, 0, 0);
	int h = sws_scale(pSwsContext, (const uint8_t * const *)&pPixelBuffer, in_linesize, 0, Height, pFrame->data, pFrame->linesize);

	SaveFrameYUV(pFrame, FrameCount);

	pFrame->pts = FrameCount;
	pFrame->pkt_duration = (int64_t)FrameTime;
		
	return avcodec_send_frame(pCodecContext, pFrame);
}

AVPacket *DirectStreamer::GetPacket()
{
	av_init_packet(pPacket);
	return avcodec_receive_packet(pCodecContext, pPacket) == 0 ? pPacket : nullptr;
}

bool DirectStreamer::Init(int width, int height, float fps, char *url)
{
	ZeroMemory(in_linesize, sizeof(in_linesize));
	in_linesize[0] = { 4 * width };

	if (fps > 0)
		FrameTime = 1000.0f / fps;
	else
		FrameTime = 100; //default to 10 fps for now

	FPS = (int)(1000 / FrameTime);

	Width = width;
	Height = height;

	av_register_all();
	avcodec_register_all();

	Left.Eye = EVREye::Eye_Left;
	Right.Eye = EVREye::Eye_Right;

	pCodec = avcodec_find_encoder(VIDEO_CODEC_ID);
	if (!pCodec)
	{
		Destroy();
		return false;
	}


	pCodecContext = avcodec_alloc_context3(pCodec);
	if (!pCodecContext)
	{
		Destroy();
		return false;
	}

	AVDictionary *pOptions = nullptr;
	av_dict_set(&pOptions, "profile", "high", 0);
	av_dict_set(&pOptions, "preset", "ultrafast", 0);
	av_dict_set(&pOptions, "tune", "zerolatency", 0);
	//av_dict_set(&pOptions, "tune", "zerolatency", 0);

	//av_opt_set(&pCodecContext, "profile", "baseline", 0);

	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	int numCPU = max(sysinfo.dwNumberOfProcessors / 2, 1);
	pCodecContext->profile = FF_PROFILE_H264_CONSTRAINED_BASELINE;
	pCodecContext->codec_id = VIDEO_CODEC_ID;
	pCodecContext->bit_rate = Width * Height;
	pCodecContext->rc_buffer_size = 3;
	pCodecContext->width = Width;
	pCodecContext->height = Height;
	pCodecContext->gop_size = 1;
	pCodecContext->keyint_min = 1;
	pCodecContext->pix_fmt = STREAM_PIX_FMT;
	pCodecContext->time_base.den = FPS;
	pCodecContext->time_base.num = 1;
	pCodecContext->max_b_frames = 0;

	pCodecContext->qmin = 5;
	pCodecContext->qmax = 30;
	pCodecContext->qcompress = 0;	
	pCodecContext->qblur = 1;	
	//pCodecContext->refs = 1;
	pCodecContext->delay = 0;
	//pCodecContext->max_qdiff = 10;
	pCodecContext->slices = 8;
	pCodecContext->ticks_per_frame = 2;

	pCodecContext->thread_count = numCPU;
	pCodecContext->thread_type = FF_THREAD_SLICE; // FF_THREAD_SLICE;	
	pCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;


	//tcp

	//av_opt_set(&pCodecContext, "preset", "ultrafast", 0);
	//av_opt_set(&pCodecContext, "tune", "zerolatency", 0);

	//av_opt_set(pCodecContext->priv_data, "x264opts", "sync-lookahead=0:slice-max-size=1400", 0);

	if (avcodec_open2(pCodecContext, nullptr, &pOptions) < 0)
	{
		Destroy();
		return false;
	}

	if (pOptions)
		av_dict_free(&pOptions);


	pPacket = av_packet_alloc();
	if (!pPacket)
	{
		Destroy();
		return false;
	}

	pFrame = av_frame_alloc();
	pFrame->format = STREAM_PIX_FMT;
	pFrame->width = pCodecContext->width;
	pFrame->height = pCodecContext->height;
	if (av_image_alloc(pFrame->data, pFrame->linesize, pFrame->width, pFrame->height, pCodecContext->pix_fmt, 32) < 0)
	{
		Destroy();
		return false;
	}

	int BufferSize = 3840 * 2160 * 4; //rgba	4k
	pPixelBuffer[0] = (uint8_t *)malloc(BufferSize);
	pPixelBuffer[1] = nullptr;
	pPixelBuffer[2] = nullptr;

	if (pPixelBuffer[0] == nullptr)
	{
		Destroy();
		return false;
	}
	ZeroMemory(pPixelBuffer[0], BufferSize);

	return true;
}

void DirectStreamer::Destroy()
{
	if (pFrame)
	{
		if (pFrame->data[0]) av_freep(&pFrame->data[0]);
		av_frame_free(&pFrame);
	}

	if (pPacket)
	{
		av_packet_unref(pPacket);
		av_packet_free(&pPacket);
	}


	if (pCodecContext) avcodec_close(pCodecContext);

	pCodecContext = nullptr;
	pCodec = nullptr;

	SAFE_FREE(pPixelBuffer[0]);
}

void DirectStreamer::SaveFrameYUV(AVFrame *pFrame, int frameno)
{
	return;
	FILE *yuvFile = nullptr;
	char szFilename[MAX_PATH];
	sprintf_s(szFilename, "D:\\OUT\\xx-%d.yuv", frameno);
	fopen_s(&yuvFile, szFilename, "wb");
	if (yuvFile)
	{
		fwrite(pFrame->data[0], 1, pCodecContext->width * pCodecContext->height, yuvFile);
		fwrite(pFrame->data[1], 1, pCodecContext->width * pCodecContext->height / 4, yuvFile);
		fwrite(pFrame->data[2], 1, pCodecContext->width * pCodecContext->height / 4, yuvFile);
		fclose(yuvFile);
	}
}

void DirectStreamer::SaveFrameRGB(uint8_t *pixels, int size, int frameno)
{
	return;		
	FILE *yuvFile = nullptr;
	char szFilename[MAX_PATH];
	sprintf_s(szFilename, "D:\\OUT\\xx-%d.rgb", frameno);
	fopen_s(&yuvFile, szFilename, "wb");
	if (yuvFile)
	{
		fwrite(pixels, 1, size, yuvFile);
		fclose(yuvFile);
	}
}

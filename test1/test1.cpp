// http://linux.amazingdev.com/blog/archives/2011/09/28/tutorial01.c

#ifdef __cplusplus
 #define __STDC_CONSTANT_MACROS
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
#endif

extern "C" { 
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <iostream>

int main(int argc, char *argv[])
{
	AVFormatContext *pFormatCtx=NULL;
	int i, videoStream;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec;
	AVFrame *pFrame;
	AVPacket packet;
	int numBytes;
	AVDictionary    *optionsDict = NULL;

	if(argc < 2) 
	{
		std::cout << "Please provide a movie file" << std::endl;
		return -1;
	}	
	
	// Register all formats and codecs
	av_register_all();
	
	// Open video file
	if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)!=0)
		return -1; // Couldn't open file	
	
	// Retrieve stream information
	if(avformat_find_stream_info(pFormatCtx, NULL)<0)
		return -1; // Couldn't find stream information
		
	// Dump information about file onto standard error
	av_dump_format(pFormatCtx, 0, argv[1], 0);	
	
	for(i=0; i<pFormatCtx->nb_streams; i++)
	{
		std::cout << "CODEC #" << i << std::endl;
		std::cout << "MEDIA TYPE: " << pFormatCtx->streams[i]->codec->codec_type << std::endl;
		std::cout << "CODEC ID: " << pFormatCtx->streams[i]->codec->codec_id << std::endl;
		std::cout << "SIZE: " << pFormatCtx->streams[i]->codec->width << "x" << pFormatCtx->streams[i]->codec->height << std::endl;
		std::cout << "CODEC TYPE: " << pFormatCtx->streams[i]->codec->codec_type << std::endl;
		if (pFormatCtx->streams[i]->r_frame_rate.den > 0)
			std::cout << "FRAME RATE: " << pFormatCtx->streams[i]->r_frame_rate.num / (float)pFormatCtx->streams[i]->r_frame_rate.den << std::endl;
		std::cout << "DURATION: " << pFormatCtx->streams[i]->duration << std::endl;
		std::cout << "FRAMES: " << pFormatCtx->streams[i]->nb_frames << std::endl;
		
		pCodecCtx=pFormatCtx->streams[i]->codec;
		
		pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
		if(!pCodec)
		{
			std::cout << "Unsupported codec!" << std::endl;
			continue;
		}
		std::cout << "CODEC: " << pCodec->name << std::endl;
		if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0)
		{
			std::cout << "Error opening codec!" << std::endl;
			continue;
		}
		
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			// Determine required buffer size and allocate buffer
			numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
								pCodecCtx->height);
			
			std::cout << "NUM BYTES: " << numBytes << std::endl;
			
			videoStream = i;
		}
		
		avcodec_close(pCodecCtx);
		
		std::cout << "---------------------------------" << std::endl;
	}
    
	pCodecCtx=pFormatCtx->streams[videoStream]->codec;
	pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	if(!pCodec)
	{
		std::cout << "Unsupported codec!" << std::endl;
		return -1;
	}

	if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0)
	{
		std::cout << "Error opening codec!" << std::endl;
		return -1;
	}
	
	
	pFrame=avcodec_alloc_frame();
	
	i=0;
	while(av_read_frame(pFormatCtx, &packet)>=0) 
	{
		if(packet.stream_index==videoStream) 
		{
			std::cout << "VIDEO FRAME" << std::endl;
			std::cout << "PACKET SIZE: " << packet.size << std::endl;
			std::cout << "PACKET PTS: " << packet.pts << std::endl;
			std::cout << "PACKET DURATION: " << packet.duration << std::endl;
			
			i++;
		}
		
		av_free_packet(&packet);
		
		if (i > 20)
			break;
	}
	
	av_free(pFrame);
	
	avcodec_close(pCodecCtx);
	
    avformat_close_input(&pFormatCtx);
	
	return 0;
}


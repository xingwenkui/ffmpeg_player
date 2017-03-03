/**
 * 最简单的基于FFmpeg的视频播放器 2
 * Simplest FFmpeg Player 2
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 第2版使用SDL2.0取代了第一版中的SDL1.2
 * Version 2 use SDL 2.0 instead of SDL 1.2 in version 1.
 *
 * 本程序实现了视频文件的解码和显示(支持HEVC，H.264，MPEG2等)。
 * 是最简单的FFmpeg视频解码方面的教程。
 * 通过学习本例子可以了解FFmpeg的解码流程。
 * This software is a simplest video player based on FFmpeg.
 * Suitable for beginner of FFmpeg.
 *
 */

//http://blog.csdn.net/leixiaohua1020/article/details/8652605

#include <stdio.h>

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "SDL2/SDL.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <SDL2/SDL.h>
#ifdef __cplusplus
};
#endif
#endif

/*



 avformat_open_input();//打开视频文件
 avformat_find_stream_info();//查找文件的流信息
 av_dump_format();//dump只是个调试函数，输出文件的音、视频流的基本信息了，帧率、分辨率、音频采样等等
 for(...);//遍历文件的各个流，找到第一个视频流，并记录该流的编码信息
 sws_getContext();//根据编码信息设置渲染格式
 avcodec_find_decoder();//在库里面查找支持该格式的解码器
 avcodec_open2();//打开解码器
 pFrame=avcodec_alloc_frame();//分配一个帧指针，指向解码后的原始帧
 pFrameRGB=avcodec_alloc_frame();//分配一个帧指针，指向存放转换成RGB后的帧
 avpicture_fill(pFrameRGB);//给pFrameRGB帧加上分配的内存;
 while(true)
 {
     av_read_frame();//读取一个帧（到最后帧则break）
     avcodec_decode_video2();//解码该帧
     sws_getCachedContext()sws_scale（）;//把该帧转换（渲染）成RGB
     SaveFrame();//对前5帧保存成ppm图形文件(这个是自定义函数，非API)
     av_free_packet();//释放本次读取的帧内存
 }




 一些数据结构的说明

    frame：是指直观上完整的视频帧或音频帧，注意可以指视频也可以指音频，一般是未解码的；
    packet：是形式上的概念，指视频或音频的包，这里的包未必是完整的视频帧或音频帧，可能是一帧视频的一部分。
    picture：指直观上完整图象，只能指视频，简言之，picture是解压后的视频frame；
    特别要区分frame和packet。frame很好理解，直观的帧。
	但frame有两个问题：
	一是每个压缩后的frame通常都比较大，特别是视频；
	二是每个 frame的压缩后的大小都不一样； 这两个原因都使frame不便于存储和传输，所以就引入了packet的概念。

    packet是文件容器和传递层面的一个技术概念，而frame是一般人都能理解的直观概念。
	对于视频，一个frame通常会被划分为等大的多个packet，但是它们的时间戮是相同的。

（8）包数据-读取数据的第一步是先读取数据包，对应结构体struct AVPacket

     从视频文件中读取一个包：av_read_frame(pFormateCtx,&packet) —— 该函数会自动为packet分配内存

     判断是否是视频包：packet.stream_index == videoStream


     对帧进行解码：avcodec_decode_video(pCodecCtx, pFrame,&frameFinished,packet.data, packet.size) 

     该函数并不能保证一定能从包中解码得到一帧完整的数据帧（比如需要其他参考帧的情况下），因此会设置标

     志位frameFinished，如果帧图像完整，则frameFinished非零，否则该值为零。需要注意的是，目前我认为该

     函数会自动为pFrame分配图像实际数据的内存空间，因为pFrame->data[i]为指针数组，其保存的只是数据存放

     空间的头地址，而分配frame的函数应该只分配了pFrame自己本身的内存，没有对data[]指向进行内存分配。

     记得packet资源需要进行释放：av_free_packet(&packet)

（9）转换帧数据，首先需要为保存帧数据找一个容器，结构体struct AVFrame

     分配帧： pFrame = avcodec_alloc_frame()

     因为需要转换帧的格式，所以需要再分配一帧： pFrameRGB = avcodec_alloc_frame()

     之前的帧pFrame的data会在av_read_frame中由函数自动分配，而pFrameRGB的data需要手动分配

     先取得需要分配内存的大小：nByteNum = avpicture_get_size(PIX_FMT_RGB24,pCodecCtx->width,pCodecCtx->height)

     分配内存：buf = av_malloc(nByteNum)

     pFrameRGB与buf关联：avpicture_fill((AVPicture*)pFrameRGB,buf,PIX_FMT_RGB24,pCodecCtx->width,pCodecCtx->height)

     转换图片格式：img_convert((AVPicture*)pFrameRGB,PIX_FMT_RGB24,(AVPicture*)pFrame,pCodecCtx->pix_fmt,pCodecCtx->widht,pCodecCtx->height)  (注：该转换格式为老的API，新API应该使用sws_getContext()以及sws_scals()这两个函数)
*/


//Output YUV420P data as a file 
#define OUTPUT_YUV420P 0

int main(int argc, char* argv[])
{
	AVFormatContext	*pFormatCtx;
	int				i, videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;
	AVFrame	*pFrame,*pFrameYUV;
	uint8_t *out_buffer;
	AVPacket *packet;
	int y_size;
	int ret, got_picture;
	struct SwsContext *img_convert_ctx;

	char filepath[] = "E:\\XMPCache\\Marvels.Agents.of.S.H.I.E.L.D.S04E14.720p.HDTV.X264-DIMENSION[rarbg]\\Marvels.Agents.of.S.H.I.E.L.D.S04E14.720p.HDTV.X264-DIMENSION.mkv";
	//char filepath[]="bigbuckbunny_480x272.h265";
	//SDL---------------------------
	int screen_w=0,screen_h=0;
	SDL_Window *screen; 
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Rect sdlRect;

	FILE *fp_yuv;

	av_register_all();//初始化ffmpeg库，如果系统里面的ffmpeg没配置好这里会出错

	avformat_network_init();//需要播放网络视频

	//AVFormatContext的初始化函数是avformat_alloc_context()，销毁函数是avformat_free_context()。
	pFormatCtx = avformat_alloc_context();//AVFormatContext：统领全局的基本结构体。主要用于处理封装格式（FLV/MKV/RMVB等）。

	if(avformat_open_input(&pFormatCtx,filepath,NULL,NULL)!=0)//打开视频文件
	{
		printf("Couldn't open input stream.\n");
		return -1;
	}

	/*
	。在初始信息不足的情况下，avformat_find_stream_info接口需要在内部调用read_frame_internal接口读取流数据，然后再分析后，设置核心数据结构AVFormatContext。
	由于需要读取数据包，avformat_find_stream_info接口会带来很大的延迟
	*/
	if(avformat_find_stream_info(pFormatCtx,NULL)<0)//查找文件的流信息
	{
		printf("Couldn't find stream information.\n");
		return -1;
	}
	
	videoindex = -1;

	/*
	//编解码器上下文AVCodecContext:
	   pFormatCtx->stream[i]->codec为流的解码器信息，结构为struct AVCodecContext；
	*/
	for(i = 0; i < pFormatCtx->nb_streams; i++) ////nb_streams;音视频流的个数 
	{
		if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex=i;
			break;
		}
	}

	if(videoindex==-1)
	{
		printf("Didn't find a video stream.\n");
		return -1;
	}

	/*
	取得流中的解码器信息（解码器上下文），表示的结构为struct AVCodecContext

     pCodeCtx = pFormatCtx->stream[videoStream]->codec；

     解码器上下文保存了流中所使用的解码的所有信息。
	*/
	pCodecCtx = pFormatCtx->streams[videoindex]->codec;


	/*
	在解码器上下文中，需要找到真正的解码器，并打开它，解码器使用结构struct AVCodec表示,

	在库里面查找支持该格式的解码器     pCodec = avcodec_find_decoder(pCodeCtx->codec->codec_id);

     codec_id指示了流的编码器类型，函数返回对应的解码器结构，赋值给解码器指针pCodec
	*/
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

	if(pCodec==NULL)
	{
		printf("Codec not found.\n");
		return -1;
	}

	if(avcodec_open2(pCodecCtx, pCodec,NULL)<0)//打开解码器
	{
		printf("Could not open codec.\n");
		return -1;
	}


	/*
	http://blog.csdn.net/luotuo44/article/details/26486877

	 AVFrame，顾名思义，这个结构体应该是保存视频帧的信息的。
	 像一帧图像也是可以保存在AVFrame结构中。
	 事实上，我们可以直接从一个YUV文件中，把一张YUV图像数据读到AVFrame中.

	 AVFrame结构体一般用于存储原始数据（即非压缩数据，例如对视频来说是YUV，RGB，对音频来说是PCM），此外还包含了一些相关的信息。
	 比如说，解码的时候存储了宏块类型表，QP表，运动矢量表等数据。编码的时候也存储了相关的数据。因此在使用FFMPEG进行码流分析的时候，AVFrame是一个很重要的结构体。

	 可以用av_frame_alloc(void)函数来分配一个AVFrame结构体。
	 这个函数只是分配AVFrame结构体，但data指向的内存并没有分配，需要我们指定。这个内存的大小就是一张特定格式图像所需的大小，
	*/
	pFrame = av_frame_alloc();

	pFrameYUV = av_frame_alloc();

	//avpicture_get_size 参考http://blog.csdn.net/leixiaohua1020/article/details/14215391 中的：在本代码中，将数据保存成了RGB24的格式。如果想保存成其他格式，比如YUV420，YUV422等，需要做2个步骤：
	out_buffer=(uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);


	/*
	AVPacket:
	 * This structure stores compressed data. It is typically exported by demuxers
	 * and then passed as input to decoders, or received as output from encoders and
	 * then passed to muxers.
	 *
	 * For video, it should typically contain one compressed frame. For audio it may
	 * contain several compressed frames.

	muxer是指合并文件，即将视频文件、音频文件和字幕文件合并为某一个视频格式。
	比如把rmvb格式的视频，mp3格式的音频文件以及srt格式的字幕文件，合并成为一个新的mp4或者mkv格式的文件。
demuxer是muxer的逆过程，就是把合成的文件中提取出不同的格式文件。
	*/
	packet=(AVPacket *)av_malloc(sizeof(AVPacket));

	//Output Info-----------------------------
	printf("--------------- File Information ----------------\n");
	av_dump_format(pFormatCtx,0,filepath,0);
	printf("-------------------------------------------------\n");

	/*
sws_getContext: http://blog.csdn.net/leixiaohua1020/article/details/44305697

libswscale是一个主要用于处理图片像素数据的类库。可以完成图片像素格式的转换，图片的拉伸等工作
	*/
	img_convert_ctx = sws_getContext( pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 
									  pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P,
									  SWS_BICUBIC, NULL, NULL, NULL); //SWS_BICUBIC参数和其他参数的区别是什么，没有找到

#if OUTPUT_YUV420P 
    fp_yuv=fopen("output.yuv","wb+");  
#endif  
	

	/*
	http://blog.csdn.net/leixiaohua1020/article/details/40680907

	http://www.cnblogs.com/landmark/archive/2012/04/29/2475252.html

	Simple DirectMedia Layer is a cross-platform multimedia library designed to provide low level access to audio, 
	keyboard, mouse, joystick, 3D hardware via OpenGL, and 2D video framebuffer


	SDL实际上封装了Direct3D，DirectSound这类的底层API


	*/
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) 
	{  
		printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
		return -1;
	} 

	screen_w = pCodecCtx->width;
	screen_h = pCodecCtx->height;
	//SDL 2.0 Support for multiple windows
	screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h,
		SDL_WINDOW_OPENGL);//SDL_WINDOW_OPENGL OPENGL下可用的窗口 

	if(!screen) {  
		printf("SDL: could not create window - exiting:%s\n",SDL_GetError());  
		return -1;
	}

	//SDL_CreateRenderer(): 基于窗口创建渲染器（Render）。 http://blog.csdn.net/leixiaohua1020/article/details/40723085
	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);  
	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)


	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,pCodecCtx->width,pCodecCtx->height);  

	sdlRect.x=0;
	sdlRect.y=0;
	sdlRect.w=screen_w;
	sdlRect.h=screen_h;

	//SDL End----------------------
	while(av_read_frame(pFormatCtx, packet) >= 0)
	{
		if(packet->stream_index == videoindex)
		{
			/*
			ffmpeg中的avcodec_decode_video2()的作用是解码一帧视频数据。输入一个压缩编码的结构体AVPacket，输出一个解码后的结构体AVFrame
			http://blog.csdn.net/leixiaohua1020/article/details/12679719

			http://blog.csdn.net/hjwang1/article/details/17957227
			*/
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if(ret < 0)
			{
				printf("Decode Error.\n");
				return -1;
			}

			if(got_picture)
			{
				/*
				http://guguclock.blogspot.com/2009/12/ffmpeg-swscale.html
				  如果想將某個PixelFormat轉換至另一個PixelFormat，例如，將YUV420P轉換成YUYV422，或是想變換圖的大小，都可以使用swscale達成。

				int sws_scale(SwsContext *c, uint8_t* src[], int srcStride[], int srcSliceY, int srcSliceH, uint8_t* dst[], int dstStride[])

				總共有七個參數； 
				第一個參數即是由 sws_getContext 所取得的參數。 
				第二個 src 及第六個 dst 分別指向input 和 output 的 buffer。 
				第三個 srcStride 及第七個 dstStride 分別指向 input 及 output 的 stride；如果不知道什麼是 stride，姑且可以先把它看成是每一列的 byte 數。 
				第四個 srcSliceY，就註解的意思來看，是指第一列要處理的位置；這裡我是從頭處理，所以直接填0。想知道更詳細說明的人，可以參考 swscale.h 的註解。 
				第五個srcSliceH指的是 source slice 的高度。

				舉一個例子如下

				sws_scale(img_convert_ctx, inbuf, inlinesize, 0, in_height, outbuf, outlinesize);

				這裡應該比較好懂，可以參考上面的參數說明。

				最後，全部處理完後，需呼叫sws_freeContext() 結束。用法很簡單，把sws_getContext 取得的參數填入即可。如下

				sws_freeContext(img_convert_ctx);

				最後再整理一次，要使用swscale，只要使用 sws_getContext() 進行初始化、sws_scale() 進行主要轉換、sws_freeContext() 結束，即可完成全部動作。

				swscale，可從foreman.yuv內取出第一張圖，轉換大小後存成另一張圖。
				*/
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
				
#if OUTPUT_YUV420P
				y_size=pCodecCtx->width*pCodecCtx->height;  
				fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y 
				fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U
				fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V
#endif
				//SDL---------------------------
#if 0
				SDL_UpdateTexture( sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0] );  
#else
				//http://wiki.libsdl.org/SDL_UpdateYUVTexture

				//这里需要搞清楚YUV
				//
				/*
				http://blog.csdn.net/leixiaohua1020/article/details/14214577
				data[]
				对于packed格式的数据（例如RGB24），会存到data[0]里面。
				对于planar格式的数据（例如YUV420P），则会分开成data[0]，data[1]，data[2]...（YUV420P中data[0]存Y，data[1]存U，data[2]存V）
				*/
				SDL_UpdateYUVTexture(sdlTexture, &sdlRect,
				pFrameYUV->data[0], pFrameYUV->linesize[0],
				pFrameYUV->data[1], pFrameYUV->linesize[1],
				pFrameYUV->data[2], pFrameYUV->linesize[2]);
#endif	
				
				SDL_RenderClear( sdlRenderer );  

				///SDL_RenderCopy(): 纹理复制给渲染器。 
				SDL_RenderCopy( sdlRenderer, sdlTexture,  NULL, &sdlRect); // 

				//SDL_RenderPresent(): 显示。
				SDL_RenderPresent( sdlRenderer );  
				//SDL End-----------------------
				//Delay 40ms
				SDL_Delay(40);
			}
		}
		av_free_packet(packet);
	}
	//flush decoder
	//FIX: Flush Frames remained in Codec
	int a = 0;
	while (1) 
	{
		/*
		http://blog.csdn.net/dangxw_/article/details/50974677

		http://blog.csdn.net/leixiaohua1020/article/details/19016109
		 avcodec_decode_video2()解码视频后丢帧的问题解决
		*/
		ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
		if (ret < 0)
			break;
		if (!got_picture)
			break;
		sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, 
			pFrameYUV->data, pFrameYUV->linesize);
#if OUTPUT_YUV420P
		int y_size=pCodecCtx->width*pCodecCtx->height;  
		fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y 
		fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U
		fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V
#endif
		//SDL---------------------------
		SDL_UpdateTexture( sdlTexture, &sdlRect, pFrameYUV->data[0], pFrameYUV->linesize[0] );  
		SDL_RenderClear( sdlRenderer );  
		SDL_RenderCopy( sdlRenderer, sdlTexture,  NULL, &sdlRect);  
		SDL_RenderPresent( sdlRenderer );  
		//SDL End-----------------------
		//Delay 40ms
		SDL_Delay(40);

		a++;
	}

	

	sws_freeContext(img_convert_ctx);

#if OUTPUT_YUV420P 
    fclose(fp_yuv);
#endif 

	SDL_Quit();

	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
	avformat_free_context(pFormatCtx);
	return 0;
}


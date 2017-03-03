/**
 * ��򵥵Ļ���FFmpeg����Ƶ������ 2
 * Simplest FFmpeg Player 2
 *
 * ������ Lei Xiaohua
 * leixiaohua1020@126.com
 * �й���ý��ѧ/���ֵ��Ӽ���
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * ��2��ʹ��SDL2.0ȡ���˵�һ���е�SDL1.2
 * Version 2 use SDL 2.0 instead of SDL 1.2 in version 1.
 *
 * ������ʵ������Ƶ�ļ��Ľ������ʾ(֧��HEVC��H.264��MPEG2��)��
 * ����򵥵�FFmpeg��Ƶ���뷽��Ľ̡̳�
 * ͨ��ѧϰ�����ӿ����˽�FFmpeg�Ľ������̡�
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



 avformat_open_input();//����Ƶ�ļ�
 avformat_find_stream_info();//�����ļ�������Ϣ
 av_dump_format();//dumpֻ�Ǹ����Ժ���������ļ���������Ƶ���Ļ�����Ϣ�ˣ�֡�ʡ��ֱ��ʡ���Ƶ�����ȵ�
 for(...);//�����ļ��ĸ��������ҵ���һ����Ƶ��������¼�����ı�����Ϣ
 sws_getContext();//���ݱ�����Ϣ������Ⱦ��ʽ
 avcodec_find_decoder();//�ڿ��������֧�ָø�ʽ�Ľ�����
 avcodec_open2();//�򿪽�����
 pFrame=avcodec_alloc_frame();//����һ��ָ֡�룬ָ�������ԭʼ֡
 pFrameRGB=avcodec_alloc_frame();//����һ��ָ֡�룬ָ����ת����RGB���֡
 avpicture_fill(pFrameRGB);//��pFrameRGB֡���Ϸ�����ڴ�;
 while(true)
 {
     av_read_frame();//��ȡһ��֡�������֡��break��
     avcodec_decode_video2();//�����֡
     sws_getCachedContext()sws_scale����;//�Ѹ�֡ת������Ⱦ����RGB
     SaveFrame();//��ǰ5֡�����ppmͼ���ļ�(������Զ��庯������API)
     av_free_packet();//�ͷű��ζ�ȡ��֡�ڴ�
 }




 һЩ���ݽṹ��˵��

    frame����ֱָ������������Ƶ֡����Ƶ֡��ע�����ָ��ƵҲ����ָ��Ƶ��һ����δ����ģ�
    packet������ʽ�ϵĸ��ָ��Ƶ����Ƶ�İ�������İ�δ������������Ƶ֡����Ƶ֡��������һ֡��Ƶ��һ���֡�
    picture��ֱָ��������ͼ��ֻ��ָ��Ƶ������֮��picture�ǽ�ѹ�����Ƶframe��
    �ر�Ҫ����frame��packet��frame�ܺ���⣬ֱ�۵�֡��
	��frame���������⣺
	һ��ÿ��ѹ�����frameͨ�����Ƚϴ��ر�����Ƶ��
	����ÿ�� frame��ѹ����Ĵ�С����һ���� ������ԭ��ʹframe�����ڴ洢�ʹ��䣬���Ծ�������packet�ĸ��

    packet���ļ������ʹ��ݲ����һ�����������frame��һ���˶�������ֱ�۸��
	������Ƶ��һ��frameͨ���ᱻ����Ϊ�ȴ�Ķ��packet���������ǵ�ʱ��¾����ͬ�ġ�

��8��������-��ȡ���ݵĵ�һ�����ȶ�ȡ���ݰ�����Ӧ�ṹ��struct AVPacket

     ����Ƶ�ļ��ж�ȡһ������av_read_frame(pFormateCtx,&packet) ���� �ú������Զ�Ϊpacket�����ڴ�

     �ж��Ƿ�����Ƶ����packet.stream_index == videoStream


     ��֡���н��룺avcodec_decode_video(pCodecCtx, pFrame,&frameFinished,packet.data, packet.size) 

     �ú��������ܱ�֤һ���ܴӰ��н���õ�һ֡����������֡��������Ҫ�����ο�֡������£�����˻����ñ�

     ־λframeFinished�����֡ͼ����������frameFinished���㣬�����ֵΪ�㡣��Ҫע����ǣ�Ŀǰ����Ϊ��

     �������Զ�ΪpFrame����ͼ��ʵ�����ݵ��ڴ�ռ䣬��ΪpFrame->data[i]Ϊָ�����飬�䱣���ֻ�����ݴ��

     �ռ��ͷ��ַ��������frame�ĺ���Ӧ��ֻ������pFrame�Լ�������ڴ棬û�ж�data[]ָ������ڴ���䡣

     �ǵ�packet��Դ��Ҫ�����ͷţ�av_free_packet(&packet)

��9��ת��֡���ݣ�������ҪΪ����֡������һ���������ṹ��struct AVFrame

     ����֡�� pFrame = avcodec_alloc_frame()

     ��Ϊ��Ҫת��֡�ĸ�ʽ��������Ҫ�ٷ���һ֡�� pFrameRGB = avcodec_alloc_frame()

     ֮ǰ��֡pFrame��data����av_read_frame���ɺ����Զ����䣬��pFrameRGB��data��Ҫ�ֶ�����

     ��ȡ����Ҫ�����ڴ�Ĵ�С��nByteNum = avpicture_get_size(PIX_FMT_RGB24,pCodecCtx->width,pCodecCtx->height)

     �����ڴ棺buf = av_malloc(nByteNum)

     pFrameRGB��buf������avpicture_fill((AVPicture*)pFrameRGB,buf,PIX_FMT_RGB24,pCodecCtx->width,pCodecCtx->height)

     ת��ͼƬ��ʽ��img_convert((AVPicture*)pFrameRGB,PIX_FMT_RGB24,(AVPicture*)pFrame,pCodecCtx->pix_fmt,pCodecCtx->widht,pCodecCtx->height)  (ע����ת����ʽΪ�ϵ�API����APIӦ��ʹ��sws_getContext()�Լ�sws_scals()����������)
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

	av_register_all();//��ʼ��ffmpeg�⣬���ϵͳ�����ffmpegû���ú���������

	avformat_network_init();//��Ҫ����������Ƶ

	//AVFormatContext�ĳ�ʼ��������avformat_alloc_context()�����ٺ�����avformat_free_context()��
	pFormatCtx = avformat_alloc_context();//AVFormatContext��ͳ��ȫ�ֵĻ����ṹ�塣��Ҫ���ڴ����װ��ʽ��FLV/MKV/RMVB�ȣ���

	if(avformat_open_input(&pFormatCtx,filepath,NULL,NULL)!=0)//����Ƶ�ļ�
	{
		printf("Couldn't open input stream.\n");
		return -1;
	}

	/*
	���ڳ�ʼ��Ϣ���������£�avformat_find_stream_info�ӿ���Ҫ���ڲ�����read_frame_internal�ӿڶ�ȡ�����ݣ�Ȼ���ٷ��������ú������ݽṹAVFormatContext��
	������Ҫ��ȡ���ݰ���avformat_find_stream_info�ӿڻ�����ܴ���ӳ�
	*/
	if(avformat_find_stream_info(pFormatCtx,NULL)<0)//�����ļ�������Ϣ
	{
		printf("Couldn't find stream information.\n");
		return -1;
	}
	
	videoindex = -1;

	/*
	//�������������AVCodecContext:
	   pFormatCtx->stream[i]->codecΪ���Ľ�������Ϣ���ṹΪstruct AVCodecContext��
	*/
	for(i = 0; i < pFormatCtx->nb_streams; i++) ////nb_streams;����Ƶ���ĸ��� 
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
	ȡ�����еĽ�������Ϣ�������������ģ�����ʾ�ĽṹΪstruct AVCodecContext

     pCodeCtx = pFormatCtx->stream[videoStream]->codec��

     �����������ı�����������ʹ�õĽ����������Ϣ��
	*/
	pCodecCtx = pFormatCtx->streams[videoindex]->codec;


	/*
	�ڽ������������У���Ҫ�ҵ������Ľ���������������������ʹ�ýṹstruct AVCodec��ʾ,

	�ڿ��������֧�ָø�ʽ�Ľ�����     pCodec = avcodec_find_decoder(pCodeCtx->codec->codec_id);

     codec_idָʾ�����ı��������ͣ��������ض�Ӧ�Ľ������ṹ����ֵ��������ָ��pCodec
	*/
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

	if(pCodec==NULL)
	{
		printf("Codec not found.\n");
		return -1;
	}

	if(avcodec_open2(pCodecCtx, pCodec,NULL)<0)//�򿪽�����
	{
		printf("Could not open codec.\n");
		return -1;
	}


	/*
	http://blog.csdn.net/luotuo44/article/details/26486877

	 AVFrame������˼�壬����ṹ��Ӧ���Ǳ�����Ƶ֡����Ϣ�ġ�
	 ��һ֡ͼ��Ҳ�ǿ��Ա�����AVFrame�ṹ�С�
	 ��ʵ�ϣ����ǿ���ֱ�Ӵ�һ��YUV�ļ��У���һ��YUVͼ�����ݶ���AVFrame��.

	 AVFrame�ṹ��һ�����ڴ洢ԭʼ���ݣ�����ѹ�����ݣ��������Ƶ��˵��YUV��RGB������Ƶ��˵��PCM�������⻹������һЩ��ص���Ϣ��
	 ����˵�������ʱ��洢�˺�����ͱ�QP���˶�ʸ��������ݡ������ʱ��Ҳ�洢����ص����ݡ������ʹ��FFMPEG��������������ʱ��AVFrame��һ������Ҫ�Ľṹ�塣

	 ������av_frame_alloc(void)����������һ��AVFrame�ṹ�塣
	 �������ֻ�Ƿ���AVFrame�ṹ�壬��dataָ����ڴ沢û�з��䣬��Ҫ����ָ��������ڴ�Ĵ�С����һ���ض���ʽͼ������Ĵ�С��
	*/
	pFrame = av_frame_alloc();

	pFrameYUV = av_frame_alloc();

	//avpicture_get_size �ο�http://blog.csdn.net/leixiaohua1020/article/details/14215391 �еģ��ڱ������У������ݱ������RGB24�ĸ�ʽ������뱣���������ʽ������YUV420��YUV422�ȣ���Ҫ��2�����裺
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

	muxer��ָ�ϲ��ļ���������Ƶ�ļ�����Ƶ�ļ�����Ļ�ļ��ϲ�Ϊĳһ����Ƶ��ʽ��
	�����rmvb��ʽ����Ƶ��mp3��ʽ����Ƶ�ļ��Լ�srt��ʽ����Ļ�ļ����ϲ���Ϊһ���µ�mp4����mkv��ʽ���ļ���
demuxer��muxer������̣����ǰѺϳɵ��ļ�����ȡ����ͬ�ĸ�ʽ�ļ���
	*/
	packet=(AVPacket *)av_malloc(sizeof(AVPacket));

	//Output Info-----------------------------
	printf("--------------- File Information ----------------\n");
	av_dump_format(pFormatCtx,0,filepath,0);
	printf("-------------------------------------------------\n");

	/*
sws_getContext: http://blog.csdn.net/leixiaohua1020/article/details/44305697

libswscale��һ����Ҫ���ڴ���ͼƬ�������ݵ���⡣�������ͼƬ���ظ�ʽ��ת����ͼƬ������ȹ���
	*/
	img_convert_ctx = sws_getContext( pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 
									  pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P,
									  SWS_BICUBIC, NULL, NULL, NULL); //SWS_BICUBIC����������������������ʲô��û���ҵ�

#if OUTPUT_YUV420P 
    fp_yuv=fopen("output.yuv","wb+");  
#endif  
	

	/*
	http://blog.csdn.net/leixiaohua1020/article/details/40680907

	http://www.cnblogs.com/landmark/archive/2012/04/29/2475252.html

	Simple DirectMedia Layer is a cross-platform multimedia library designed to provide low level access to audio, 
	keyboard, mouse, joystick, 3D hardware via OpenGL, and 2D video framebuffer


	SDLʵ���Ϸ�װ��Direct3D��DirectSound����ĵײ�API


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
		SDL_WINDOW_OPENGL);//SDL_WINDOW_OPENGL OPENGL�¿��õĴ��� 

	if(!screen) {  
		printf("SDL: could not create window - exiting:%s\n",SDL_GetError());  
		return -1;
	}

	//SDL_CreateRenderer(): ���ڴ��ڴ�����Ⱦ����Render���� http://blog.csdn.net/leixiaohua1020/article/details/40723085
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
			ffmpeg�е�avcodec_decode_video2()�������ǽ���һ֡��Ƶ���ݡ�����һ��ѹ������Ľṹ��AVPacket�����һ�������Ľṹ��AVFrame
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
				  ����댢ĳ��PixelFormat�D�Q����һ��PixelFormat�����磬��YUV420P�D�Q��YUYV422��������׃�Q�D�Ĵ�С��������ʹ��swscale�_�ɡ�

				int sws_scale(SwsContext *c, uint8_t* src[], int srcStride[], int srcSliceY, int srcSliceH, uint8_t* dst[], int dstStride[])

				�������߂������� 
				��һ������������ sws_getContext ��ȡ�õą����� 
				�ڶ��� src �������� dst �քeָ��input �� output �� buffer�� 
				������ srcStride �����߂� dstStride �քeָ�� input �� output �� stride�������֪��ʲ�N�� stride�����ҿ����Ȱ���������ÿһ�е� byte ���� 
				���Ă� srcSliceY�����]�����˼������ָ��һ��Ҫ̎���λ�ã��@�e���Ǐ��^̎������ֱ����0����֪����Ԕ���f�����ˣ����ԅ��� swscale.h ���]�⡣ 
				���傀srcSliceHָ���� source slice �ĸ߶ȡ�

				�eһ����������

				sws_scale(img_convert_ctx, inbuf, inlinesize, 0, in_height, outbuf, outlinesize);

				�@�e��ԓ���^�ö������ԅ�������ą����f����

				���ᣬȫ��̎�����ᣬ�����sws_freeContext() �Y�����÷��ܺ��Σ���sws_getContext ȡ�õą������뼴�ɡ�����

				sws_freeContext(img_convert_ctx);

				����������һ�Σ�Ҫʹ��swscale��ֻҪʹ�� sws_getContext() �M�г�ʼ����sws_scale() �M����Ҫ�D�Q��sws_freeContext() �Y�����������ȫ��������

				swscale���ɏ�foreman.yuv��ȡ����һ���D���D�Q��С������һ���D��
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

				//������Ҫ�����YUV
				//
				/*
				http://blog.csdn.net/leixiaohua1020/article/details/14214577
				data[]
				����packed��ʽ�����ݣ�����RGB24������浽data[0]���档
				����planar��ʽ�����ݣ�����YUV420P�������ֿ���data[0]��data[1]��data[2]...��YUV420P��data[0]��Y��data[1]��U��data[2]��V��
				*/
				SDL_UpdateYUVTexture(sdlTexture, &sdlRect,
				pFrameYUV->data[0], pFrameYUV->linesize[0],
				pFrameYUV->data[1], pFrameYUV->linesize[1],
				pFrameYUV->data[2], pFrameYUV->linesize[2]);
#endif	
				
				SDL_RenderClear( sdlRenderer );  

				///SDL_RenderCopy(): �����Ƹ���Ⱦ���� 
				SDL_RenderCopy( sdlRenderer, sdlTexture,  NULL, &sdlRect); // 

				//SDL_RenderPresent(): ��ʾ��
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
		 avcodec_decode_video2()������Ƶ��֡��������
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


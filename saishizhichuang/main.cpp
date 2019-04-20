//
//  main.cpp
//  saishizhichuang
//
//  Created by gaoliwen on 2019/4/15.
//  Copyright © 2019 gaoliwen. All rights reserved.
//

#include <iostream>

#ifdef __cplusplus
extern "C"
{
#endif
    
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <SDL2/SDL.h>
    
#ifdef __cplusplus
}
#endif

#include "RTSPSource.hpp"

#include <unistd.h>

int main(int argc, const char * argv[]) {
    //char filepath[]="/Users/gaoliwen/work/testfile/gaoliwen.flv";
    //char filepath[]="rtsp://admin:zq888888@47.104.180.74:8872/h264/ch34/main/av_stream";
    //char filepath[]="rtsp://admin:zq888888@47.104.180.74:8872/h264/ch40/main/av_stream";
    char filepath[]="rtsp://admin:a12345678@192.168.0.62/h264/ch1/main/av_stream";
    
    av_register_all();
    avformat_network_init();
    
//    //上下文
//    AVFormatContext *pFormatCtx = nullptr;//format context
//    AVCodecContext *pCodecCtx = nullptr;//codec context
//    AVCodec *pCodec = nullptr;//codec
//    AVFrame    *pFrame = nullptr,
 //   AVFrame *pFrameYUV = nullptr;//frame 编码前的数据，如yuv数据
//    AVPacket *packet = nullptr;//编码后的数据，如h264数据
//    struct SwsContext *img_convert_ctx = nullptr;//图像转换上下文
    
    //SDL相关的变量
    int screen_w = 1920, screen_h = 1080;//显示的宽和高
    SDL_Window *screen = nullptr;
    SDL_Rect rect;
    SDL_Event event;
    
//    //format 上下文初始化
//    pFormatCtx = avformat_alloc_context();
//    
//    AVDictionary *format_opts = NULL;
//    //av_dict_set(&format_opts, "stimeout", std::to_string( 2* 1000000).c_str(), 0); //设置链接超时时间（us）
//    av_dict_set(&format_opts, "rtsp_transport",  "tcp", 0); //设置推流的方式，默认udp。
//    
//    int ret = avformat_open_input(&pFormatCtx, filepath, nullptr, &format_opts);
//    std::cout<<"avformat_open_input result is "<<ret<<std::endl;
//    
//    ret = avformat_find_stream_info(pFormatCtx, nullptr);
//    std::cout<<"avformat_find_stream_info result is "<<ret<<std::endl;
//    
//    //查找视频流索引
//    int videoindex = -1;
//    for(int i=0; i<pFormatCtx->nb_streams; i++){
//        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
//            videoindex = i;
//            break;
//        }
//    }
//    std::cout<<"videoindex is "<<videoindex<<std::endl;
//    
//    //codec 上下文初始化
//    pCodecCtx=pFormatCtx->streams[videoindex]->codec;
//    //codec 初始化
//    //to do : 确认解码器是否需要释放
//    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
//    std::cout<<"pCodecCtx->codec_id is "<< pCodecCtx->codec_id<<" (如果是27，就表示是h264编码)"<<std::endl;
//    
//    //打开解码器
//    ret = avcodec_open2(pCodecCtx, pCodec, nullptr);
//    std::cout<<"avcodec_open2 result is "<<ret<<std::endl;
//    
//    //frame初始化
//    pFrame = av_frame_alloc();
    //frame初始化
//    pFrameYUV = av_frame_alloc();
//    
//    uint8_t *out_buffer = new uint8_t[avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height)];//分配AVFrame所需内存
//    avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);//填充AVFrame
    
    //SDL2初始化
    int ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
    std::cout<<"SDL_Init result is "<<ret<<std::endl;
    screen = SDL_CreateWindow("saishizhichuang", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              screen_w, screen_h,
                              SDL_WINDOW_RESIZABLE/* SDL_WINDOW_HIDDEN*/| SDL_WINDOW_OPENGL);
    SDL_Renderer *sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
    SDL_Texture *sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV,
                                                SDL_TEXTUREACCESS_STREAMING,
                                                screen_w, screen_h);
    
    //to do
    //逻辑
    
//    //packet初始化
//    packet = (AVPacket*)av_malloc(sizeof(AVPacket));
//    
//    //打印一下输入文件的信息
//    av_dump_format(pFormatCtx, 0, filepath, 0);
//    
//    //初始化图像转换上下文
//    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
//                                     pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC,
//                                     NULL, NULL, NULL);

    //主逻辑
//    int got_picture = -1;
//    while (av_read_frame(pFormatCtx, packet)>=0) {
//        if(packet->stream_index == videoindex){
//            //解码：h264->yuv
//            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
//            if(ret<0){
//                std::cout<<"严重错误"<<std::endl;
//                //return -1;
//                break;
//            }
//            if(got_picture){//解码成功
//                static int count = 0;
//                std::cout<<count++<<" frame"<<std::endl;
//                
//                //像素格式转换
//                sws_scale(img_convert_ctx,
//                          (const uint8_t* const*)pFrame->data, pFrame->linesize,
//                          0, pCodecCtx->height,
//                          pFrameYUV->data, pFrameYUV->linesize);
//
    
    RTSPSource rtspSource;
    rtspSource.Init();
    rtspSource.Start();
    
    while (true) {
        rect.x = 0;
        rect.y = 0;
        rect.w = screen_w;
        rect.h = screen_h;
        
        AVFrame* pFrameYUV = rtspSource.getFrame();
        if(pFrameYUV){
            //todo:这里获得frame
            SDL_UpdateTexture( sdlTexture, nullptr, pFrameYUV->data[0], pFrameYUV->linesize[0] );
            SDL_RenderClear( sdlRenderer );
            SDL_RenderCopy( sdlRenderer, sdlTexture, nullptr, nullptr );
            SDL_RenderPresent( sdlRenderer );
        }
        if(pFrameYUV){
            
            static FILE *fp_yuv = fopen("glw1.yuv", "wb");
            if(fp_yuv&&false){
                int y_size=screen_w*screen_h;
                fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y
                fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U
                fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V
            }
            
            delete[] pFrameYUV->data[0];
            av_free(pFrameYUV);
        }
        //延时10ms
        SDL_Delay(10);
        
        SDL_PollEvent(&event);
        switch( event.type ) {
            case SDL_QUIT:
                SDL_Quit();
                exit(0);
                break;
            default:
                break;
        }

    }
//                //////////
//                //@begin

//                //@end
//                //////////
//            }
//        }//end of "if(packet->stream_index == videoindex)"
//        av_free_packet(packet);

    
    //while (true) {
        SDL_PollEvent(&event);
        switch( event.type ) {
            case SDL_QUIT:
                SDL_Quit();
                exit(0);
                break;
            default:
                break;
        }

    //}
    
    
    //}
    
//    sws_freeContext(img_convert_ctx);
//
    
    
    rtspSource.Stop();
    
    SDL_DestroyTexture(sdlTexture);
//    delete[] out_buffer;
//    av_free(pFrameYUV);
//
//    //avcodec_free_context(&pCodecCtx);//不需要
//    avcodec_close(pCodecCtx);
//    pCodecCtx = nullptr;
//    
//    avformat_close_input(&pFormatCtx);
//    avformat_free_context(pFormatCtx);
//    pFormatCtx = nullptr;
    
    return 0;
}

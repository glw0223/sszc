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
    char filepath1[]="rtsp://admin:a12345678@192.168.0.63/h264/ch1/main/av_stream";
    
    av_register_all();
    avformat_network_init();
    
    //SDL相关的变量
    int screen_w = 1920*2, screen_h = 1080*2;//显示的宽和高
    SDL_Window *screen = nullptr;
    SDL_Event event;
    
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
    
    RTSPSource rtspSource("source0",filepath,false);
    rtspSource.Init();
    rtspSource.Start();
    
    RTSPSource rtspSource1("source1",filepath1,false);
    rtspSource1.Init();
    rtspSource1.Start();
    bool timeline = false;
    while (true) {
        if(rtspSource.hasFrame() && rtspSource1.hasFrame()){
            if(!timeline){
                timeline = true;
                int64_t pts = rtspSource.clearFrameAndBasePTS();
                int64_t pts1 = rtspSource1.clearFrameAndBasePTS();
                std::cout<<"pts:"<<pts/90<<" "<<"pts1:"<<pts1/90<<std::endl;
                continue;
            }
            AVFrame* pFrameYUV = rtspSource.getFrame();
            AVFrame* pFrameYUV1 = rtspSource1.getFrame();
            std::cout<<"pFrameYUV->pts:"<<pFrameYUV->pts<<" pFrameYUV1->pts:"<<pFrameYUV1->pts<<std::endl;
            if(pFrameYUV && pFrameYUV1){
                
                int w=1920,h=1080;
                SDL_Rect rect1,rect2;
                rect1.x=0;rect1.y=0;rect1.w=w;rect1.h=h;
                rect2.x=w;rect2.y=0;rect2.w=w;rect2.h=h;
                
                SDL_UpdateTexture( sdlTexture, &rect1, pFrameYUV->data[0], pFrameYUV->linesize[0] );
                SDL_UpdateTexture( sdlTexture, &rect2, pFrameYUV1->data[0], pFrameYUV1->linesize[0] );
                
                SDL_RenderClear( sdlRenderer );
                SDL_RenderCopy( sdlRenderer, sdlTexture, nullptr, nullptr );
                SDL_RenderPresent( sdlRenderer );
            }
            if(pFrameYUV && pFrameYUV1){
                //            static FILE *fp_yuv = fopen("glw1.yuv", "wb");
                //            if(fp_yuv&&false){
                //                int y_size=screen_w*screen_h;
                //                fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y
                //                fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U
                //                fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V
                //            }
                
                delete[] pFrameYUV->data[0];
                av_free(pFrameYUV);
                pFrameYUV = nullptr;
                
                delete[] pFrameYUV1->data[0];
                av_free(pFrameYUV1);
                pFrameYUV1 = nullptr;
            }
        }
        //延时1ms
        SDL_Delay(1);
        
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
    
    SDL_PollEvent(&event);
    switch( event.type ) {
        case SDL_QUIT:
            SDL_Quit();
            exit(0);
            break;
        default:
            break;
    }
    
    rtspSource.Stop();
    rtspSource1.Stop();
    
    SDL_DestroyTexture(sdlTexture);
    return 0;
}


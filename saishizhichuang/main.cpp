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
    
    //SDL相关的变量
    int screen_w = 1920, screen_h = 1080;//显示的宽和高
    SDL_Window *screen = nullptr;
    SDL_Rect rect;
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
    
    SDL_DestroyTexture(sdlTexture);    
    return 0;
}

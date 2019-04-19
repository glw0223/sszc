//
//  source.cpp
//  saishizhichuang
//
//  Created by gaoliwen on 2019/4/17.
//  Copyright © 2019 gaoliwen. All rights reserved.
//

#include <iostream>
#include <unistd.h>
#include "RTSPSource.hpp"

#include <stdio.h>

RTSPSource::RTSPSource(){
    
}
RTSPSource::~RTSPSource(){
    
}
void RTSPSource::Init(){
    char filepath[]="rtsp://admin:zq888888@47.104.180.74:8872/h264/ch40/main/av_stream";
    //format 上下文初始化
    pFormatCtx = avformat_alloc_context();
    
    AVDictionary *format_opts = NULL;
    //av_dict_set(&format_opts, "stimeout", std::to_string( 2* 1000000).c_str(), 0); //设置链接超时时间（us）
    av_dict_set(&format_opts, "rtsp_transport",  "tcp", 0); //设置推流的方式，默认udp。
    
    int ret = avformat_open_input(&pFormatCtx, filepath, nullptr, &format_opts);
    std::cout<<"avformat_open_input result is "<<ret<<std::endl;
    
    ret = avformat_find_stream_info(pFormatCtx, nullptr);
    std::cout<<"avformat_find_stream_info result is "<<ret<<std::endl;
    
    //查找视频流索引
    for(int i=0; i<pFormatCtx->nb_streams; i++){
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
            videoindex = i;
            break;
        }
    }
    std::cout<<"videoindex is "<<videoindex<<std::endl;
    
    //codec 上下文初始化
    pCodecCtx=pFormatCtx->streams[videoindex]->codec;
    //codec 初始化
    //to do : 确认解码器是否需要释放
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    std::cout<<"pCodecCtx->codec_id is "<< pCodecCtx->codec_id<<" (如果是27，就表示是h264编码)"<<std::endl;
    
    //打开解码器
    ret = avcodec_open2(pCodecCtx, pCodec, nullptr);
    std::cout<<"avcodec_open2 result is "<<ret<<std::endl;
    
    //frame初始化
    pFrame = av_frame_alloc();
//
//    pFrameYUV = av_frame_alloc();
//    out_buffer = new uint8_t[avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height)];//分配AVFrame所需内存
//    avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);//填充AVFrame

    //packet初始化
    packet = (AVPacket*)av_malloc(sizeof(AVPacket));
    
    //打印一下输入文件的信息
    av_dump_format(pFormatCtx, 0, filepath, 0);
    
    //初始化图像转换上下文
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                     pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC,
                                     NULL, NULL, NULL);
}

void RTSPSource::Uninit(){
    sws_freeContext(img_convert_ctx);
    
//    delete[] out_buffer;
//    av_free(pFrameYUV);
    
    myMutex.lock();
    for(int i=0;i<myAVFrame.size();i++){
        AVFrame* temp = myAVFrame.front();
        myAVFrame.pop();
        delete[] temp->data[0];
        av_free(temp);
    }
    myMutex.unlock();
    
    //avcodec_free_context(&pCodecCtx);//不需要
    avcodec_close(pCodecCtx);
    pCodecCtx = nullptr;
    
    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);
    pFormatCtx = nullptr;
}

void RTSPSource::Start(){
    bRun = true;
    pthread_create(&(this->pth), NULL, (this->Run), this);
}

void RTSPSource::Stop(){
    bRun = false;
    
    pthread_join(pth, NULL);
}

AVFrame* RTSPSource::getFrame(){
    AVFrame* frame = nullptr;
    myMutex.lock();
    if(myAVFrame.size()>0){
        frame = myAVFrame.front();
        myAVFrame.pop();
    }
    myMutex.unlock();
    return frame;
}

void RTSPSource::putFrame(AVFrame* frame){
    myMutex.lock();
    myAVFrame.push(frame);
    std::cout<<"myAVFrame size is "<<myAVFrame.size()<<std::endl;
    myMutex.unlock();
}

void *RTSPSource::Run(void* context){
    RTSPSource *source = (RTSPSource*)context;
    while (source->bRun) {
        source->Loop();
    }
    
    return nullptr;
}

void RTSPSource::Loop(){
    int got_picture = -1;
    if (av_read_frame(pFormatCtx, packet)>=0) {
        if(packet->stream_index == videoindex){
            //解码：h264->yuv
            int ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            if(ret<0){
                std::cout<<"严重错误"<<std::endl;
                //return -1;
                //break;
            }
            if(got_picture){//解码成功
                static int count = 0;
                std::cout<<count++<<" frame"<<std::endl;
                
                
                
                pFrameYUV = av_frame_alloc();
                out_buffer = new uint8_t[avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height)];//分配AVFrame所需内存
                avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);//填充AVFrame

                //像素格式转换(yuv->yuv)
                sws_scale(img_convert_ctx,
                          (const uint8_t* const*)pFrame->data, pFrame->linesize,
                          0, pCodecCtx->height,
                          pFrameYUV->data, pFrameYUV->linesize);
                
                static FILE *fp_yuv = fopen("glw.yuv", "wb");
                if(fp_yuv&&false){
                    int y_size=pCodecCtx->width*pCodecCtx->height;
                    fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y
                    fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U
                    fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V
                }
                
                putFrame(pFrameYUV);
            }
        }//end of "if(packet->stream_index == videoindex)"
        av_free_packet(packet);
    }
}


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

RTSPSource::RTSPSource(std::string name, std::string url, bool useTcp){
    this->name = name;
    this->url = url;
    this->isUseTCP = useTcp;
}
RTSPSource::~RTSPSource(){
    
}

void RTSPSource::Start(){
    bRun = true;
    pthread_create(&(this->pth), NULL, (this->Run), this);
}

void RTSPSource::Stop(){
    bRun = false;
    
    pthread_join(pth, NULL);
    
    sws_freeContext(img_convert_ctx);
    
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

AVFrame* RTSPSource::getFrame(){
    AVFrame* frame = nullptr;
    myMutex.lock();
    if(myAVFrame.size()>0){
        frame = myAVFrame.front();
        myAVFrame.pop();
    }
    if(frame){
        std::cout<<" "<<name<<" getFrame "<<frame->pts/90<<" ms"<<std::endl;
    }
    std::cout<<" "<< name <<" getFrame "<<" myAVFrame size is "<<myAVFrame.size()<<std::endl;
    myMutex.unlock();
    frame->pts = frame->pts-basePTS;
    return frame;
}

bool RTSPSource::hasFrame(){
    myMutex.lock();
    size_t size = myAVFrame.size();
    myMutex.unlock();
    return size>0;
}

int64_t RTSPSource::clearFrameAndBasePTS(){
    int64_t pts = -1;
    myMutex.lock();
    while(myAVFrame.size()>1){
        AVFrame* frame = myAVFrame.front();
        myAVFrame.pop();
        delete[] frame->data[0];
        av_free(frame);
        frame = nullptr;
    }
    std::cout<<name <<" clearFrame "<<" myAVFrame size is "<<myAVFrame.size()<<std::endl;
    pts = myAVFrame.front()->pts;
    myMutex.unlock();
    basePTS = pts;
    return pts;
}

void RTSPSource::putFrame(AVFrame* frame){
    myMutex.lock();
    myAVFrame.push(frame);
    std::cout<<name <<" putFrame "<<" myAVFrame size is "<<myAVFrame.size()<<std::endl;
    myMutex.unlock();
}

void *RTSPSource::Run(void* context){
    RTSPSource *source = (RTSPSource*)context;
    
    
    source->pFormatCtx = avformat_alloc_context();
    
    AVDictionary *format_opts = NULL;
    //av_dict_set(&format_opts, "stimeout", std::to_string( 2* 1000000).c_str(), 0); //设置链接超时时间（us）
    if(source->isUseTCP){
        av_dict_set(&format_opts, "rtsp_transport",  "tcp", 0); //设置推流的方式，默认udp。
    }
    
    int ret = avformat_open_input(&source->pFormatCtx, source->url.c_str(), nullptr, &format_opts);
    std::cout<<"avformat_open_input result is "<<ret<<std::endl;
    
    ret = avformat_find_stream_info(source->pFormatCtx, nullptr);
    std::cout<<"avformat_find_stream_info result is "<<ret<<std::endl;
    
    //查找视频流索引
    for(int i=0; i<source->pFormatCtx->nb_streams; i++){
        if(source->pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
            source->videoindex = i;
            break;
        }
    }
    std::cout<<"videoindex is "<<source->videoindex<<std::endl;
    
    //codec 上下文初始化
    source->pCodecCtx=source->pFormatCtx->streams[source->videoindex]->codec;
    //codec 初始化
    //to do : 确认解码器是否需要释放
    source->pCodec=avcodec_find_decoder(source->pCodecCtx->codec_id);
    std::cout<<"pCodecCtx->codec_id is "<< source->pCodecCtx->codec_id<<" (如果是27，就表示是h264编码)"<<std::endl;
    
    //打开解码器
    ret = avcodec_open2(source->pCodecCtx, source->pCodec, nullptr);
    std::cout<<"avcodec_open2 result is "<<ret<<std::endl;
    
    //frame初始化
    source->pFrame = av_frame_alloc();
    
    //packet初始化
    source->packet = (AVPacket*)av_malloc(sizeof(AVPacket));
    
    //打印一下输入文件的信息
    av_dump_format(source->pFormatCtx, 0, source->url.c_str(), 0);
    
    //初始化图像转换上下文
    source->img_convert_ctx = sws_getContext(source->pCodecCtx->width, source->pCodecCtx->height, source->pCodecCtx->pix_fmt,
                                     source->swsDstWidth, source->swsDstHeight, AV_PIX_FMT_YUV420P, SWS_BICUBIC,
                                     NULL, NULL, NULL);
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
            }
            if(got_picture){//解码成功
                //static int count = 0;
                //std::cout<<name<<" decode "<<count++<<" frame"<<std::endl;
                
                pFrameYUV = av_frame_alloc();
                out_buffer = new uint8_t[avpicture_get_size(AV_PIX_FMT_YUV420P, swsDstWidth, swsDstHeight)];//分配AVFrame所需内存
                avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, swsDstWidth, swsDstHeight);//填充AVFrame

                //像素格式转换(yuv->yuv)
                sws_scale(img_convert_ctx,
                          (const uint8_t* const*)pFrame->data, pFrame->linesize,
                          0, pCodecCtx->height,
                          pFrameYUV->data, pFrameYUV->linesize);
                pFrameYUV->pts = pFrame->pts;
                putFrame(pFrameYUV);
            }
        }//end of "if(packet->stream_index == videoindex)"
        av_free_packet(packet);
    }
}


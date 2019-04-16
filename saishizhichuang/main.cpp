//
//  main.cpp
//  saishizhichuang
//
//  Created by gaoliwen on 2019/4/15.
//  Copyright © 2019 gaoliwen. All rights reserved.
//

#include <iostream>
extern "C"
{
#include<libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

int main(int argc, const char * argv[]) {
    char filepath[]="/Users/gaoliwen/work/testfile/gaoliwen.flv";
    
    av_register_all();
    avformat_network_init();
    
    //上下文
    AVFormatContext *pFormatCtx = nullptr;//format context
    AVCodecContext *pCodecCtx = nullptr;//codec context
    AVCodec *pCodec = nullptr;//codec
    AVFrame    *pFrame = nullptr, *pFrameYUV = nullptr;//frame 编码前的数据，如yuv数据
    AVPacket *packet = nullptr;//编码后的数据，如h264数据
    struct SwsContext *img_convert_ctx = nullptr;//图像转换上下文
    
    //format 上下文初始化
    pFormatCtx = avformat_alloc_context();
    int ret = avformat_open_input(&pFormatCtx, filepath, nullptr, nullptr);
    std::cout<<"avformat_open_input result is "<<ret<<std::endl;
    
    ret = avformat_find_stream_info(pFormatCtx, nullptr);
    std::cout<<"avformat_find_stream_info result is "<<ret<<std::endl;
    
    //查找视频流索引
    int videoindex = -1;
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
    pFrameYUV = av_frame_alloc();
    
    //to do
    //逻辑
    
    //packet初始化
    packet = (AVPacket*)av_malloc(sizeof(AVPacket));
    
    //打印一下输入文件的信息
    av_dump_format(pFormatCtx, 0, filepath, 0);
    
    //初始化图像转换上下文
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                     pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC,
                                     NULL, NULL, NULL);

    //主逻辑
    int got_picture = -1;
    while (av_read_frame(pFormatCtx, packet)>=0) {
        if(packet->stream_index == videoindex){
            //解码：h264->yuv
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            if(ret<0){
                std::cout<<"严重错误"<<std::endl;
                //return -1;
                break;
            }
            if(got_picture){//解码成功
                static int count = 0;
                std::cout<<count++<<" frame"<<std::endl;
            }
        }
    }
    
    
    
    //avcodec_free_context(&pCodecCtx);//不需要
    avcodec_close(pCodecCtx);
    pCodecCtx = nullptr;
    
    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);
    pFormatCtx = nullptr;
    
    return 0;
}

//
//  source.hpp
//  saishizhichuang
//
//  Created by gaoliwen on 2019/4/17.
//  Copyright © 2019 gaoliwen. All rights reserved.
//

#ifndef source_hpp
#define source_hpp

#include <stdio.h>
#include <queue>

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

class RTSPSource{
public:
    RTSPSource(std::string name, std::string url, bool useTcp);
    ~RTSPSource();
    void Init();
    void Uninit();
    void Start();
    void Stop();

    void Loop();
    
public:
    AVFrame* getFrame();
    bool hasFrame();
    int64_t clearFrameAndBasePTS();
private:
    void putFrame(AVFrame* frame);
    
private:
    static void *Run(void* context);
    
private:
    //上下文
    AVFormatContext *pFormatCtx = nullptr;//format context
    AVCodecContext *pCodecCtx = nullptr;//codec context
    AVCodec *pCodec = nullptr;//codec
    AVFrame    *pFrame = nullptr;
    AVFrame *pFrameYUV = nullptr;//frame 编码前的数据，如yuv数据
    AVPacket *packet = nullptr;//编码后的数据，如h264数据
    struct SwsContext *img_convert_ctx = nullptr;//图像转换上下文
    
    //视频索引
    int videoindex = -1;
    
    //????
    uint8_t *out_buffer = nullptr;
    
    //thread handle
    pthread_t pth;
    
    bool bRun = false;
    
    std::queue<AVFrame*> myAVFrame;
    std::mutex myMutex;
    
    std::string name = "";
    std::string url = "";
    bool isUseTCP = false;
    
    int swsDstWidth = 1920;
    int swsDstHeight = 1080;
    int64_t basePTS = 0;
};

#endif /* source_hpp */

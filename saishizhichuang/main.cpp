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
}

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    char filepath[]="/Users/gaoliwen/work/testfile/gaoliwen.flv";
    
    AVFormatContext *pFromatCtx = nullptr;
    pFromatCtx = avformat_alloc_context();
    int ret = avformat_open_input(&pFromatCtx, filepath, nullptr, nullptr);
    
    std::cout<<"result is "<< ret<<std::endl;
    
    avformat_close_input(&pFromatCtx);
    avformat_free_context(pFromatCtx);
    pFromatCtx = nullptr;
    
    return 0;
}

//
//  CombineImage.hpp
//  saishizhichuang
//
//  Created by gaoliwen on 2019/4/20.
//  Copyright © 2019 gaoliwen. All rights reserved.
//

#ifndef CombineImage_hpp
#define CombineImage_hpp

#include <stdio.h>

class CombineImage{
public:
    CombineImage();
    ~CombineImage();
    
    void Start();
    void Stop();
    void Loop();
    
private:
    static void *Run(void* context);
};

#endif /* CombineImage_hpp */

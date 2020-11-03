//
//  file.hpp
//  VFS
//
//  Created by Adam Lev-Ari on 01/11/2020.
//  Copyright © 2020 Adam Lev-Ari. All rights reserved.
//

#ifndef file_hpp
#define file_hpp

#include <stdio.h>

class file{
public:
    char name[64];
    long int len;
    int startPos;
    bool flag;

    char* getName();
    long int getLength();
    int getStartPos();
    bool isOpen();
    void flagOff();
    void flagOn();
    
};

#endif /* file_hpp */

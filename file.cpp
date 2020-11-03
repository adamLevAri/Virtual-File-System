//
//  file.cpp
//  VFS
//
//  Created by Adam Lev-Ari on 01/11/2020.
//  Copyright © 2020 Adam Lev-Ari. All rights reserved.
//

#include "file.hpp"

char* file::getName()
{
    return name;
}
long int file::getLength()
{
    return len;
}
int file::getStartPos()
{
    return startPos;
}
bool file::isOpen()
{
    return flag;
}
void file::flagOn()
{
    this->flag = true;
}
void file::flagOff()
{
    this->flag = false;
}

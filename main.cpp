//
//  main.cpp
//  VFS
//
//  Created by Adam Lev-Ari on 01/11/2020.
//  Copyright © 2020 Adam Lev-Ari. All rights reserved.
//

#include <iostream>
#include <string.h>
#include <thread>
#include "System.hpp"

using namespace std;

int main(int argc, const char * argv[]) {
    
    //load existing VFS from hard disk named: "d"
    System s1("d");
    
    //to set new VFS pass as arg NULL
    //System s1(NULL);
    
    
    //multithread append, create, read, list to the same VFS:
    std::thread first(&System::newFile, &s1, (char*)("f2"), (char*)("f2 hello"));
    std::thread fifth(&System::printFile, &s1, (char*)("f2"));
    //std::thread sixth(&System::deleteFile, &s1, (char*)("f2"));
    
    //std::thread third(&System::printFile, &s1, (char*)("f1"));
    std::thread second(&System::appendFile, &s1, (char*)("f2"), (char*)("2 "));
    std::thread second2(&System::appendFile, &s1, (char*)("f2"), (char*)("3 "));
    //std::thread fourth(&System::listVFS, &s1);
    
    
    second.join();
    second2.join();
    //third.join();
    //fourth.join();
    first.join();
    fifth.join();
    //sixth.join();
    
    //can run on the main thread as well
    //s1.searchOffset("f3", 3);
    //s1.listVFS();
    
    return 0;
}

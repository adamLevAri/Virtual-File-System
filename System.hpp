//
//  System.hpp
//  VFS
//
//  Created by Adam Lev-Ari on 01/11/2020.
//  Copyright © 2020 Adam Lev-Ari. All rights reserved.
//
#define MAX_MEMORY 16
#define MAX_FILE_LEN 1000

#ifndef System_hpp
#define System_hpp
#include "file.hpp"
#include <mutex>
#include <condition_variable>

using namespace std;

class System{
public:
    file fileTable[MAX_MEMORY];
    char systemName[20];
    std::mutex disklock;
    std::condition_variable cv;
    
    //set the VFS system, name, intialize the memory
    System(char fileName[]);
    void system_intialize();
    void setSystem();
    void setSystemName();
    
    //print the VFS
    void listVFS();
    
    //read write to VFS
    void read_from_VFS();
    void write_to_VFS();
    
    //I/O operations
    void newFile(char* name, char* content);
    void printFile(char* name);
    void deleteFile(char* name);
    void appendFile(char* name, char* content);
    void searchOffset(char* name, int offset);
    
};

#endif /* system_hpp */

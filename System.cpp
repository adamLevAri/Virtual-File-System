//
//  system.cpp
//  VFS
//
//  Created by Adam Lev-Ari on 01/11/2020.
//  Copyright © 2020 Adam Lev-Ari. All rights reserved.
//

#include "System.hpp"
#include<iostream>
#include<fstream>
#include<string.h>
#include<stdio.h>
#include<string>
#include<math.h>
#include <chrono>
#include <thread>

using namespace std;

System:: System(char fileName[])
{
    if(fileName == NULL){
        setSystemName();
    }else {
        cout << "Existing File System" << endl;
        strcpy(systemName, fileName);
        read_from_VFS();
    }
}

//set a new system VFS
void System::setSystemName()
{
        cout << "Enter file system name" << endl;
        char temp[80];
        cin >> temp;
        strcpy(systemName, temp);
        system_intialize();
}

void System::system_intialize()
{
    fstream myVFS(systemName, ios::out);
    int i;
    //sets the position to the start of 'myfile'
    myVFS.seekp(0, ios::beg);
    //fill the memory
    for(i = 0; i < MAX_MEMORY; i++)
    {
        strcpy(fileTable[i].name, "\0");
        fileTable[i].len = 0;
        fileTable[i].startPos = 0;
        fileTable[i].flagOff();
    }
    myVFS.close();
    write_to_VFS();
}

void System::listVFS()
{
    for(int i = 0; i < MAX_MEMORY; i++)
    {
        if(!strcmp(fileTable[i].getName(), "\0"))
            break;
        cout << fileTable[i].getName() << endl;
    }
}

void System::newFile(char* name, char* content)
{
    int i;
    std::unique_lock<std::mutex> ul(disklock);
    for(i = 0; i < MAX_MEMORY; i++){
        if(!strcmp(fileTable[i].getName(), "\0"))
            break;
        //FLAT file system
        if (!strcmp(fileTable[i].getName(), name)){
            cout << "No duplicate files allowed" << endl;
            return;
        }
    }
    if(i == MAX_MEMORY)
        cout<<"No more memory in VFS!" << endl;
    else
    {
        //mange threads by mutex and update file open flag
        cv.wait(ul, [this, &i] { return !fileTable[i].isOpen(); });
        fileTable[i].flagOn();
        
        fstream myVFS(systemName, ios::out | ios::app);
        strcpy(fileTable[i].name, name);
        fileTable[i].len = strlen(content);
        myVFS.seekp(0, ios::end);
        fileTable[i].startPos = (int)myVFS.tellp();
        myVFS.write(content, sizeof(char) * strlen(content));
        myVFS.close();
        write_to_VFS();
        
        //release the lock
        fileTable[i].flagOff();
        cv.notify_all();
    }
}

void System::read_from_VFS()
{
    std::lock_guard<std::mutex>lock(disklock);
    
    fstream myVFS(systemName, ios::in);
    int i;
    myVFS.seekg(0 , ios::beg);
    for(i = 0; i < MAX_MEMORY; i++)          //read already created files till null string is encountered or max limit
    {
        myVFS.read((char*)&(fileTable[i].name) , sizeof(fileTable[i].name));
        if(!strcmp(fileTable[i].name,"\0"))
            break;
        myVFS.read((char*)&fileTable[i].len , sizeof(long int));
        myVFS.read((char*)&fileTable[i].startPos , sizeof(int));
    }
    while(i < MAX_MEMORY)
    {
        strcpy(fileTable[i].name,"\0");
        fileTable[i].len = 0;
        fileTable[i].startPos = 0;
        i++;
    }
    myVFS.close();
}

void System::write_to_VFS()
{
    fstream myVFS(systemName, ios::in);
    fstream newVFS("temp.txt", ios::out);
    int i;
    char content[MAX_FILE_LEN];
    newVFS.seekp(0 , ios::beg);
    cout << std::this_thread::get_id() << "writting to disk" << endl;
    for(i = 0; i < MAX_MEMORY; i++)
    {
        newVFS.write((char*)&fileTable[i].name , sizeof(fileTable[i].name));
        newVFS.write((char*)&fileTable[i].len , sizeof(long int));
        newVFS.write((char*)&fileTable[i].startPos , sizeof(int));
    }
    for(i = 0; i < MAX_MEMORY; i++)
    {
        if(fileTable[i].getLength())
        {
            myVFS.seekg(fileTable[i].getStartPos(), ios::beg);
            myVFS.read((char*)&content, sizeof(char) * fileTable[i].len);
            newVFS.seekp(0 , ios::end);
            newVFS.write((char*)&content, sizeof(char) * fileTable[i].len);
        }
    }
    
    newVFS.close();
    myVFS.close();
    remove(systemName);
    rename("temp.txt", systemName);
}

void System::printFile(char* name)
{
    std::unique_lock<std::mutex> ul(disklock);
    fstream myVFS(systemName, ios::in);
    int i;
    char* content = new char[MAX_FILE_LEN];
    for(i = 0; i < MAX_MEMORY; i++)
    {
        if(!strcmp(fileTable[i].getName(), name))
        {
            //mange threads by mutex and update file open flag
            cv.wait(ul, [this, &i] { return !fileTable[i].isOpen(); });
            fileTable[i].flagOn();
            
            myVFS.seekg(fileTable[i].getStartPos(), ios::beg);
            myVFS.read(content, fileTable[i].getLength());
            *(content + fileTable[i].getLength()) = '\0';
            cout << name << ": " << content << endl;
            
            //release the lock
            fileTable[i].flagOff();
            cv.notify_all();
            break;
        }
    }
    if(i == MAX_MEMORY){
        cout<<"File not found!"<<endl;
    }
    myVFS.close();
}

void System::deleteFile(char* name)
{
    int i,j;
    std::unique_lock<std::mutex> ul(disklock);
    for(i = 0; i < MAX_MEMORY; i++)
    {
        if(!strcmp(fileTable[i].getName(), name))
        {
            long tempLen;
            
            //mange threads by mutex and update file open flag
            cv.wait(ul, [this, &i] { return !fileTable[i].isOpen(); });
            fileTable[i].flagOn();
            
            strcpy(fileTable[i].name, "\0");
            tempLen = fileTable[i].getLength();
            fileTable[i].len = 0;
            fileTable[i].startPos = 0;
            write_to_VFS();        //Will skip the content of the file to be deleted
            
            for(j = i + 1; j < MAX_MEMORY ; j++)
            {
                strcpy(fileTable[j - 1].name, fileTable[j].getName());
                fileTable[j - 1].len = fileTable[j].getLength();
                fileTable[j - 1].startPos = (int)(fileTable[j].getStartPos() - tempLen);
            }
            
            write_to_VFS();
            cout << "\nFile deleted!"<<endl;
            
            //release the lock
            fileTable[i].flagOff();
            cv.notify_all();
            
            break;
        }
    }
    if (i == MAX_MEMORY){
        cout<<"File not found!"<<endl;
    }
}

void System::appendFile(char* name, char* content)
{
    std::unique_lock<std::mutex> ul(disklock);
    int i;
    for(i = 0; i < MAX_MEMORY; i++)
    {
        if(!strcmp(fileTable[i].getName(), name))
        {
            //mange threads by mutex and update file open flag
            cv.wait(ul, [this, &i] { return !fileTable[i].isOpen(); });
            fileTable[i].flagOn();
            
            cout << "appending to file" << endl;
            fstream myVFS(systemName, ios::in | ios::out );
            char* temp = new char[MAX_FILE_LEN];
            long tempLen = fileTable[i].getLength() + strlen(content);
            
            myVFS.seekp(fileTable[i].getStartPos(), ios::beg);
            myVFS.read(temp, sizeof(char) * fileTable[i].getLength());
            myVFS.seekp(fileTable[i].getStartPos(), ios::beg);
            fileTable[i].len = tempLen;
            
            //concat new content and previos content
            char* resualt = new char[MAX_FILE_LEN];
            strcpy(resualt, content);
            strcat(resualt, temp);
            
            myVFS.write(resualt, sizeof(char) * tempLen);
            remove(resualt);
            remove(temp);
            myVFS.close();
            write_to_VFS();
            
            //notify pending threads and update file status
            fileTable[i].flagOff();
            cv.notify_all();
            break;
        }
    }
    if (i == MAX_MEMORY){
        cout<<"File not found!"<<endl;
    }
}
    

void System::searchOffset(char* name, int offset)
{
    if(offset >= MAX_FILE_LEN || offset < 0){
        cout << "offset is out of bounce" << endl;
        return;
    }
    int i;
       for(i = 0; i < MAX_MEMORY; i++)
       {
           if(!strcmp(fileTable[i].getName(), name))
           {
               if(offset >= fileTable[i].getLength()){
                   cout << "empty position" << endl;
                   return;
               }
               fstream myVFS(systemName, ios::in | ios::out );
               char* resualt = new char[MAX_FILE_LEN];
               
               myVFS.seekp(fileTable[i].getStartPos()+offset, ios::beg);
               myVFS.read(resualt, sizeof(char) * fileTable[i].getLength());
               
               cout << name << "[" << offset << "]" << ": " << resualt << endl;
               remove(resualt);
               myVFS.close();
               break;
           }
       }
    if (i == MAX_MEMORY){
        cout<<"File not found!"<<endl;
    }

}

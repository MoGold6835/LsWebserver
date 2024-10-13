#ifndef BUFFER_H
#define BUFFER_H
#include<cstring>  //perror
#include<iostream>
#include<unistd.h>  //writr
#include<sys/uio.h> //readv
#include<vector>    //readv
#include<atomic>
#include<assert.h>
using namespace std;

class Buffer
{
public:
    Buffer(int initBuffSize=1024);
    ~Buffer()=default;

    size_t WritableBytes() const; //size_t 无符号整数，用于循环计数器，内存分配，数组索引    const表示函数类型为常成员函数，不能改变成员变量值得函数
    size_t ReadableBytes() const;
    size_t PrependableBytes() const;

    const char* Peek() const;
    void EnsureWriteable(size_t len);
    void HasWritten(size_t len);

    void Retrieve(size_t len);
    void RetrieveUntil(const char* end);

    void RetrieveAll();
    std::string RetrieveAllToStr();


    const char* BeginWriteConst() const;
    char* BeginWrite();

    void Append(const string& str);
    void Append(const char* str,size_t len);
    void Append(const void* data,size_t len);
    void Append(const Buffer& buff);

    ssize_t ReadFd(int fd,int* Errno);  //32位系统size_t 表示无符号整数，64位系统表示长整型     ssize_t表示signed size_t
    ssize_t WriteFd(int fd,int* Errno);

private:
    char* BeginPtr_();   //buffer得开头
    const char* BeginPtr_() const;
    void MakeSpace_(size_t len);


    vector<char> buffer_;
    atomic<size_t> readPos_; //读得下标
    atomic<size_t> writePos_;  //写的下标
};
#endif  //防止被重复定义

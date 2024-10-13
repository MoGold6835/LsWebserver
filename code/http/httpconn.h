#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include<sys/types.h>
#include<sys/uio.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<errno.h>

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "httprequest.h"
#include "httpresponse.h"
using namespace std;


/*
进行读写数据并调用httprequest 来解析数据以及httpresponse来生成响应
*/
class HttpConn{
    public:
        HttpConn();
        ~HttpConn();

        void init(int sockFd,const sockaddr_in& addr);
        ssize_t read(int* saveErrno);
        ssize_t write(int* saveErrno);
        void Close();
        int GetFd() const;
        int GetPort() const;
        const char* GetIP() const;
        sockaddr_in GetAddr() const;
        bool process();


        int ToWriteBytes(){
            return iov_[0].iov_len+iov_[1].iov_len;
        }

        bool IsKeepAlive() const{
            return request_.IsKeepAlive();
        }

        static bool isET;
        static const char* srcDir;
        static atomic<int> userCount;  // 原子，支持锁

    private:
        int fd_;
        struct sockaddr_in addr_;

        bool isClose_;

        int iovCnt_;
        struct iovec iov_[2];

        Buffer readBuff_;  // 读缓冲区
        Buffer writeBuff_;  // 写缓冲区

        HttpRequest request_;
        HttpResponse response_;
};


#endif
#pragma once

#include <iostream>
#include <fstream>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>
#include <syncstream>
#include <fcntl.h>
#include <poll.h>
#include <netinet/tcp.h>

// 设置是否非阻塞,0关，1开,默认为非阻塞
#define NONBLOCK 1

struct file
{
    char name[256];
    struct stat fileinfo;
};

class stcp
{
public:
    stcp() : _serverfd(-1), _clientfd(-1) { memset(&_serv, 0, sizeof(sockaddr_in)); }
    stcp(const stcp &tmp)
    {
        _serverfd = tmp._serverfd;
        _clientfd = tmp._serverfd;
        _serv = tmp._serv;
    }
    ~stcp()
    {
        close(_serverfd), close(_clientfd);
        _serverfd = -1, _clientfd = -1;
        memset(&_serv, 0, sizeof(sockaddr_in));
    }
    // 设置IP和端口
    bool SetAddr(const char *ip, const unsigned short port);
    // 初始化服务端，绑定监听的端口,返回服务端文件描述符
    int Init(const unsigned short port);
    // 接受客户端连接,并返回客户端文件描述符，若已连接，则直接返回文件描述符
    int Accept();
    // 设置对端客户端
    void SetClient(int fd) { _clientfd = fd; if(_clientfd < 0) printf("Set Client Error!\n");}
    // 获取服务端文件描述符
    int GetServer() { return _serverfd; }
    // 获取客户端文件描述符
    int GetClient() { return _clientfd; }

    // 向客户端发送数据,false失败 true成功
    bool Send(const std::string &buff);
    bool Send(const void *buff, const size_t size);
    bool Sendfile(const file filebuf);

    // 从客户端接收数据,false失败 true成功
    bool Recv(std::string &buff);
    bool Recv(void *buff, const size_t size);
    bool Recvfile(const file filebuf);

    // 关闭fd
    void CloseCfd() { close(_clientfd), _clientfd = -1; }
    void CloseSfd() { close(_serverfd), _serverfd = -1; }

    constexpr stcp &operator=(const stcp &tmp)
    {
        _serverfd = tmp._serverfd;
        _clientfd = tmp._serverfd;
        _serv = tmp._serv;
        return *this;
    }

private:
    int _serverfd;     // 服务端监听fd
    int _clientfd;     // 客户端连接的fd
    sockaddr_in _serv; // 客户端ip,端口等
};
#pragma once
#include <iostream>
#include <fstream>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>

// 设置是否非阻塞,0关，1开,默认为非阻塞
#define NONBLOCK 1

struct file
{
    char name[256];
    struct stat fileinfo;
};

class ctcp
{
public:
    ctcp() : _clientfd(-1), _ip(0), _port(0) {}
    ~ctcp()
    {
        close(_clientfd);
        _clientfd = -1;
        _ip = 0;
        _port = 0;
    }

    // 获得客户端fd
    int Getfd() { return _clientfd; }

    // 向服务端发送连接请求，成功返回true,失败返回false
    bool Connect(const char *lip, const unsigned short lport);

    // 向服务端发送数据信息
    bool Send(const std::string &buff);
    bool Send(const void *buff, const size_t size);
    bool Sendfile(const file filebuf);

    // 从服务端接受数据信息
    bool Recv(std::string &buff);
    bool Recv(void *buff, const size_t size);
    bool Recvfile(const file filebuf);

private:
    int _clientfd;  // 客户端fd
    uint32_t _ip;   // 服务端域名/ip
    uint16_t _port; // 服务端端口
};
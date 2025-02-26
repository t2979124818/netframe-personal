#include "stcp.h"

//设置IP和端口
bool stcp::SetAddr(const char * ip,const unsigned short port)
{
    hostent * h = nullptr;
    if (nullptr == (h = gethostbyname2(ip, AF_INET)))
    {
        printf("Server Get host error!\n");
        return 0;
    }
    _serv.sin_family = AF_INET;
    _serv.sin_port = htons(port);
    memcpy(&_serv.sin_addr, h->h_addr, h->h_length);
    return 1;
}

int stcp::Init(const unsigned short port)
{
    if (-1 != _serverfd)
    {
        printf("Socket has opened!\n");
        return 0;
    }
#if (NONBLOCK == 1)
    if (-1 == (_serverfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)))
    {
        printf("Socket Open Error!\n");
        return 0;
    }
    
    int opt = 1;
    setsockopt(_serverfd,SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT | TCP_NODELAY,&opt,static_cast<socklen_t>(sizeof opt));

#else
    if (-1 == (_serverfd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        printf("Socket Open Error!\n");
        return 0;
    }
#endif
    if(_serv.sin_port == 0)
    {
        _serv = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = INADDR_ANY};
    }
    if (bind(_serverfd, (const sockaddr *)&_serv, sizeof(_serv)) < 0)
    {
        printf("Bind Error!\n");
        return 0;
    }
    if (listen(_serverfd, 5) == -1)
    {
        printf("Listen Error!\n");
        return 0;
    }
    return _serverfd;
}

int stcp::Accept()
{
    if (-1 != _clientfd)
    {
        printf("Have Connected!\n");
        return _clientfd;
    }
#if (NONBLOCK == 1)
    std::osyncstream sy_out(std::cout);
    socklen_t len = sizeof(sockaddr);
    if (0 != (_clientfd = accept4(_serverfd, (sockaddr *)&_serv, &len,SOCK_NONBLOCK)))
    {
        if (errno != EAGAIN)
        {
            sy_out << "Accept Error!\n";
            return 0;
        }
    }
#else
    std::osyncstream sy_out(std::cout);
    socklen_t len = sizeof(sockaddr);
    if (-1 == (_clientfd = accept(_serverfd, (sockaddr *)&_serv, &len)))
    {
        sy_out << "Accept Error!\n";
        return 0;
    }
#endif
    return _clientfd;
}

bool stcp::Send(const std::string &buff)
{
    if (buff.size() == 0)
    {
        printf("Buffer NULL!\n");
        return 0;
    }
    if (-1 == _clientfd)
    {
        printf("Not Connect!\n");
        return 0;
    }
#if (NONBLOCK == 1)
    pollfd fds;
    fds.fd = _clientfd;
    fds.events = POLLOUT;
    int sended;
    int totle = 0;
    while (1)
    {
        poll(&fds, 0, -1);
        if (fds.revents == POLLOUT)
        {
            if ((sended = send(_clientfd, buff.data() + totle, buff.size() - totle, 0)) > 0)
            {
                if (errno == EAGAIN) // 缓冲区已满
                    continue;
                if (sended == -1) // 发送失败
                {
                    printf("send error!\n");
                    return 0;
                }
                totle += sended;
                if (totle == buff.size()) // 发完了
                    break;
            }
        }
    }
#else
    if (send(_clientfd, &buff[0], buff.size(), 0) <= 0)
    {
        printf("Send Error!\n");
        return 0;
    }
#endif
    return 1;
}
bool stcp::Send(const void *buff, const size_t size)
{
    if (buff == 0)
    {
        printf("Buffer NULL!\n");
        return 0;
    }
    if (-1 == _clientfd)
    {
        printf("Not Connect!\n");
        return 0;
    }
#if (NONBLOCK == 1)
    pollfd fds;
    fds.fd = _clientfd;
    fds.events = POLLOUT;
    int sended;
    int totle = 0;
    while (1)
    {
        poll(&fds, 0, -1);
        if (fds.revents == POLLOUT)
        {
            if ((sended = send(_clientfd, buff + totle, size - totle, 0)) > 0)
            {
                if (errno == EAGAIN) // 缓冲区已满
                    continue;
                if (sended == -1) // 发送失败
                {
                    printf("send error!\n");
                    return 0;
                }
                totle += sended;
                if (totle == size) // 发完了
                    break;
            }
        }
    }
#else
    if (send(_clientfd, buff, size, 0) <= 0)
    {
        printf("Send Error!\n");
        return 0;
    }
#endif
    return 1;
}
bool stcp::Sendfile(const file filebuf)
{
    if (strcmp("", filebuf.name) == 0)
    {
        printf("SEND FILENAME NULL!\n");
        return 0;
    }

    std::ifstream fin(filebuf.name, std::ios::binary);
    char buff[BUFSIZ];
    int onread = BUFSIZ;
    int readed = 0;
    do
    {
        memset(buff, 0, sizeof(buff));
        if (filebuf.fileinfo.st_size - readed > BUFSIZ)
            onread = BUFSIZ;
        else
            onread = filebuf.fileinfo.st_size - readed;
        fin.read(buff, onread);
        if (Send(buff, onread) == 0)
            return 0;
        readed += onread;

    } while (readed != filebuf.fileinfo.st_size);
    fin.close();
    return 1;
}

#define MAX_LEN 1024
bool stcp::Recv(std::string &buff)
{
    if (-1 == _clientfd)
    {
        printf("Not Connect!\n");
        return 0;
    }

    buff.clear();
    buff.resize(MAX_LEN);
    int len;

    if ((len = recv(_clientfd, &buff[0], MAX_LEN, 0)) <= 0)
    {
        printf("Recv Error!\n");
        return 0;
    }
    buff.resize(len);
    return 1;
}
bool stcp::Recv(void *buff, const size_t size)
{
    if (-1 == _clientfd)
    {
        printf("Not Connect!\n");
        return 0;
    }
    int len;
    if ((len = recv(_clientfd, buff, size, 0)) < 0)
    {
        printf("Recv Error!\n");
        return 0;
    }
    /*if(len == 0)
    {
        printf("Recv NULL!\n");
        return 0;
    }*/
    return 1;
}
bool stcp::Recvfile(const file filebuf)
{
    if (strcmp("", filebuf.name) == 0)
    {
        printf("RECV FILENAME NULL!");
        return 0;
    }
    std::ofstream fout(filebuf.name, std::ios::binary);
    int onwrite = BUFSIZ;
    int writen = 0;
    char buff[BUFSIZ];
    do
    {
        memset(buff, 0, sizeof(buff));
        if (filebuf.fileinfo.st_size > BUFSIZ + writen)
            onwrite = BUFSIZ;
        else
            onwrite = filebuf.fileinfo.st_size - writen;
        if (Recv(buff, onwrite) == 0)
        {
            printf("Recv File Error!\n");
            return 0;
        }
        fout.write(buff, onwrite);
        writen += onwrite;
    } while (writen != filebuf.fileinfo.st_size);
    fout.close();
    return 1;
}
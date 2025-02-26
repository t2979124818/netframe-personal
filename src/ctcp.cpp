#include <ctcp.h>

bool ctcp::Connect(const char *rip, const unsigned short rport)
{
    if (-1 != _clientfd)
        return 0;
    hostent *h;
    if (nullptr == (h = gethostbyname2(rip, AF_INET)))
    {
        printf("get host error!\n");
        return 0;
    }
#if(NONBLOCK == 1)
    if (-1 == (_clientfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)))
    {
        printf("Socket error!\n");
        return 0;
    }
#else
    if (-1 == (_clientfd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        printf("Socket error!\n");
        return 0;
    }
#endif
    sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(rport);
    memcpy(&serveraddr.sin_addr, h->h_addr, h->h_length);

    _ip = serveraddr.sin_addr.s_addr;
    _port = rport;

#if (NONBLOCK == 1)
    if (connect(_clientfd, (const sockaddr *)&serveraddr, sizeof(serveraddr)) != 0)
    {
        if (errno != EINPROGRESS)
        {
            printf("connect error!\n");
            return 0;
        }
    }
    pollfd fds;
    fds.fd = _clientfd;
    fds.events = POLLOUT;
    poll(&fds, 0, -1);
    if (fds.revents == POLLOUT)
        printf("connect success!\n");
    else
        printf("connect fail!\n");
#else
    if (-1 == connect(_clientfd, (const sockaddr *)&serveraddr, sizeof(serveraddr)))
    {
        printf("connect error!\n");
        return 0;
    }
#endif
    return 1;
}

bool ctcp::Send(const std::string &buff)
{
    if (-1 == _clientfd)
    {
        printf("Not Connect!\n");
        return 0;
    }
    if (buff.size() == 0)
    {
        printf("Input NULL!\n");
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
                if (errno == EAGAIN)
                    continue;
                if (sended == -1)
                {
                    printf("send error!\n");
                    return 0;
                }
                totle += sended;
                if (totle == buff.size())
                    break;
            }
        }
    }

#else
    if (send(_clientfd, buff.data(), buff.size(), 0) < 0)
    {
        printf("Send Error!\n");
        return 0;
    }
#endif

    return 1;
}
bool ctcp::Send(const void *buff, const size_t size)
{
    if (-1 == _clientfd)
    {
        printf("Not Connect!\n");
        return 0;
    }
    if (buff == 0)
    {
        printf("Input NULL!\n");
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
                if (errno == EAGAIN)
                    continue;
                if (sended == -1)
                {
                    printf("send error!\n");
                    return 0;
                }
                totle += sended;
                if (totle == size)
                    break;
            }
        }
    }

#else
    if (send(_clientfd, buff, size, 0) < 0)
    {
        printf("Send Error!\n");
        return 0;
    }
#endif
    return 1;
}
bool ctcp::Sendfile(const file filebuf)
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

#define MAX_SIZE 1024
bool ctcp::Recv(std::string &buff)
{
    if (-1 == _clientfd)
    {
        printf("Not Connect!\n");
        return 0;
    }
    buff.clear();
    buff.resize(MAX_SIZE);

    int len;

    if ((len = recv(_clientfd, &buff[0], MAX_SIZE, 0)) <= 0)
    {
        buff.clear();
        printf("Read Error!\n");
        return 0;
    }
    buff.resize(len);

    return 1;
}
bool ctcp::Recv(void *buff, const size_t size)
{
    if (-1 == _clientfd)
    {
        printf("Not Connect!\n");
        return 0;
    }
    if ((recv(_clientfd, buff, size, 0)) <= 0)
    {
        buff = nullptr;
        printf("Read Error!\n");
        return 0;
    }

    return 1;
}
bool ctcp::Recvfile(const file filebuf)
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
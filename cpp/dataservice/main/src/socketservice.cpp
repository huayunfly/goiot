/**
 * Socket service implementation. 
 * 
 */

#include <csignal>
#include <cstdio>
#include <cstring>
#include <unistd.h>    // close()
#include <arpa/inet.h> // inet_ntoa()
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h> // accept()
#include <sys/ioctl.h>
#include "socketservice.h"

namespace goiot
{
int SocketService::Open(const char *hostname, port_t port, int af)
{
    int error, s = -1;
    int sock;
    char _port[6]; /* strlen("65535") */
    struct addrinfo hints, *servinfo, *p;

    snprintf(_port, 6, "%d", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = af;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; /* No effect if bindaddr != NULL */

    if (getaddrinfo(hostname, _port, &hints, &servinfo) != 0)
    {
        errno = EINVAL;
        return -1;
    }

    /* AF_INET for IPV4 
            socket() call does not return an EINTR error, implying that it 
            either restarts itself or blocks signals. 
            SIGPIPE is ignored, if an attempt is made to write to pipe or 
            socket that no process has open for reading, write generate SIGPIPE
            signal.
            */
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((IgnoreSigPipe() == -1) ||
            ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1))
        {
            continue;
        }

        int yes = 1;
        /* 1. Socket is reusable in TIME_WAIT after called close() 
            2. Set the send() recv() delay
            3. Set the send() recv() buffer size
            4. No buffer in the send/recv process.
            5. Delay close socket after send() completed.
            */
        s = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
        if (s == -1)
        {
            error = errno;
            Close(sock);
            errno = error;
            freeaddrinfo(servinfo);
            return -1;
        }

        if ((bind(sock, p->ai_addr, p->ai_addrlen) == -1) ||
            (listen(sock, MAXBACKLOG) == -1))
        {
            error = errno;
            Close(sock);
            errno = error;
            freeaddrinfo(servinfo);
            return -1;
        }

        freeaddrinfo(servinfo);
        return sock;
    }

    freeaddrinfo(servinfo);
    return -1;
}

int SocketService::Accept(int fd, char *hostn, int hostnsize)
{
    /* Structure sockaddr describing a generic socket address. */
    socklen_t len = sizeof(struct sockaddr);
    struct sockaddr_in netclient;
    memset(&netclient, 0, sizeof(struct sockaddr_in));
    int clientsock;

    while (((clientsock = accept(fd, (struct sockaddr *)&netclient, &len)) != -1) &&
           (errno == EINTR))
        ;
    if (clientsock == -1)
    {
        return clientsock;
    }
    addr2name(netclient.sin_addr, hostn, hostnsize);
    return clientsock;
}

int SocketService::Connect(port_t port, const char *hostname)
{
    int retval, error;
    int sock;
    struct sockaddr_in server;
    fd_set sockset;
    memset(&server, 0, sizeof(struct sockaddr));
    if ((name2addr(hostname, &(server.sin_addr.s_addr))) == -1)
    {
        errno = EINVAL;
        return -1;
    }
    server.sin_port = htons((short)port);
    server.sin_family = AF_INET;

    if ((IgnoreSigPipe() == -1) ||
        ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1))
    {
        return -1;
    }
    retval = connect(sock, (struct sockaddr *)&server, sizeof(server)); /* asynchronous */
    if ((retval == -1) && ((errno == EINTR) || (errno == EALREADY)))
    {
        /* Unlike otherr library functions that set errno to EINTR,
            connect() should not be restarted, because the network has
            initiated the TCP 3-way handshake. In this case, the connection
            request complete asynchronously to program execution. The application
            must call select() or poll() to detect the descriptor is read for
            writing.
        */
        FD_ZERO(&sockset);
        FD_SET(sock, &sockset);
        while (
            ((retval = select(sock + 1, NULL, &sockset, NULL, NULL)) == -1) &&
            (errno == EINTR))
        {
            FD_ZERO(&sockset);
            FD_SET(sock, &sockset);
        }
    }
    if (retval == -1)
    {
        error = errno;
        Close(sock);
        errno = error;
        return -1;
    }

    return sock;
}

void SocketService::Close(int sock)
{
    while ((close(sock) == -1) && (errno == EINTR))
        ;
}

/* Like read(2) but make sure 'count' is read before to return
 * (unless error or EOF condition is encountered) */
int SocketService::Read(int sockfd, char *buf, int count)
{
    ssize_t nread, totlen = 0;
    while (totlen != count)
    {
        nread = read(sockfd, buf, count - totlen);
        if (nread == 0)
            return totlen;
        if (nread == -1)
            return -1;
        totlen += nread;
        buf += nread;
    }
    return totlen;
}

/* Like write(2) but make sure 'count' is written before to return
 * (unless error is encountered) */
int SocketService::Write(int sockfd, const char *buf, int count)
{
    ssize_t nwritten, totlen = 0;
    while (totlen != count)
    {
        nwritten = write(sockfd, buf, count - totlen);
        if (nwritten == 0)
            return totlen;
        if (nwritten == -1)
            return -1;
        totlen += nwritten;
        buf += nwritten;
    }
    return totlen;
}

int SocketService::Work(int server_sockfd)
{
    int retval = -1;
    int error;
    fd_set readfds, testfds;
    FD_ZERO(&readfds);
    FD_SET(server_sockfd, &readfds);
    const int HOSTN_SIZE = 128;
    char hostn[HOSTN_SIZE];
    int client_sockfd;
    int maxfd = server_sockfd + 1;

    while (true)
    {
        char ch;
        int nread;
        memcpy(&testfds, &readfds, sizeof(fd_set));

        while (
            ((retval = select(maxfd, &testfds, NULL, NULL, NULL)) == -1) &&
            (errno == EINTR))
        {
            FD_ZERO(&testfds);
            FD_SET(server_sockfd, &testfds);
        }
        if (retval == -1)
        {
            error = errno;
            Close(server_sockfd);
            errno = error;
            return -1;
        }
        for (int fd = 0; fd < maxfd; fd++)
        {
            if (FD_ISSET(fd, &testfds))
            {
                if (fd == server_sockfd)
                {
                    client_sockfd = Accept(server_sockfd, hostn, HOSTN_SIZE);
                    if (client_sockfd > 0)
                    {
                        FD_SET(client_sockfd, &readfds);
                        if (client_sockfd >= maxfd)
                        {
                            maxfd = client_sockfd + 1;
                        }
                    }
                }
                else
                {
                    ioctl(fd, FIONREAD, &nread);
                    if (nread == 0)
                    {
                        Close(fd);
                        FD_CLR(fd, &readfds);
                    }
                    else
                    {
                        /* Working routine: a simple read write */
                        Read(fd, &ch, 1);
                        sleep(1);
                        ch++;
                        Write(fd, &ch, 1);
                    }
                }
            }
        }
    }
}

int SocketService::IgnoreSigPipe()
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    /* sigaction() call does not return an error when interrupted by a signal. */
    if (sigaction(SIGPIPE, (struct sigaction *)NULL, &act) == -1)
    {
        return -1;
    }
    if (act.sa_handler == SIG_DFL)
    {
        act.sa_handler = SIG_IGN;
        if (sigaction(SIGPIPE, &act, (struct sigaction *)NULL) == -1)
        {
            return -1;
        }
    }
    return 0;
}

int SocketService::name2addr(const char *name, in_addr_t *addrp)
{
    struct addrinfo hints;
    struct addrinfo *result;
    struct sockaddr_in *saddrp;
    int s;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;       /* Allow IPv4 only. PF_UNSPEC for IPv4 & IPv6*/
    hints.ai_socktype = SOCK_STREAM; /* TCP Stream socket */
    hints.ai_flags = AI_PASSIVE;     /* For wildcard IP address */
    hints.ai_protocol = 0;           /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(name, NULL, &hints, &result);
    if (s != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }

    /* getaddrinfo() returns a list of address structures.
              A better way is to try each address until we successfully bind(2).
              If socket(2) (or bind(2)) fails, we (close the socket
              and) try the next address. */
    saddrp = (struct sockaddr_in *)(result->ai_addr);
    memcpy(addrp, &saddrp->sin_addr.s_addr, 4);
    freeaddrinfo(result);
    return 0;
}

void SocketService::addr2name(struct in_addr addr, char *name, int namelen)
{
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = 0;
    saddr.sin_addr = addr;
    int s;
    s = getnameinfo((const struct sockaddr *)&saddr, sizeof(saddr), name, namelen, NULL, 0, 0);
    if (s != 0)
    {
        strncpy(name, inet_ntoa(addr), namelen - 1);
        name[namelen - 1] = 0;
    }
}

} // namespace goiot

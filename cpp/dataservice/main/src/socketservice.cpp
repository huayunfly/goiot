/**
 * Socket service implementation. 
 * 
 */

#include <csignal>
#include <cstdio>
#include <cstring>
#include <unistd.h> // close()
#include <arpa/inet.h> // inet_ntoa()
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h> // accept()
#include "socketservice.h"

namespace goiot
{
int SocketService::Open(port_t port)
{
    int error, s;
    struct sockaddr_in server;

    // AF_INET for IPV4
    // socket() call does not return an EINTR error, implying that it
    // either restarts itself or blocks signals.
    if ((IgnoreSigPipe() == -1) || (sock = socket(AF_INET, SOCK_STREAM, 0) == -1))
    {
        return -1;
    }

    bool optval = true;
    s = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(bool));
    if (s == -1)
    {
        error = errno;
        while (!(close(sock) == -1) && (errno == EINTR))
            ;
        errno = error;
        return -1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons((short)port);
    if ((bind(sock, (struct sockaddr *)&server, sizeof(server)) == -1) ||
        (listen(sock, MAXBACKLOG) == -1))
    {
        error = errno;
        while (!(close(sock) == -1) && (errno == EINTR))
            ;
        errno = error;
        return -1;
    }

    return 0;
}

int SocketService::Accept(int fd, char* hostn, int hostnsize)
{
    socklen_t len = sizeof(struct sockaddr); /* Structure describing a generic socket address. */
    struct sockaddr_in netclient;
    memset(&netclient, 0, sizeof(struct sockaddr_in));
    int retval;

    while (((retval = accept(fd, (struct sockaddr *)&netclient, &len)) != -1) &&
           (errno != EINTR))
        ;
    if (retval == -1)
    {
        return retval;
    }
    addr2name(netclient.sin_addr, hostn, hostnsize);

    return 0;
}

int SocketService::Connect(port_t port, const std::string &hostname)
{
    return 0;
}

int SocketService::IgnoreSigPipe()
{
    struct sigaction act;
    int s;
    memset(&act, 0, sizeof(struct sigaction));
    /* sigaction() call does not return an error when interrupted by a signal. */
    s = sigaction(SIGPIPE, (struct sigaction *)NULL, &act);
    if (s == -1)
    {
        return -1;
    }
    if (act.sa_handler == SIG_DFL)
    {
        act.sa_handler = SIG_IGN;
        s = sigaction(SIGPIPE, &act, (struct sigaction *)NULL);
        if (s == -1)
        {
            return -1;
        }
    }
    return 0;
}

int SocketService::name2addr(char *name, in_addr_t *addrp)
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
    s = getnameinfo((struct sockaddr*)&saddr, sizeof(saddr), name, namelen, NULL, 0, 0);
    if (s != 0)
    {
        strncpy(name, inet_ntoa(addr), namelen - 1);
        name[namelen - 1] = 0;
    }

}

} // namespace goiot

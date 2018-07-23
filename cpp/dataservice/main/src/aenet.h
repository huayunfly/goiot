/**
 * TCP socket programming wrapper class.
 * 
 * @modifier Yun Hua
 * @version 2018.07.23
 */

#include <sys/types.h>

namespace goiot
{
class AENet
{
  public:
    static const int ANET_OK = 0;
    static const int ANET_ERR = -1;
    static const int ANET_ERR_LEN = 256;

    /* Flags used with certain functions. */
    static const int ANET_NONE = 0;
    static const int ANET_IP_ONLY = (1 << 0);

    enum CONNCET_FLAG {
        ANET_CONNECT_NONE = 0,
        ANET_CONNECT_NONBLOCK = 1,
        ANET_CONNECT_BE_BINDING = 2 /* Best effort binding. */
    };

  public:
    static int anetTcpConnect(char *err, const char *addr, int port);
    static int anetTcpNonBlockConnect(char *err, const char *addr, int port);
    static int anetTcpNonBlockBindConnect(char *err, const char *addr, int port, char *source_addr);
    static int anetTcpNonBlockBestEffortBindConnect(char *err, const char *addr, int port, char *source_addr);
    static int anetUnixConnect(char *err, char *path);
    static int anetUnixNonBlockConnect(char *err, char *path);
    static int anetRead(int fd, char *buf, int count);
    static int anetResolve(char *err, char *host, char *ipbuf, size_t ipbuf_len);
    static int anetResolveIP(char *err, char *host, char *ipbuf, size_t ipbuf_len);
    static int anetTcpServer(char *err, int port, const char *bindaddr, int backlog);
    static int anetTcp6Server(char *err, int port, const char *bindaddr, int backlog);
    static int anetUnixServer(char *err, char *path, mode_t perm, int backlog);
    static int anetTcpAccept(char *err, int serversock, char *ip, size_t ip_len, int *port);
    static int anetUnixAccept(char *err, int serversock);
    static int anetWrite(int fd, const char *buf, int count);
    static int anetNonBlock(char *err, int fd);
    static int anetBlock(char *err, int fd);
    static int anetEnableTcpNoDelay(char *err, int fd);
    static int anetDisableTcpNoDelay(char *err, int fd);
    static int anetTcpKeepAlive(char *err, int fd);
    static int anetSendTimeout(char *err, int fd, long long ms);
    static int anetPeerToString(int fd, char *ip, size_t ip_len, int *port);
    static int anetKeepAlive(char *err, int fd, int interval);
    static int anetSockName(int fd, char *ip, size_t ip_len, int *port);
    static int anetFormatAddr(char *fmt, size_t fmt_len, char *ip, int port);
    static int anetFormatPeer(int fd, char *fmt, size_t fmt_len);
    static int anetFormatSock(int fd, char *fmt, size_t fmt_len);
    static void anetClose(int fd);

  private:
    static void anetSetError(char *err, const char *fmt, ...);
    static int anetSetBlock(char *err, int fd, int non_block);
    static int anetSetTcpNoDelay(char *err, int fd, int val);
    static int anetSetSendBuffer(char *err, int fd, int buffsize);
    static int anetGenericResolve(char *err, char *host, char *ipbuf, size_t ipbuf_len, int flags);
    static int anetSetReuseAddr(char *err, int fd);
    static int anetCreateSocket(char *err, int domain);
    static int anetTcpGenericConnect(char *err, const char *addr, int port, char *source_addr, int flags);
    static int anetUnixGenericConnect(char *err, char *path, int flags);
    static int anetListen(char *err, int s, struct sockaddr *sa, socklen_t len, int backlog);
    static int anetV6Only(char *err, int s);
    static int _anetTcpServer(char *err, int port, const char *bindaddr, int af, int backlog);
    static int anetGenericAccept(char *err, int s, struct sockaddr *sa, socklen_t *len);

  private:
    AENet(const AENet &);
    AENet &operator=(const AENet &);
};

} // namespace goiot

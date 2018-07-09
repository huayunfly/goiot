/**
 * Socket communication service on Linux.
 * Learn from UNIX Systems Programming.
 * 
 * @author Yun Hua
 * @version 0.1 2018.07.04
 */

#include <string>
#include <netinet/in.h>

namespace goiot
{

typedef unsigned short port_t;

class SocketService
{

  static const int MAXBACKLOG = 5;

public:
  SocketService() : hostsock(-1)
  {
  }

  ~SocketService()
  {
  }

  /**
   * Create a TCP socket bound to port and sets the socket to be passive.
   * combining socket(), bind() and listen().
   * @param hostname: host name.
   * @param port: communication port.
   * @param af: ai_family, AF_INEF or AF_INET6 etc.
   * 
   * @return 0 if it succeeded. otherwise the error code.
   */
  int Open(const char *hostname, port_t port, int af);

  /**
   * Waits for connection request on fd. accept()
   * On return the hostname will be sent back.
   * 
   * @param fd: the file descriptor previously bound to listening port
   * @param hostn: a hostname buffer
   * @param hostnsize: hostname buffer size
   * @return: a communication file descriptor.
   *           -1 on error and set errno
   */
  int Accept(int fd, char *hostn, int hostnsize);

  /**
   * Initiates a connnection to remote server on port and host.
   * The implementation should never return because of interrupted by a signal.
   * @param port: communication port.
   * @hostname: communication hostname.
   * 
   * @return: a communication file descriptor if successful. 
   *          -1 on error
   */
  int Connect(port_t port, const char *hostname);

  /**
   * Close a communication.
   * @param sock: socket file descriptor.
   * 
   */
  void Close(int sock);

/**
 * Like read(2) but make sure 'count' is read before to return
 * (unless error or EOF condition is encountered)
 * 
 * @param fd: file descriptor
 * @param buffer: data buffer.
 * @param count: buffer size.
 * 
 * @return: the total character number read. -1 if it failed.
 */
int Read(int fd, char *buf, int count);

/** Like write(2) but make sure 'count' is written before to return
 * (unless error is encountered) 
 * 
 * @param fd: file descriptor
 * @param buffer: data buffer.
 * @param count: buffer size.
 * 
 * @return: the total character number written. -1 if it failed.
 */
int Write(int fd, const char *buf, int count);

private:
  SocketService(const SocketService &);
  SocketService &operator=(const SocketService &);

  /**
   * Ignore SIGPIPE if the default action (termination process) is effect.
   * 
   * Note: Writing to a network socket that has no readers generates
   * a SIGPIPE signal. If an application does not handle this signal,
   * the remote host can cause the application to terminate by prematurely
   * closing the connection.
   * 
   * @return: 0 if succeeded, otherwise -1.
   */
  int IgnoreSigPipe();

  /**
   * An implementaion using getaddrinfo() - MT safe env local.
   * @param name: the host name.
   * @param addrp: the converted address binary
   * 
   * @return: 0 if it succeeded, otherwise -1.
   */
  int name2addr(const char *name, in_addr_t *addrp);

  /**
   * An implementation using getnameinfo() - MT safe env local.
   * @param in_addr: internet address structure
   * @param name: return host name. If getting hostname failed, it is a doted address.
   * @param namelen: name length.
   * 
   */
  void addr2name(struct in_addr addr, char *name, int namelen);

private:
  int hostsock;
};

} // namespace goiot

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
  SocketService() : sock(-1)
  {
  }

  ~SocketService()
  {
  }

  /**
   * Create a TCP socket bound to port and sets the socket to be passive.
   * combining socket(), bind() and listen().
   * @param port: communication port.
   * 
   * @return 0 if it succeeded. otherwise the error code.
   */
  int Open(port_t port);

  /**
   * Waits for connection request on fd. accept()
   * On return the hostname will be sent back.
   * 
   * @param fd: the file descriptor.
   * @param hostn: returned hostname buffer
   * @param hostnsize: hostname buffer size
   * @return a communication file descriptor.
   */
  int Accept(int fd, char* hostn, int hostnsize);

  /**
   * Initiates a connnection to server on port and host.
   * The implementation should never return because of interrupted by a signal.
   * @param port: communication port.
   * @hostname: communication hostname.
   * 
   * @return a communication file descriptor. 
   */
  int Connect(port_t port, const std::string &hostname);

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
  int name2addr(char *name, in_addr_t *addrp);

  /**
   * An implementation using getnameinfo() - MT safe env local.
   * @param in_addr: internet address structure
   * @param name: return host name. If getting hostname failed, it is a doted address.
   * @param namelen: name length.
   * 
   */
  void addr2name(struct in_addr addr, char *name, int namelen);

private:
  int sock;
};

} // namespace goiot

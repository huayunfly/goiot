/**
 * Socket communication service on Linux.
 * 
 * @author Yun Hua
 * @version 0.1 2018.07.04
 */

namespace goiot
{
class SocketServcie
{
  public:
    SocketService() : sock(-1)
    {
    }

    ~SocketService()
    {
    }

  private:
    SocketService(const SocketServcie &);
    SocketService &operator=(const SocketServcie &);

  private:
    int sock;
};

} // namespace goiot

/**
 * Circular buffer delaration.
 * 
 * @author Yun Hua
 * @version 0.1 2018.06.28
 */

#include <vector>
#include <mutex>
#include <condition_variable>

namespace goiot
{

template <typename T>
class CircularBuffer
{
public:
  explicit CircularBuffer(std::size_t size) : mysize(size), inpos(0),
                                              outpos(0), totalitems(0), doneflag(0)
  {
    buffer.resize(size);
  }

  ~CircularBuffer()
  {
  }

  /**
     * Get an item.
     * @param item: an buffer item
     * @throw std::system_error
     * 
     * @return 0 if it is succeeded, otherwise the error no.
     * ECANCELED if it detects completion.
     */
  int GetItem(T &item);

  /**
     * Put an item.
     * @param item: an buffer item
     * @throw std::system_error
     * 
     * @return 0 if it is succeeded, otherwise the error no.
     * ECANCELED if it detects completion.
     */
  int PutItem(const T &item);

  /**
   * Get the completion status
   * @param flag: 0, it continues. otherwise 1, it completes
   * @return 0 if it is succeeded, otherwise it failed.
   */
  int GetDone(int &flag);

  /**
   * Set the completion
   * @return 0 if it is succeeded. otherwise it failed.
   */
  int SetDone(void);

private:
  CircularBuffer(const CircularBuffer &);
  const CircularBuffer &operator=(const CircularBuffer &);

private:
  std::size_t mysize;
  std::size_t inpos;
  std::size_t outpos;
  std::size_t totalitems;
  int doneflag;
  std::vector<T> buffer;

  std::mutex bufferlock;
  std::condition_variable items;
  std::condition_variable slots;
};
} // namespace goiot
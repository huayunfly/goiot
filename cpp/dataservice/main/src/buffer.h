/**
 * Circular buffer delaration.
 * 
 * @author Yun Hua
 * @version 0.1 2018.06.28
 */

#include <vector>
#include <mutex>
#include <condition_variable>
#include <system_error>

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
  int GetItem(T &item)
  {
    std::unique_lock<std::mutex> lk(bufferlock); // It may throws system_error.
    while ((totalitems <= 0) && !doneflag)
    {
      items.wait(lk);
    }
    if (doneflag && (totalitems <= 0))
    {
      throw std::system_error(std::error_code(ECANCELED, std::generic_category()));
      // return ECANCELED;
    }
    item = buffer.at(outpos);
    outpos = (outpos + 1) % mysize;
    totalitems--;
    slots.notify_one();
    return 0;
  }

  /**
     * Put an item.
     * @param item: an buffer item
     * @throw std::system_error
     * 
     * @return 0 if it is succeeded, otherwise the error no.
     * ECANCELED if it detects completion.
     */
  int PutItem(const T &item)
  {
    std::unique_lock<std::mutex> lk(bufferlock); // It may throws system_error.
    while (totalitems >= mysize && !doneflag)
    {
      slots.wait(lk);
    }
    if (doneflag)
    {
      throw std::system_error(std::error_code(ECANCELED, std::generic_category()));
      // return ECANCELED;
    }
    buffer.at(inpos) = item;
    inpos = (inpos + 1) % mysize;
    totalitems++;
    items.notify_one();
    return 0;
  }

  /**
   * Get the completion status
   * @param flag: 0, it continues. otherwise 1, it completes
   * @return 0 if it is succeeded, otherwise it failed.
   */
  int GetDone(int &flag) const
  {
    std::lock_guard<std::mutex> lk(bufferlock); // It may throws system_error.
    flag = doneflag;
    return 0;
  }

  /**
   * Set the completion
   * @return 0 if it is succeeded. otherwise it failed.
   */
  int SetDone(void)
  {
    std::lock_guard<std::mutex> lk(bufferlock); // It may throws system_error.
    doneflag = 1;
    items.notify_all();
    slots.notify_all();
    return 0;
  }

private:
  CircularBuffer(const CircularBuffer &);
  CircularBuffer &operator=(const CircularBuffer &);

private:
  std::size_t mysize;
  std::size_t inpos;
  std::size_t outpos;
  std::size_t totalitems;
  int doneflag;
  std::vector<T> buffer;

  mutable std::mutex bufferlock;
  std::condition_variable items;
  std::condition_variable slots;
};
} // namespace goiot
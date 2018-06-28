/**
 * Circular buffer delaration.
 * 
 * @author Yun Hua
 * @version 0.1 2018.06.28
 */

#include <vector>

namespace goiot
{

template <typename T>
class CircularBuffer
{
  public:
    explicit CircularBuffer(std::size_t size) : mysize(size), inpos(0),
                                                outpos(0), totalitems(0)
    {
        buffer.resize(size);
    }

    ~CircularBuffer()
    {
    }

    /**
     * Get an item.
     * @param item: an buffer item
     * 
     * @return 0 if it is succeeded, otherwise the error code.
     */
    int GetItem(T &item);

    /**
     * Put an item.
     * @param item: an buffer item
     * 
     * @return 0 if it is succeeded, otherwise the error code.
     */
    int PutItem(const T &item);

  private:
    CircularBuffer(const CircularBuffer &);
    const CircularBuffer &operator=(const CircularBuffer &);

  private:
    std::size_t mysize;
    std::size_t inpos;
    std::size_t outpos;
    std::size_t totalitems;
    std::vector<T> buffer;
};
} // namespace goiot
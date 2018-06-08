/**
 * Hash table for the key, value pair storage.
 * 
 * @author Yun Hua
 * @version 0.1 2018.06.07
 */

#include <vector>
#include <memory>
#include "tagdef.h"

namespace goiot
{
/**
 * The hash table implementation for the fixed length and non-sizable dictionary.
 * Every insert manipulation will return the table slot index.
 * 
 */
class FixedDict
{
  public:
    explicit FixedDict(std::size_t size);
    ~FixedDict();

  private:
    FixedDict(const FixedDict &);
    const FixedDict &operator=(const FixedDict &);

    private:
        std::vector<std::shared_ptr<TagEntry>> slots;
        std::size_t size;
        std::size_t reserved_size;
        std::size_t used;
        std::size_t filled;
};

} // namespace goiot

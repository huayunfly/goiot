/**
 * Hash table for the key, value pair storage.
 * 
 * @author Yun Hua
 * @version 0.1 2018.06.07
 */

#include <vector>
#include <memory> // for shared_ptr
#include "tagdef.h"

namespace goiot
{
/**
 * The hash table implementation for the fixed length and non-sizable dictionary.
 * Every insert manipulation will return the table slot index.
 * 
 */
class FixedDict final
{
public:
  const static std::size_t PERTURB_SHIFT = 5;
  const static std::size_t DICT_MINSIZE = 8;
  const static std::size_t DICT_MAXSIZE = (1 << 16) - 1;

public:
/**
 * Initialization with a fixed length. The length shall larger than 0.
 * To reduce the open address collisions, we reserve 3/2 length.
 * Considering the numeric_limit<long> is 1 << 31 - 1, the hash mask (for mod) 
 * will be 1 << 30. The maximum length is (1 << 30 - 1) / 3 * 2. However, the
 * value exceeds std::vector::max_size(). Vector container will throw bad_alloc. 
 * 
 * @param size a fixed dictionary length.
 * @throw invalid_argument() if @param size is out of limits.
 * @throw bad_alloc() if the underlining container allocation is out of memory.
 */
  explicit FixedDict(std::size_t size);
  ~FixedDict();

  std::size_t Size()
  {
    return mysize;
  }

  std::size_t ReservedSize()
  {
    return slots.size();
  }

private:
  FixedDict(const FixedDict &);
  const FixedDict &operator=(const FixedDict &);

private:
  std::vector<std::shared_ptr<TagEntry>> slots;
  std::size_t mysize;
  std::size_t mask;
};

} // namespace goiot

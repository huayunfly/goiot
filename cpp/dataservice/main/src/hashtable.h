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
   * Exception class for AddItem() with key repetition.
   */
  class KeyRepetition : public std::logic_error
  {
  public:
    virtual const char *what() const throw()
    {
      return "container key repeats.";
    }
  };

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

  std::size_t Size() const
  {
    return mysize;
  }

  std::size_t ReservedSize() const
  {
    return slots.size();
  }

  /**
   * Add item.
   * 
   * @param key a new item's key, which can not be empty.
   * @throw invalid_argument key is empty.
   * @throw 
   */
  std::shared_ptr<TagEntry> AddItem(const std::wstring &key);

private:
  /**
   * Lookup the slots by wstring key and hash.
   * If the given key is not found, an empty slot, with empty value 
   * will be returned.
   * 
   * @param key wide string key
   * @param hash key's hash code
   * @ruturn a tag entry with value or empty value
   */
  std::shared_ptr<TagEntry> Lookup(const std::wstring &key, std::size_t hash);

private:
  FixedDict(const FixedDict &);
  const FixedDict &operator=(const FixedDict &);

private:
  std::vector<std::shared_ptr<TagEntry>> slots;
  std::size_t mysize;
  std::size_t mask;
};

} // namespace goiot

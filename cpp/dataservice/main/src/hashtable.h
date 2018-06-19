/**
 * Hash table for the key, value pair storage.
 * 
 * @author Yun Hua
 * @version 0.1 2018.06.07
 */

#include <vector>
#include <memory> // for shared_ptr
#include <string>
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
  const static std::wstring DUMMY_KEY;

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

  std::size_t Used() const
  {
    return used;
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
   * @return new added item, nullptr if the item's name is repeated 
   * or the dictionary is full. 
   */
  std::shared_ptr<TagEntry> AddItem(const std::wstring &key);


  /**
   * Get item by the position.
   * 
   * @param the item position in the dictionary.
   * @throw std::out_of_range, if the positon out of dictionary's ReservedSize.
   * @return an item, nullptr is possible. 
   */
  std::shared_ptr<TagEntry> GetItem(std::size_t pos);

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

  /**
   * Lookup the slots by wstring key and hashcode.
   * If the given key is not found, an empty slot, with empty value 
   * will be returned. If the key is existed, it returns nullptr and sets
   * existed parameter. 
   * 
   * @param key Wide string key
   * @param hash Key's hash code
   * @param existed Return a existed tagentry if a key is existed
   * @throw std::out_of_range
   * @ruturn an empty tag entry if a new tagentry is added, otherwise nullptr
   */
  std::shared_ptr<TagEntry> AddItemRaw(const std::wstring &key, std::size_t hash,
                                       std::shared_ptr<TagEntry> &existed);

  /**
   * Helper function setting the tag entry and increading count.
   * 
   * @param entry the tag entry
   * @param hash the hash code
   * @param name the tag name
   * @param tagid the tag id
   */
  void ResetTagAttrIncCount(std::shared_ptr<TagEntry> entry,
                            const std::wstring &name, std::size_t hash, std::size_t tagid);

private:
  FixedDict(const FixedDict &);
  const FixedDict &operator=(const FixedDict &);

private:
  std::vector<std::shared_ptr<TagEntry>> slots;
  std::size_t mysize;
  std::size_t mask;
  std::size_t used;
};

} // namespace goiot

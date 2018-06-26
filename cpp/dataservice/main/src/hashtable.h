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
  const static std::string DUMMY_KEY;

public:
  /**
   * Exception class for AddItem() with key repetition.
   */
  class KeyRepetition : public std::exception
  {
  public:
    KeyRepetition(const std::string& keyname) : key(keyname)
    {

    }
    virtual const char *what() const throw()
    {
      return std::string("Key: ").append(key).append(" repeated.").c_str();
    }
  
  private:
    std::string key;
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
  std::shared_ptr<TagEntry> AddItem(const std::string &key);


  /**
   * Get item by the position.
   * 
   * @param the item position in the dictionary.
   * @throw std::out_of_range, if the positon out of dictionary's ReservedSize.
   * @return an item, nullptr is possible. 
   */
  std::shared_ptr<TagEntry> GetItem(std::size_t pos);


  /**
   * Search and put the matched entry back.
   * 
   * @param key: the entry key in utf8.
   * @param entry: the mateched key entry.
   * @return true if succeeded, otherwise false.
   */
  bool Find(const std::string& key, std::shared_ptr<TagEntry>& entry) const;


  /**
   * Insert a new key.
   * 
   * @param key: the entry key in utf8.
   * @throw Fixed::KeyRepetition, std::out_of_range 
   * @return the inserted entry.
   */
  std::shared_ptr<TagEntry> Insert(const std::string& key);

  /**
   * Erase an entry at the given position.
   * 
   * @param pos: the entry position;
   * @throw std::out_of_range
   */
  void Erase(std::size_t pos);

private:
  /**
   * Internal search the position of entries by key and hash.
   * A slot id will be given.
   * 
   * @param key: utf8 string key
   * @param hash: key's hash code
   * @ruturn a slot id which can be an empty slot, a repetition (slots full)
   */
  std::size_t InterFind(const std::string &key, std::size_t hash) const;

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
  std::shared_ptr<TagEntry> AddItemRaw(const std::string &key, std::size_t hash,
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
                            const std::string &name, std::size_t hash, std::size_t tagid);

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

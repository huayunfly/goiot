/**
 * Hash table for the key, value pair storage.
 * 
 * The initial probe index is computed as hash mod the table size.
 * Subsequent probing algorithms came from Python dict's C implementation.
 * j = (5*j) + 1 + perturb; 
 * perturb >>= PERTURB_SHIFT; 
 * use j % 2**i as the next table index;
 * Refer to "http://svn.python.org/projects/python/trunk/Objects/dictobject.c"
 *
 * However, Python dict insert manipulation does not return the slot index. The dict is
 * sizable automatically. It is difficult to track a key-value pair's index.
 * In some scenarios, a fixed length hash table is useful, which is non-sizable. Thus,
 * tracking the table slot index is meaningful for we can access the key-value pair via
 * the index.
 * 
 * @author Yun Hua
 * @version 0.1 2018.06.07
 */

#include <stdexcept>
#include <functional>
#include <iostream>
#include <cassert>
#include "hashtable.h"

namespace goiot
{

FixedDict::FixedDict(std::size_t size)
{
    if (size < 1)
    {
        std::cout << "FixedDict::FixedDict() throws: size is less than 1."
                  << std::endl;
        throw std::invalid_argument("Size is less than 1.");
    }
    if (size > DICT_MAXSIZE)
    {
        std::cout << "FixedDict::FixedDict() throws: size is too large." << std::endl;
        throw std::invalid_argument("Size is larger than FixedDict::DICT_MAXSIZE");
    }

    mysize = size;
    std::size_t moresize = size / 2 * 3;
    std::size_t newsize = FixedDict::DICT_MINSIZE;
    for (; newsize < moresize; newsize <<= 1)
        ;
    mask = newsize - 1;
    slots.resize(newsize);
}

FixedDict::~FixedDict()
{
}

 std::shared_ptr<TagEntry> FixedDict::AddItem(const std::wstring& key)
 {
     if (key.empty())
     {
         throw std::invalid_argument("Key is empty.");
     }
     auto entry = Lookup(key, std::hash<std::wstring>()(key));
     if (!entry)
     {
         assert(false);
     }
     return entry;
 }

std::shared_ptr<TagEntry> FixedDict::Lookup(const std::wstring& key, std::size_t hash)
{
    return nullptr;
}

} // namespace goiot

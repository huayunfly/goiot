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

const std::string FixedDict::DUMMY_KEY = u8"Dummy_Key";

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

    used = 0;
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

std::shared_ptr<TagEntry> FixedDict::AddItem(const std::string &key)
{
    if (key.empty())
    {
        throw std::invalid_argument("Key is empty.");
    }
    std::shared_ptr<TagEntry> existed;
    try
    {
        return AddItemRaw(key, std::hash<std::string>()(key), existed);
    }
    catch (const std::out_of_range &e)
    {
        std::cout << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<TagEntry> FixedDict::GetItem(std::size_t pos)
{
    return slots.at(pos);
}

std::size_t FixedDict::InterFind(const std::string &key, std::size_t hash) const
{
    std::size_t i = hash & mask;
    std::size_t j = i;  // start position
    auto ptr = slots.at(i);
    std::shared_ptr<TagEntry> freeslot;

    if (ptr == NULL)
    {
        return i;
    }
    if (key == ptr->attr.name && hash == ptr->attr.hashcode)
    {
        return i;
    }
    if (DUMMY_KEY == ptr->attr.name)
    {
        freeslot = ptr;
    }

    for (std::size_t perturb = hash; ; perturb >>= PERTURB_SHIFT)
    {
        i =  (i << 2) + i + perturb + 1;
        ptr = slots.at(i & mask);
        if (ptr == NULL)
        {
            if (freeslot != NULL)
            {
                return freeslot->tagid;
            }
            else
            {
                return i & mask;
            }
        }
        if (key == ptr->attr.name && hash == ptr->attr.hashcode)
        {
            return i & mask;
        }
        if (DUMMY_KEY == ptr->attr.name && NULL == freeslot)
        {
            freeslot = ptr;
        }
        if (j == (i & mask))
        {
            break;
        }
    }
    return j;   // slots full
}

bool FixedDict::Find(const std::string& key, std::shared_ptr<TagEntry>& entry) const
{
    entry = nullptr;
    std::size_t i = InterFind(key, std::hash<std::string>()(key));
    if (slots.at(i) == NULL || slots.at(i)->attr.name != key)
    {
        return false;
    }
    entry = slots.at(i);
    return true;
}

std::shared_ptr<TagEntry> FixedDict::Insert(const std::string& key)
{
    if (used >= mysize)
    {
        throw std::out_of_range("Dictionary fixed size exceeded.");
    }
    std::size_t hash = std::hash<std::string>()(key);
    std::size_t i = InterFind(key, hash);
    if (slots.at(i) == NULL)
    {
        slots.at(i).reset(new TagEntry());
        slots.at(i)->tagid = i;
        slots.at(i)->attr.name = key;
        slots.at(i)->attr.hashcode = hash;
        used++;
        return slots.at(i);
    }
    if (slots.at(i)->attr.name == key)
    {
        throw KeyRepetition("Key repeated.");
    }
    if (slots.at(i)->attr.name == DUMMY_KEY)
    {
        slots.at(i)->tagid = i;
        slots.at(i)->attr.name = key;
        slots.at(i)->attr.hashcode = hash;
        used++;
        return slots.at(i);  
    }
    // Throws only when it returns to the initial slot.
    throw std::out_of_range("Dictionary is full.");
}

void FixedDict::Erase(std::size_t pos)
{
    if (pos >= slots.size())
    {
        throw std::out_of_range("Erase position is out of range.");
    }
    if (slots.at(pos) && (slots.at(pos)->attr.name != DUMMY_KEY))
    {
        slots.at(pos)->attr.name = DUMMY_KEY;
        slots.at(pos)->attr.hashcode = 0;
        used--;
    }
}

void FixedDict::ResetTagAttrIncCount(std::shared_ptr<TagEntry> entry,
                                     const std::string &name,
                                     std::size_t hash,
                                     std::size_t tagid)
{
    entry->attr.name = name;
    entry->attr.hashcode = hash;
    entry->tagid = tagid;
    used++;
}

std::shared_ptr<TagEntry> FixedDict::AddItemRaw(const std::string &key,
                                                std::size_t hash,
                                                std::shared_ptr<TagEntry> &existed)
{
    existed = nullptr;
    std::size_t i = hash & mask;
    std::shared_ptr<TagEntry> freeslot;
    auto ptr = slots.at(i);

    if (used >= mysize)
    {
        throw std::out_of_range("Dictionary is full.");
    }
    if (ptr == NULL)
    {

        ptr.reset(new TagEntry);
        ResetTagAttrIncCount(ptr, key, hash, i);
        return ptr;
    }
    if (ptr->attr.name == key && ptr->attr.hashcode == hash)
    {
        existed = ptr;
        return nullptr;
    }
    if (ptr->attr.name == DUMMY_KEY)
    {
        freeslot = ptr;
    }

    // No name matched, no free slot, a dummy_key slot, continues.
    for (std::size_t perturb = hash;; perturb >>= PERTURB_SHIFT)
    {
        i = (i << 2) + i + perturb + 1;
        ptr = slots[i & mask];
        if (ptr == NULL)
        {
            if (freeslot)
            {
                ResetTagAttrIncCount(freeslot, key, hash, i & mask);
                return freeslot;
            }
            else
            {
                ptr.reset(new TagEntry);
                ResetTagAttrIncCount(ptr, key, hash, i & mask);
                return ptr;
            }
        }
        if (ptr->attr.name == key && ptr->attr.hashcode == hash)
        {
            existed = ptr;
            return nullptr;
        }
        if (ptr->attr.name == DUMMY_KEY && freeslot == NULL)
        {
            freeslot = ptr;
        }
    }
    assert(0);
    return nullptr;
}

} // namespace goiot

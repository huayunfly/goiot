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

    std::size_t newsize = FixedDict::DICT_MINSIZE;
    for (; newsize < size; newsize <<= 1)
        ;
    if (newsize == 0 || newsize == 1UL << 31)
    {
        throw std::invalid_argument("Size exceeds unsigned long limit.");
    }
    mask = newsize - 1;
    mysize = size;
    std::size_t reserved_size = size / 2 * 3;
    slots.resize((reserved_size < DICT_MINSIZE) ? DICT_MINSIZE : reserved_size);
}


FixedDict::~FixedDict()
{
}

} // namespace goiot

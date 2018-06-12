/**
 * Hash table for the key, value pair storage.
 * The hash() and position probing algorithms came from Python dict's C implementation.
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
#include "hashtable.h"

namespace goiot
{
    const static int PERTURB_SHIFT = 5;
    const static std::size_t DICT_MIN_VALUE = 1;
    const static std::size_t DICT_MAX_VALUE = 1 << 16;

FixedDict::FixedDict(std::size_t size)
{
    if (size < DICT_MIN_VALUE) 
    {
        throw std::invalid_argument("size is less than DICT_MIN_VALUE.");
    }
    if (size > DICT_MAX_VALUE)
    {
        throw std::invalid_argument("size is larger than DICT_MAX_VALUE.");
    }
    std::size_t shift_bits = 0;
    while ((1 << shift_bits) < size)
    {
        shift_bits += 1;
    }
    mask = 1 << (shift_bits + 1) + 1 << shift_bits;
    reserved_size = mask + 1;
}

FixedDict::~FixedDict()
{

}

} // namespace goiot

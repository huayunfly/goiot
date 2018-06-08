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
#include "hashtable.h"

namespace goiot
{
FixedDict::FixedDict(std::size_t size)
{
    if (size < 1) 
    {
        throw std::invalid_argument("size is less than 1.");
    }
    
}

FixedDict::~FixedDict()
{

}

} // namespace goiot

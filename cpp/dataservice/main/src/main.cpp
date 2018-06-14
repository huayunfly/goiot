/**
 * Main entry. Currently it is for Make object.
 * 
 * @author Yun Hua
 * @version 0.1 2018.06.07
 */

#include <iostream>
#include <memory>
#include "hashtable.h"

int main(int argc, char* argv[])
{
    using namespace goiot;
    std::shared_ptr<FixedDict> dict(new FixedDict(16));
    std::cout << "dataserver runs." << std::endl;
    return 0;
}
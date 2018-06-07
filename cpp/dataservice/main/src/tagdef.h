/**
 * Tag definiton and manipulation utility.
 * 
 * @author Yun Hua
 * @version 0.1 2018.06.05
 */

#include <variant>
#include <string>
#include <ctime>

#include "constants.h"

namespace goiot
{

typedef double GO_REAL;
typedef float GO_FLOAT;
typedef int GO_INT;
typedef long GO_LONG;
typedef std::wstring GO_WSTRING;
typedef std::variant<bool, GO_INT, GO_LONG,
                     GO_FLOAT, GO_REAL, GO_WSTRING>
    GO_VARIANT;

typedef std::time_t GO_TIME;
typedef long HRESULT;

struct TagState
{
    TagState()
        : time(std::time(nullptr)), status(GoStatus::S_OK), quality(GoQuality::Q_NORMAL)
    {
    }

    GO_TIME time;
    GoStatus status;
    GoQuality quality;
};

struct TagValue
{
    GO_VARIANT value;
    unsigned int tagid;
    TagState state;
};

struct EUnit
{
    double rangeMax;
    double rangeMin;
    double deadband;
};

struct TagAttr
{
    std::wstring name;
    int hashcode;
    GoPrivilege rights;
    EUnit units;
};

struct TagEntry
{
    unsigned int tagid;
    bool isActive;
    TagValue primValue;
    TagState state;
    TagAttr attr;
};

} // namespace goiot

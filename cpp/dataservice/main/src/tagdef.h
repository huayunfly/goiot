/**
 * Tag definiton and manipulation uitilty
 * 
 * @author Yun Hua
 * @version 0.1 2018.06.05
 */

#include <variant>
#include <string>
#include <ctime>

namespace goiot
{

typedef double GO_TYPE_REAL;
typedef float GO_TYPE_FLOAT;
typedef int GO_TYPE_INT;
typedef long GO_TYPE_LONG;
typedef std::wstring GO_TYPE_WSTRING;

typedef std::time_t GO_TYPE_TIMEPOINT;
typedef long HRESULT;

struct goTagState
{
    GO_TYPE_TIMEPOINT clock;
    HRESULT error;
    int quality;
};

struct goTagValue
{
    std::variant<bool, GO_TYPE_INT, GO_TYPE_LONG,
                 GO_TYPE_FLOAT, GO_TYPE_REAL, GO_TYPE_WSTRING>
        value;
    unsigned int tagid;
    goTagState state;
};

} // namespace goiot

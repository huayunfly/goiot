/**
 * The constants definition.
 * 
 * @author Yun Hua
 * @version 0.1 2018.06.07
 */

namespace goiot
{

enum class GoStatus
{
    S_OK,
    S_ERROR,
    S_INVALID_NAME,
    S_INVALID_ID,
    S_INVALID_PROVIDER,
    S_NO_IMPLEMENTED,
    S_COMM_ERROR,
    S_PORT_ERROR,
    S_UNKNOWN
};

enum class GoOperation
{
    OP_READ,
    OP_WRITE,
    OP_ASYNC_READ,
    OP_ASYNC_WRITE,
    OP_REFRESH
};

enum class GoPrivilege
{
    READABLE,
    WRITABLE,
    READWRITABLE
};

enum class GoTransactionId
{
    ID_EMPTY = -1

};

enum class GoQuality
{
    Q_NORMAL = 1
};

} // namespace goiot
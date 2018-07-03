/**
 * Circular buffer implementation.
 * 
 */

#include "buffer.h"

// namespace goiot
// {
// template <typename T>
// int CircularBuffer<T>::GetItem(T &item)
// {
//     std::error_code error;
//     std::unique_lock<std::mutex> lk(bufferlock); // It may throws system_error.
//     while ((totalitems <= 0) && !error && !doneflag)
//     {
//         items.wait(lk, error);
//     }
//     if (error)
//     {
//         // return error.value();
//         throw std::system_error(error);
//     }
//     if (doneflag && (totalitems <= 0))
//     {
//         // return ECANCELED;
//         throw std::system_error(std::errc::operation_canceled);
//     }
//     item = buffer.at(outpos);
//     outpos = (outpos + 1) % mysize;
//     totalitems--;
//     slots.notify_one();
//     return 0;
// }

// template <typename T>
// int CircularBuffer<T>::PutItem(const T &item)
// {
//     std::error_code error;
//     std::unique_lock<std::mutex> lk(bufferlock); // It may throws system_error.
//     while (totalitems >= mysize && !error && !doneflag)
//     {
//         slots.wait(lk, error);
//     }
//     if (error)
//     {
//         throw std::system_error(error);
//     }
//     if (doneflag)
//     {
//         throw std::system_error(std::errc::operation_canceled);
//     }
//     buffer.at(inpos) = item;
//     inpos = (inpos + 1) % mysize;
//     totalitems++;
//     items.notify_one();
//     return 0;
// }

// template <typename T>
// int CircularBuffer<T>::GetDone(int &flag) const
// {
//     std::lock_guard<std::mutex> lk(bufferlock); // It may throws system_error.
//     flag = doneflag;
//     return 0;
// }

// template <typename T>
// int CircularBuffer<T>::SetDone(void)
// {
//     std::lock_guard<std::mutex> lk(bufferlock); // It may throws system_error.
//     bufferlock.lock();
//     doneflag = 1;
//     items.notify_all();
//     slots.notify_all();
//     return 0;
// }
// } // namespace goiot
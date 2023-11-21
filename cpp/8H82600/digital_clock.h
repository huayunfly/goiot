#ifndef DIGITALCLOCK_H
#define DIGITALCLOCK_H

#include <QLCDNumber>
#include <QDateTime>
#include <atomic>

class DigitalClock : public QLCDNumber
{

public:
    DigitalClock(QWidget* parent = nullptr, bool nowtime = true);

    void SetStartTime(double start)
    {
        _start.store(start);
    }

    void ClearDisplay(bool yes)
    {
        _clear.store(yes);
    }

public slots:
    void show_time();
    bool _nowtime; // Yes: nowtime mode, No: interval from the start point
    std::atomic<double> _start; // Start time in seconds
    std::atomic<bool> _clear; // Clear display to 00:00:00

};

#endif // DIGITALCLOCK_H

#include "digital_clock.h"
#include <QTimer>


DigitalClock::DigitalClock(QWidget* parent, bool nowtime) :
    QLCDNumber(parent), _nowtime(nowtime), _start(0.0), _clear(false)
{
    setSegmentStyle(Flat);
    if (_nowtime)
    {
        setDigitCount(8); // hh:mm:ss
    }
    else
    {
        setDigitCount(9); // hhh:mm:ss
    }

    QTimer * timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &DigitalClock::show_time);
    timer->start(1000);

    this->setStyleSheet("QLCDNumber {background-color:green;color:black}");

    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    resize(150, 60);
}

void DigitalClock::show_time()
{
    if (_clear.load())
    {
        display(QString("00:00:00"));
    }
    else
    {
        if (_nowtime)
        {
            QTime time = QTime::currentTime();
            QString text = time.toString("hh:mm:ss");
            if ((time.second() % 2) == 0)
            {
                text[5] = ' ';
            }
            display(text);
        }
        else
        {
            double interval_seconds = QDateTime::currentMSecsSinceEpoch() / 1000 - _start.load();
            int hours = static_cast<int>(interval_seconds / 3600);
            int minutes = static_cast<int>((interval_seconds - hours * 3600) / 60);
            int seconds = static_cast<int>(interval_seconds - hours * 3600 - minutes * 60);
            QString text = QString("%1:%2:%3").arg(hours).arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0'));
            display(text);
        }
    }
}

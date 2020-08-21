#ifndef EVENTS_H
#define EVENTS_H

#include <qevent.h>

namespace Ui
{

enum class ControlStatus
{
    OK = 0,
    FAILURE = 1
};

class RefreshEvent : public QEvent
{
public:
    static const QEvent::Type myType = static_cast<QEvent::Type>(QEvent::User + 1000);

    explicit RefreshEvent(const QString& name, const QString& text, ControlStatus status) :
        QEvent (myType), name_(name), text_(text), status_(status)
    {

    }

    // Properties
    QString Name()
    {
        return name_;
    }

    QString Text()
    {
        return text_;
    }

    ControlStatus Status()
    {
        return status_;
    }

    friend QWidget;

private:
    QString name_;
    QString text_;
    ControlStatus status_;
};

}


#endif // EVENTS_H

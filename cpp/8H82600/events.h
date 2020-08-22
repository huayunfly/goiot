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
    explicit RefreshEvent(QEvent::Type type, const QString& name, ControlStatus status) :
        QEvent (type), name_(name), status_(status)
    {

    }

    virtual ~RefreshEvent()
    {
    }

    // Properties
    QString Name()
    {
        return name_;
    }

    ControlStatus Status()
    {
        return status_;
    }

private:
    QString name_;
    ControlStatus status_;
};

class RefreshTextEvent : public RefreshEvent
{
public:
    static const QEvent::Type myType = static_cast<QEvent::Type>(QEvent::User + 1001);

    explicit RefreshTextEvent(const QString& name, ControlStatus status, const QString& text) :
        RefreshEvent (myType, name, status), text_(text)
    {

    }

    QString Text()
    {
        return text_;
    }

private:
    QString text_;
};

// Refresh state including the device position
class RefreshStateEvent : public RefreshEvent
{
public:
    static const QEvent::Type myType = static_cast<QEvent::Type>(QEvent::User + 1002);

    explicit RefreshStateEvent(const QString& name, ControlStatus status, int state) :
        RefreshEvent (myType, name, status), state_(state)
    {

    }

    int State()
    {
        return state_;
    }

private:
    int state_;
};

}

#endif // EVENTS_H

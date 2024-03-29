#ifndef EVENTS_H
#define EVENTS_H

#include <qevent.h>
#include "data_model.h"

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
    explicit RefreshEvent(QEvent::Type type, const QString& name, ControlStatus status,
                          const UiInfo& ui_info, const std::string& data_info_id, double timestamp) :
        QEvent (type), name_(name), status_(status), ui_info_(ui_info), data_info_id_(data_info_id), timestamp_(timestamp)
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

    const UiInfo& GetUiInfo() const
    {
        return ui_info_;
    }

    const std::string& GetDataInfoId() const
    {
        return data_info_id_;
    }

    double GetTimestamp() const
    {
        return timestamp_;
    }

private:
    QString name_;
    ControlStatus status_;
    UiInfo ui_info_;
    std::string data_info_id_;
    double timestamp_;
};

class RefreshTextEvent : public RefreshEvent
{
public:
    static const QEvent::Type myType = static_cast<QEvent::Type>(QEvent::User + 1001);

    explicit RefreshTextEvent(const QString& name, ControlStatus status, const UiInfo& ui_info,
                              const std::string& data_info_id, double timestamp, const QString& text) :
        RefreshEvent (myType, name, status, ui_info, data_info_id, timestamp), text_(text)
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

    explicit RefreshStateEvent(const QString& name, ControlStatus status, const UiInfo& ui_info,
                               const std::string& data_info_id, double timestamp, int state) :
        RefreshEvent (myType, name, status, ui_info, data_info_id, timestamp), state_(state)
    {

    }

    int State()
    {
        return state_;
    }

private:
    int state_;
};

// Refresh process value for alarm/normal state only.
class ProcessValueEvent : public RefreshEvent
{
public:
    static const QEvent::Type myType = static_cast<QEvent::Type>(QEvent::User + 1003);

    explicit ProcessValueEvent(const QString& name, ControlStatus status, const UiInfo& ui_info,
                               const std::string& data_info_id, double timestamp) :
        RefreshEvent (myType, name, status, ui_info, data_info_id, timestamp)
    {

    }
};


}

#endif // EVENTS_H

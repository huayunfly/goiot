#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLabel>
#include "events.h"
#include "form_common.h"

FormCommon::FormCommon(QWidget *parent) : QWidget(parent)
{

}

bool FormCommon::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    if (event->type() == Ui::RefreshTextEvent::myType)
    {
        Ui::RefreshTextEvent* e = static_cast<Ui::RefreshTextEvent*>(event);
        auto text_edit = this->findChild<QTextEdit*>(e->Name());
        if (text_edit != nullptr)
        {
            if (e->Status() == Ui::ControlStatus::OK)
            {
                text_edit->setText(e->Text());
            }
            else
            {
                ; // todo
            }
        }
        return true;
    }
    else if (event->type() == Ui::RefreshStateEvent::myType)
    {
        Ui::RefreshStateEvent* e = static_cast<Ui::RefreshStateEvent*>(event);
        auto label = this->findChild<QLabel*>(e->Name());
        auto state = GetUiState(e->Name());
        if (label != nullptr && !state.normal_pixmap.isEmpty())
        {
            if (e->Status() == Ui::ControlStatus::OK)
            {
                if (state.device_type == VDeviceType::ONOFF)
                {
                    QPixmap pixmap;
                    e->State() == 0 ? pixmap.load(state.normal_pixmap) : pixmap.load(state.active_pixmap);
                    label->setPixmap(pixmap);
                }
                else if (state.device_type == VDeviceType::SVALVE)
                {

                }
            }
            else
            {
                label->setPixmap(QPixmap(state.error_pixmap)); // error status
            }
        }
        return true;
    }

    return QWidget::event(event);
}

UiStateDef FormCommon::GetUiState(const QString& ui_name)
{
    const auto iter = ui_state_map_.find(ui_name);
    if (iter != ui_state_map_.cend())
    {
        return iter->second;
    }
    return UiStateDef();
}

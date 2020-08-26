#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLabel>
#include "form_common.h"
#include "dialog_setvalue.h"
#include "dialog_setposition.h"
#include "dialog_onoff.h"

FormCommon::FormCommon(QWidget *parent) : QWidget(parent)
{

}

bool FormCommon::eventFilter(QObject* object, QEvent* event)
{
    if (object->inherits("QLabel"))
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if(mouseEvent->button() == Qt::LeftButton)
            {
                UiSetValue(static_cast<QWidget*>(object));
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return QWidget::eventFilter(object, event);
    }
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

void FormCommon::UiSetValue(QWidget* sender)
{
    if (sender == nullptr)
    {
        assert(false);
        return;
    }

    auto state = GetUiState(sender->objectName());
    if (state.device_type == VDeviceType::EMPTY)
    {
        assert(false);
        return;
    }

    QString value;
    Ui::ControlStatus status;
    bool ok = read_data_func_(this->objectName(), sender->objectName(), value, status);
    if (!ok)
    {
        assert(false);
        return;
    }
    if (status != Ui::ControlStatus::OK)
    {
        return; // todo
    }

    // convert the widget position to the screen position.
    QPoint screen_pos = this->mapToGlobal(sender->pos());
    screen_pos.setX(screen_pos.x() + 25);
    screen_pos.setY(screen_pos.y() + 10);

    if (state.device_type == VDeviceType::PROCESS_FLOAT)
    {
         DialogSetValue set_value_dialog(sender, value, state.measure_unit, state.high_limit, state.low_limit);
         set_value_dialog.move(screen_pos);
         int result = set_value_dialog.exec();
         if (result == QDialog::Accepted)
         {
             float f = set_value_dialog.NewValue();
             ok = write_data_func_(this->objectName(), sender->objectName(), QString::number(f, 'f', 2));
         }
    }
    else if (state.device_type == VDeviceType::ONOFF)
    {
        DialogOnOff set_onoff_dialog(sender, value.toInt());
        set_onoff_dialog.move(screen_pos);
        int result = set_onoff_dialog.exec();
        if (result == QDialog::Accepted)
        {
            int pos = set_onoff_dialog.NewValue();
            ok = write_data_func_(this->objectName(), sender->objectName(), QString::number(pos, 10));
        }
    }
    else if (state.device_type == VDeviceType::SVALVE)
    {
        DialogSetPosition set_position_dialog(sender, state.high_limit, value.toInt());
        set_position_dialog.move(screen_pos);
        int result = set_position_dialog.exec();
        if (result == QDialog::Accepted)
        {
            int pos = set_position_dialog.NewValue();
            ok = write_data_func_(this->objectName(), sender->objectName(), QString::number(pos, 10));
        }
    }
}

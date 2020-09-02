#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLabel>
#include "form_common.h"
#include "dialog_setvalue.h"
#include "dialog_setposition.h"
#include "dialog_onoff.h"
#include <QResource>

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
        bool ok = false;
        auto state = GetUiState(e->Name(), ok);
        if (label != nullptr && ok)
        {
            if (e->Status() == Ui::ControlStatus::OK)
            {
                if (state.device_type == VDeviceType::ONOFF)
                {
                    QPixmap pixmap;
                    if (e->State() == 0)
                    {
                        pixmap = GetImageCache(state.normal_pixmap);
                    }
                    else
                    {
                        pixmap = GetImageCache(state.active_pixmap);
                    }
                    label->setPixmap(pixmap);
                }
                else if (state.device_type == VDeviceType::MULTI_STATE)
                {
                    QPixmap pixmap = GetImageCache(state.normal_pixmap);
                    int w = pixmap.size().width() / state.positions_for_normal_pixmap;
                    int h = pixmap.size().height();
                    assert(w > 0 && h > 0 && state.positions_for_normal_pixmap > 0 && e->State() < state.positions_for_normal_pixmap);
                    if (w > 0 && h > 0 && state.positions_for_normal_pixmap > 0 && e->State() < state.positions_for_normal_pixmap)
                    {
                        label->setPixmap(pixmap.copy(w * e->State(), // valve position starts from 1
                                                     0,
                                                     w,
                                                     h));
                    }
                }
            }
            else
            {
                if (state.device_type == VDeviceType::MULTI_STATE)
                {
                    QPixmap pixmap = GetImageCache(state.normal_pixmap);
                    int w = pixmap.size().width() / state.positions_for_normal_pixmap;
                    int h = pixmap.size().height();
                    assert(w > 0 && h > 0 && state.positions_for_normal_pixmap > 0 && e->State() < state.positions_for_normal_pixmap);
                    if (w > 0 && h > 0 && state.positions_for_normal_pixmap > 0 && e->State() < state.positions_for_normal_pixmap)
                    {
                        label->setPixmap(pixmap.copy(0, // valve position 0 indicated abnormal state
                                                     0,
                                                     w,
                                                     h));
                    }
                }
                else
                {
                    label->setPixmap(QPixmap(state.error_pixmap)); // error status
                }
            }
        }
        return true;
    }

    return QWidget::event(event);
}

UiStateDef FormCommon::GetUiState(const QString& ui_name, bool& ok)
{
    ok = false;
    const auto iter = ui_state_map_.find(ui_name);
    if (iter != ui_state_map_.cend())
    {
        ok = true;
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

    bool ok = false;
    auto state = GetUiState(sender->objectName(), ok);
    if (!ok)
    {
        assert(false);
        return;
    }

    QString value;
    Ui::ControlStatus status;
    ok = read_data_func_(this->objectName(), sender->objectName(), value, status);
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
    else if (state.device_type == VDeviceType::MULTI_STATE)
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

QPixmap FormCommon::GetImageCache(const QString& image_path)
{
    const auto iter = image_cache_.find(image_path);
    if (iter != image_cache_.end())
    {
        return iter->second;
    }
    // About unordered_map.emplace()
    // If the insertion takes place (because no other element existed with the same key), the function returns a pair object, whose first component is an iterator to the inserted element, and whose second component is true.
    // Otherwise, the pair object returned has as first component an iterator pointing to the element in the container with the same key, and false as its second component.
    return image_cache_.emplace(image_path, QPixmap(image_path)).first->first;
}

#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLabel>
#include "form_common.h"
#include "dialog_setvalue.h"
#include "dialog_setposition.h"
#include "dialog_onoff.h"
#include <QResource>

FormCommon::FormCommon(QWidget *parent, const QString& object_name,
                       const QString& display_name) :
    QWidget(parent), display_name_(display_name)
{
    this->setObjectName(object_name);
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
                QString unit;
                switch (e->GetUiInfo().unit)
                {
                case MeasurementUnit::ML:
                    unit = "ml";
                    break;
                case MeasurementUnit::BARA:
                    unit = "bara";
                    break;
                case MeasurementUnit::BARG:
                    unit = "barg";
                    break;
                case MeasurementUnit::SCCM:
                    unit = "sccm";
                    break;
                case MeasurementUnit::DEGREE:
                    unit = u8"°C";
                    break;
                case MeasurementUnit::MM:
                    unit = "mm";
                    break;
                case MeasurementUnit::MLM:
                    unit = "mlm";
                    break;
                case MeasurementUnit::MPA:
                    unit = "mpa";
                    break;
                case MeasurementUnit::RPM:
                    unit= "rpm";
                    break;
                case MeasurementUnit::LEL:
                    unit= "lel";
                    break;
                case MeasurementUnit::PPM:
                    unit = "ppm";
                    break;
                case MeasurementUnit::G:
                    unit = "g";
                    break;
                case MeasurementUnit::KG:
                    unit = "kg";
                    break;
                default:
                    break;
                }
                text_edit->setText(e->Text() + unit);
            }
            else
            {
                ; // todo
            }
        }
        return true;
    }
    else if (event->type() == Ui::ProcessValueEvent::myType)
    {
        Ui::ProcessValueEvent* e = static_cast<Ui::ProcessValueEvent*>(event);
        auto label = this->findChild<QLabel*>(e->Name());
        auto ui_info = e->GetUiInfo();
        if (label != nullptr)
        {
            QPixmap pixmap = GetImageCache(ui_info.pixmap_path);
            int w = pixmap.size().width() / 2; // alarm, normal
            int h = pixmap.size().height();
            assert(w > 0 && h > 0);
            if (e->Status() == Ui::ControlStatus::OK)
            {
                label->setPixmap(pixmap.copy(w, // jump alarm position
                                             0,
                                             w,
                                             h));
            }
            else
            {
                label->setPixmap(pixmap.copy(0, 0, w, h));
            }
        }
    }
    else if (event->type() == Ui::RefreshStateEvent::myType)
    {
        Ui::RefreshStateEvent* e = static_cast<Ui::RefreshStateEvent*>(event);
        auto label = this->findChild<QLabel*>(e->Name());
        auto ui_info = e->GetUiInfo();
        if (label != nullptr)
        {
            if (ui_info.type == WidgetType::ONOFF)
            {
                QPixmap pixmap = GetImageCache(ui_info.pixmap_path);
                int w = pixmap.size().width() / 3; // alarm, normal and active states
                int h = pixmap.size().height();
                assert(w > 0 && h > 0 && e->State() <= ui_info.high_limit);
                if (e->Status() == Ui::ControlStatus::OK)
                {
                    if (w > 0 && h > 0 && e->State() <= ui_info.high_limit)
                    {
                        if (e->State() == 0)
                        {
                            label->setPixmap(pixmap.copy(w, // jump alarm position
                                                         0,
                                                         w,
                                                         h));
                        }
                        else
                        {
                            label->setPixmap(pixmap.copy(w * 2, // jump alarm and normal position
                                                         0,
                                                         w,
                                                         h));
                        }
                    }
                }
                else
                {
                    label->setPixmap(pixmap.copy(0, 0, w, h));
                }
            }
            else if (ui_info.type == WidgetType::STATE)
            {
                QPixmap pixmap = GetImageCache(ui_info.pixmap_path);
                int w = pixmap.size().width() / (ui_info.high_limit + 1/* alarm position */);
                int h = pixmap.size().height();
                assert(w > 0 && h > 0 && e->State() <= ui_info.high_limit);
                if (e->Status() == Ui::ControlStatus::OK)
                {
                    if (w > 0 && h > 0 && e->State() <= ui_info.high_limit)
                    {
                        label->setPixmap(pixmap.copy(w * e->State(), // valve position starts from 1
                                                     0,
                                                     w,
                                                     h));
                    }
                }
                else
                {
                    label->setPixmap(pixmap.copy(0, 0, w, h));
                }
            }

        }
        return true;
    }

    return QWidget::event(event);
}

//UiStateDef FormCommon::GetUiState(const QString& ui_name, bool& ok)
//{
//    ok = false;
//    const auto iter = ui_state_map_.find(ui_name);
//    if (iter != ui_state_map_.cend())
//    {
//        ok = true;
//        return iter->second;
//    }
//    return UiStateDef();
//}

void FormCommon::UiSetValue(QWidget* sender)
{
    if (sender == nullptr)
    {
        assert(false);
        return;
    }

    bool ok = false;

    QString value;
    Ui::ControlStatus status;
    UiInfo ui_info;
    ok = read_data_func_(this->objectName(), sender->objectName(), value, status, ui_info);
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

    if (ui_info.type == WidgetType::PROCESS_VALUE)
    {
         DialogSetValue set_value_dialog(sender, value, ui_info.unit, ui_info.high_limit, ui_info.low_limit);
         set_value_dialog.move(screen_pos);
         int result = set_value_dialog.exec();
         if (result == QDialog::Accepted)
         {
             float f = set_value_dialog.NewValue();
             ok = write_data_func_(this->objectName(), sender->objectName(), QString::number(f, 'f', 2));
         }
    }
    else if (ui_info.type == WidgetType::ONOFF)
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
    else if (ui_info.type == WidgetType::STATE)
    {
        DialogSetPosition set_position_dialog(sender, ui_info.high_limit, value.toInt());
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

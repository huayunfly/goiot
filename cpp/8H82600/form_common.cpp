#include <QtWidgets/QTextEdit>
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
            text_edit->setText(e->Text());
            return true;
        }
    }
    else if (event->type() == Ui::RefreshStateEvent::myType)
    {
        Ui::RefreshStateEvent* e = static_cast<Ui::RefreshStateEvent*>(event);
        auto state_edit = this->findChild<QWidget*>(e->Name());
        if (state_edit != nullptr)
        {
            return true;
        }
    }

    return QWidget::event(event);
}

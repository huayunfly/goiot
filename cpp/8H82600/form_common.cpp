#include <QtWidgets/QTextEdit>
#include "events.h"
#include "form_common.h"

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
        }
    }
    return QWidget::event(event);
}

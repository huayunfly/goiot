#include <qwindowdefs_win.h>
#include "form_gasfeed.h"
#include "ui_form_gasfeed.h"
#include "events.h"

FormGasFeed::FormGasFeed(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormGasFeed)
{
    ui->setupUi(this);
}

FormGasFeed::~FormGasFeed()
{
    delete ui;
}

bool FormGasFeed::nativeEvent(const QByteArray &eventType, void *message, long *result)
{

    if (eventType == "windows_generic_MSG") //windows platform
    {
//        MSG* msg = reinterpret_cast<MSG*>(message);

//        if(msg->message == WM_COPYDATA) // message type
//        {
//            auto wnd = reinterpret_cast<HWND>(msg->wParam);//高地址的参数
//        }
    }

    return QWidget::nativeEvent(eventType, message, result);
}

bool FormGasFeed::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    if (event->type() == Ui::RefreshEvent::myType)
    {
        Ui::RefreshEvent* e = static_cast<Ui::RefreshEvent*>(event);
        auto text_edit = this->findChild<QTextEdit*>(e->Name());
        if (text_edit != nullptr)
        {
            text_edit->setText(e->Text());
        }
    }
    return QWidget::event(event);
}

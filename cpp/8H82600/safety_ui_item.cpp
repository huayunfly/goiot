#include "safety_ui_item.h"
#include <QPainter>

SafetyUIItem::SafetyUIItem(double radius, const QString& pid_code,
             SafetyUIItemStatus status) :
    radius_(radius), pid_code_(pid_code), status_(status)
{

}

void SafetyUIItem::paint(QPainter *painter,
           const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QString label;
    switch (status_)
    {
    case SafetyUIItemStatus::Normal:
        painter->setBrush(Qt::green);
        label = QString();
        break;
    case SafetyUIItemStatus::HLimit:
        painter->setBrush(Qt::yellow);
        label = QString("超限");
        break;
    case SafetyUIItemStatus::HHLimit:
        painter->setBrush(Qt::red);
        label = QString("高限");
        break;
    case SafetyUIItemStatus::LLimit:
        painter->setBrush(Qt::yellow);
        label = QString("低限");
        break;
    case SafetyUIItemStatus::TBreak:
        painter->setBrush(Qt::red);
        label = QString("断线");
        break;
    default:
        painter->setBrush(Qt::red);
    }

    painter->drawEllipse(QPointF(0, 0), radius_, radius_);
    // position
    QFont font;
    font.setPointSize(7);
    painter->setFont(font);
    painter->drawText(QPointF(10, -10), pid_code_);

    // channel
    font.setPointSize(10);
    painter->setFont(font);
    painter->drawText(QPointF(-10, 10), label);
}

void SafetyUIItem::SetStatus(SafetyUIItemStatus status)
{
    status_ = status;
    update();
}


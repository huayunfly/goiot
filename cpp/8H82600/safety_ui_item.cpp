#include "safety_ui_item.h"
#include <QPainter>
#include <QToolTip>
#include <QGraphicsSceneHoverEvent>

SafetyUIItem::SafetyUIItem(double radius, const QString& pid_code,
                           const QString& note, SafetyUIItemStatus status) :
    radius_(radius), pid_code_(pid_code), note_(note), status_(status)
{
    setAcceptHoverEvents(true); // Active hover
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
        painter->setBrush(Qt::darkGreen);
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
    case SafetyUIItemStatus::Inactive:
        painter->setBrush(Qt::gray);
        label = QString("N/A");
        break;
    case SafetyUIItemStatus::Failure:
        painter->setBrush(Qt::red);
        label = QString("故障");
        break;
    case SafetyUIItemStatus::Trigger:
        painter->setBrush(Qt::red);
        label = QString("触发");
        break;
    default:
        painter->setBrush(Qt::red);
    }

    painter->drawEllipse(QPointF(0, 0), radius_, radius_);
    // code
    QFont font;
    font.setPointSize(7);
    painter->setFont(font);
    painter->drawText(QPointF(0, -10), pid_code_);

    // channel
    font.setPointSize(10);
    painter->setFont(font);
    painter->drawText(QPointF(-10, 10), label);
}

void SafetyUIItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QToolTip::showText(event->screenPos(), note_);
    // How to make persistent tooltip:
    // label->move(event->screenPos());
    // label->setText("Tooltip that follows the mouse");
    // if(label->isHidden()) label->show();
}

void SafetyUIItem::SetStatus(SafetyUIItemStatus status)
{
    status_ = status;
    switch (status_)
    {
    case SafetyUIItemStatus::Normal:
        status_note_ = "正常";
        break;
    case SafetyUIItemStatus::HLimit:
        status_note_ = QString("超限");
        break;
    case SafetyUIItemStatus::HHLimit:
        status_note_ = QString("高限");
        break;
    case SafetyUIItemStatus::LLimit:
        status_note_ = QString("低限");
        break;
    case SafetyUIItemStatus::TBreak:
        status_note_ = QString("测温断线");
        break;
    case SafetyUIItemStatus::Inactive:
        status_note_ = QString("未激活");
        break;
    case SafetyUIItemStatus::Failure:
        status_note_ = QString("故障");
        break;
    case SafetyUIItemStatus::Trigger:
        status_note_ = QString("触发");
        break;
    default:
        status_note_ = QString("Unknown");
    }
    update();
}


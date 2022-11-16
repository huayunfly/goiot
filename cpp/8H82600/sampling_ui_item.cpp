#include "sampling_ui_item.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <math.h>

void SamplingUIItem::paint(QPainter *painter,
           const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    switch (status_)
    {
    case SamplingUIItemStatus::Unsigned:
        painter->setBrush(Qt::transparent);
        break;
    case SamplingUIItemStatus::Waiting:
        painter->setBrush(Qt::lightGray);
        break;
    case SamplingUIItemStatus::Sampling:
        painter->setBrush(Qt::yellow);
        break;
    case SamplingUIItemStatus::Finished:
        painter->setBrush(Qt::green);
        break;
    case SamplingUIItemStatus::Discharge:
        painter->setBrush(Qt::blue);
        break;
    case SamplingUIItemStatus::Undischarge:
        painter->setBrush(Qt::white);
        break;
    case SamplingUIItemStatus::Error:
        painter->setBrush(Qt::red);
        break;
    default:
        throw std::invalid_argument("undefined sampling item status.");
    }

    int width = (type_ == SamplingUIItemType::Injection) ? 2 : 1;
    QPen pen(Qt::black);
    pen.setWidth(width);
    painter->setPen(pen);
    painter->drawEllipse(QPointF(0, 0), radius_, radius_);
    // position
    QFont font;
    font.setPointSize(7);
    painter->setFont(font);
    if (type_ == SamplingUIItemType::Home)
    {
        painter->drawText(QPointF(10, -10), "原点");
    }
    else if (type_ == SamplingUIItemType::Injection)
    {
        painter->drawText(QPointF(10, -10), QString::number(number_));
    }
    else if (type_ == SamplingUIItemType::Purge)
    {
        painter->drawText(QPointF(10, -10), "吹扫");
    }
    else if (type_ == SamplingUIItemType::Clean)
    {
        painter->drawText(QPointF(10, -10), "清洗");
    }
    // channel
    font.setPointSize(10);
    painter->setFont(font);
    if (channel_ > 0 && channel_ <= 16)
    {
        painter->drawText(QPointF(-10, 10), QString::number(channel_));
    }
    else
    {
        painter->drawText(QPointF(-10, 10), "");
    }
}

void SamplingUIItem::SetStatus(SamplingUIItemStatus status, int channel)
{
    status_ = status;
    channel_ = channel;
    update();
}

void SamplingUIItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void SamplingUIItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    // TEST code.
    //status_ = SamplingUIItemStatus::Finished;
    //update();
    QGraphicsItem::mouseDoubleClickEvent(event);
}


void SamplingUIItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
}

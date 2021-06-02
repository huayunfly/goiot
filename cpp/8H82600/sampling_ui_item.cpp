#include "sampling_ui_item.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <math.h>

void SamplingUIItem::paint(QPainter *painter,
           const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    widget = nullptr;

    switch (status_)
    {
    case SamplingUIItemStatus::Unsigned:
        painter->setBrush(Qt::gray);
        break;
    case SamplingUIItemStatus::Waiting:
        painter->setBrush(Qt::darkRed);
        break;
    case SamplingUIItemStatus::Sampling:
        painter->setBrush(Qt::yellow);
        break;
    case SamplingUIItemStatus::Finished:
        painter->setBrush(Qt::green);
        break;
    case SamplingUIItemStatus::Discharge:
        painter->setBrush(Qt::black);
        break;
    case SamplingUIItemStatus::Undischarge:
        painter->setBrush(Qt::white);
        break;
    default:
        throw std::invalid_argument("undefined sampling item status.");
    }

    painter->drawEllipse(QPointF(0, 0), radius_, radius_);
    if (number_ > 0 && number_ <= MAX_SAMPLING)
    {
        painter->drawText(QPointF(0, 0), QString::number(number_));
    }
    else if (number_ > MAX_SAMPLING)
    {
        painter->drawText(QPointF(0, 0), "sink");
    }

}

void SamplingUIItem::SetStatus(SamplingUIItemStatus status)
{
    status_ = status;
    update();
}

void SamplingUIItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void SamplingUIItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    status_ = SamplingUIItemStatus::Finished;
    QGraphicsItem::mouseDoubleClickEvent(event);
    update();
}


void SamplingUIItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
}

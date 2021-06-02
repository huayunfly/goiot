// @purpose: Qt graphic item for liquid sampling display.
// @author: huayunfly@126.com
// @date: 2021.05.28
// @copyright: GNU
// @version: 0.1

#ifndef SAMPLINGUIITEM_H
#define SAMPLINGUIITEM_H

#include <QGraphicsItem>

class SamplingUIItem : public QGraphicsItem
{
public:
    enum class SamplingUIItemStatus
    {
        Unsigned,
        Waiting,
        Sampling,
        Finished,
        Discharge,
        Undischarge
    };

public:
    SamplingUIItem(double radius, int number = 0,
                   SamplingUIItemStatus status = SamplingUIItemStatus::Unsigned) :
        radius_(radius), number_(number), status_(status)
    {
        setFlags(QGraphicsItem::ItemIsSelectable
                 | QGraphicsItem::ItemIsFocusable);
    }

    ~SamplingUIItem()
    {

    }

    SamplingUIItem(const SamplingUIItem&) = delete;
    SamplingUIItem& operator=(const SamplingUIItem&) = delete;

    QRectF boundingRect() const override
    {
        return QRectF(-radius_, -radius_, 2 * radius_, 2 * radius_);
    }

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void SetStatus(SamplingUIItemStatus status);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent (QGraphicsSceneMouseEvent *event) override;

private:
    const int MAX_SAMPLING = 128;
    double radius_;
    int number_;
    SamplingUIItemStatus status_; // 0: gray unsigned, 1: red waiting, 2: yellow sampling, 3: green finished.
};

#endif // SAMPLINGUIITEM_H

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
        Undischarge,
        Error
    };

    enum class SamplingUIItemType
    {
        Injection,
        Purge,
        Clean,
        Home
    };

    enum class Layout
    {
        I = 0, // Inject
        P = 1, // Purge
        C = 2, // Clean
        H = 3 // Home
    };

public:
    SamplingUIItem(double radius, int number = 0, int channel = 0,
                   SamplingUIItemType type = SamplingUIItemType::Injection,
                   SamplingUIItemStatus status = SamplingUIItemStatus::Unsigned) :
        radius_(radius), number_(number), channel_(channel), type_(type), status_(status)
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

    void SetStatus(SamplingUIItemStatus status, int channel);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent (QGraphicsSceneMouseEvent *event) override;

private:
    const int MAX_SAMPLING = 64;
    const int PURGE_POSITION_LOWER = 65; // N2 purge number range lower
    const int PURGE_POSITION_UPPER = 96; // N2 purge number range upper
    double radius_;
    int number_;
    int channel_;
    SamplingUIItemType type_;
    SamplingUIItemStatus status_; // 0: transparent unsigned, 1: grey waiting, 2: yellow sampling, 3: green finished.
};

#endif // SAMPLINGUIITEM_H

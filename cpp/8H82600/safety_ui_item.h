// @purpose: Qt graphic item for safety display.
// @author: huayunfly at 126.com
// @date: 2022.09.21
// @version: 1.0

#ifndef SAFETYUIITEM_H
#define SAFETYUIITEM_H

#include <QGraphicsItem>


class SafetyUIItem : public QGraphicsItem
{
public:
    enum class SafetyUIItemStatus
    {
        Normal,
        HLimit,
        HHLimit,
        LLimit,
        TBreak,
    };

public:
    SafetyUIItem(double radius, const QString& pid_code,
                 SafetyUIItemStatus status = SafetyUIItemStatus::Normal);

    SafetyUIItem(const SafetyUIItem&) = delete;
    SafetyUIItem& operator=(const SafetyUIItem&) = delete;

    ~SafetyUIItem()
    {
    }

    QRectF boundingRect() const override
    {
        return QRectF(-radius_, -radius_, 2 * radius_, 2 * radius_);
    }

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void SetStatus(SafetyUIItemStatus status);

private:
    double radius_;
    QString pid_code_;
    SafetyUIItemStatus status_;
};

#endif // SAFETYUIITEM_H

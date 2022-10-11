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
        Inactive
    };

public:
    SafetyUIItem(double radius, const QString& pid_code,
                 const QString& note = QString(),
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

    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void SetStatus(SafetyUIItemStatus status);

    const QString& Note()
    {
        return note_;
    }

    const QString& StatusNote()
    {
        return status_note_;
    }

private:
    double radius_;
    QString pid_code_;
    QString note_;
    SafetyUIItemStatus status_;
    QString status_note_;
};

#endif // SAFETYUIITEM_H

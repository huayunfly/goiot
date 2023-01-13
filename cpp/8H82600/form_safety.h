#ifndef FORM_SAFETY_H
#define FORM_SAFETY_H

#include "form_common.h"
#include "safety_ui_item.h"
#include <map>
#include <QTimer>

namespace Ui {
class FormSafety;
}

struct AlarmItemInfo
{
    AlarmItemInfo(const QString& name, const QString& exp) : id(name), note(exp)
    {

    }
    QString id;
    QString note;
};

struct AlarmGroupInfo
{
    AlarmGroupInfo(int i, SafetyUIItem::SafetyUIItemStatus s, int num, int offset) :
        index(i), status(s), total_num(num), item_offset(offset)
    {

    }
    int index;
    SafetyUIItem::SafetyUIItemStatus status;
    int total_num;
    int item_offset;
};

class FormSafety : public FormCommon
{
    Q_OBJECT

public:
    explicit FormSafety(QWidget *parent = nullptr, bool admin = true);
    ~FormSafety();

    bool event(QEvent *event) override;

private:
    void on_buttonClicked(void);

    void on_boxStateChanged(int state);

    void InitExpStatusTable(void);

    void InitAlarmView(void);

    void InitAlarmEnableTable(void);

    void EnableTotalAlarm(void);

    // Force check the alarm checkboxes.
    void CheckTotalAlarmBox();

private:
    Ui::FormSafety *ui;
    // constants
    const int ROW_COUNT = 16;
    const int COL_COUNT = 4;
    const int COL_NAME = 0;
    const int COL_STATUS = 1;
    const int COL_RUN = 2;
    const int COL_STOP = 3;
    // alarm view group
    std::vector<std::shared_ptr<SafetyUIItem> > alarm_ui_items_;
    std::vector<std::pair<QString, std::vector<AlarmItemInfo> > > alarm_items_;
    std::map<std::string, AlarmGroupInfo> alarm_group_;
    std::vector<int> byte_order_big16_ = {8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7};
    std::vector<int> byte_order_big32_ = {24, 25, 26, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7};
    QTimer timer_;
};

#endif // FORM_SAFETY_H

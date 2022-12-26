#ifndef DIALOG_LIQUIDSAMPLINGCHECK_H
#define DIALOG_LIQUIDSAMPLINGCHECK_H

#include <QDialog>

namespace Ui {
class DialogLiquidSamplingCheck;
}

class DialogLiquidSamplingCheck : public QDialog
{
    Q_OBJECT

public:
    explicit DialogLiquidSamplingCheck(QWidget *parent = nullptr);
    ~DialogLiquidSamplingCheck();

private:
    Ui::DialogLiquidSamplingCheck *ui;
};

#endif // DIALOG_LIQUIDSAMPLINGCHECK_H

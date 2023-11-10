#ifndef DIALOG_INPUTINFO_H
#define DIALOG_INPUTINFO_H

#include <QDialog>

namespace Ui {
class DialogInputInfo;
}

class DialogInputInfo : public QDialog
{
    Q_OBJECT

public:
    explicit DialogInputInfo(QWidget *parent = nullptr, const QString& headline = "标题");
    ~DialogInputInfo();

    void keyPressEvent(QKeyEvent* e) override;

    QString NewInput()
    {
        return _new_input;
    }

    bool IsOK()
    {
        return _ok;
    }

private slots:
    void on_lineEdit_info_textChanged(const QString &arg1);

    void on_button_ok_clicked();

    void on_button_cancel_clicked();

private:
    Ui::DialogInputInfo *ui;
    QString _headline;
    QString _new_input;
    bool _ok;
};

#endif // DIALOG_INPUTINFO_H

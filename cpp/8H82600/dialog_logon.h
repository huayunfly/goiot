#ifndef DIALOG_LOGON_H
#define DIALOG_LOGON_H

#include <QDialog>

namespace Ui {
class DialogLogon;
}

class DialogLogon : public QDialog
{
    Q_OBJECT

public:
    explicit DialogLogon(QWidget *parent = nullptr);
    ~DialogLogon();

    bool IsAdmin()
    {
        return admin_logon_;
    }

private slots:
    void on_pushButton_logon_clicked();

    void on_lineEdit_user_textChanged(const QString &arg1);

    void on_lineEdit_password_textChanged(const QString &arg1);

private:
    Ui::DialogLogon *ui;
    bool admin_logon_;
};

#endif // DIALOG_LOGON_H

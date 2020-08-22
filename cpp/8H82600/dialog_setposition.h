#ifndef DIALOG_SETPOSITION_H
#define DIALOG_SETPOSITION_H

#include <QDialog>
#include <QModelIndex>
#include <QItemSelection>

namespace Ui {
class DialogSetPosition;
}

// Set position from 1, 0 is invalid.
class DialogSetPosition : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSetPosition(QWidget *parent = nullptr, int position_num = 0, int position = 0);
    ~DialogSetPosition();

    void keyPressEvent(QKeyEvent* e) override;

    int NewValue()
    {
        return new_value_;
    }

private slots:
    // Selection changed, get the selection.
    void handleSelectionChanged(const QItemSelection& selection);

    // Double click, get listView seletion and call QDialog::accept()
    void on_listView_doubleClicked(const QModelIndex &index);

private:
    Ui::DialogSetPosition *ui;
    int new_value_;
};

#endif // DIALOG_SETPOSITION_H

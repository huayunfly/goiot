#ifndef DIALOG_RECIPE_MGR_H
#define DIALOG_RECIPE_MGR_H

#include <QDialog>

namespace Ui {
class DialogRecipeMgr;
}

class DialogRecipeMgr : public QDialog
{
    Q_OBJECT

public:
    explicit DialogRecipeMgr(QWidget *parent = nullptr,
                             const std::vector<QString>& recipe_names = {});
    ~DialogRecipeMgr();

private:
    void InitRecipeTable();

private:
    std::vector<QString> recipe_names_;
    Ui::DialogRecipeMgr *ui;
};

#endif // DIALOG_RECIPE_MGR_H

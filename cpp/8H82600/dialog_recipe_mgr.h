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

    enum class RecipeAction
    {
        NONE = 0,
        LOAD = 1,
        SAVE = 2,
        DELETE = 3
    };

    RecipeAction GetRecipeAction()
    {
        return action_;
    }

    QString GetActingRecipeName()
    {
        return acting_recipe_name_;
    }

private slots:
    void on_pushButtonLoad_clicked();

    void on_pushButtonSave_clicked();

    void on_pushButtonDelete_clicked();

private:
    void InitRecipeTable();

private:
    std::vector<QString> recipe_names_;
    Ui::DialogRecipeMgr *ui;
    RecipeAction action_;
    QString acting_recipe_name_;
};

#endif // DIALOG_RECIPE_MGR_H

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
    enum class RecipeCategory
    {
        SAMPLING = 0,
        COLLECTION = 1
    };

public:
    explicit DialogRecipeMgr(QWidget *parent = nullptr,
                             const std::vector<QString>& recipe_names = {},
                             const std::vector<std::vector<double>>& params_value = {},
                             RecipeCategory category = RecipeCategory::SAMPLING);
    ~DialogRecipeMgr();

    enum class RecipeAction
    {
        NONE = 0,
        LOAD = 1,
        SAVE = 2,
        DELETE = 3,
        RUN = 4,
        STOP = 5,
        UPDATE_PARAMS = 6
    };

    RecipeAction GetRecipeAction()
    {
        return action_;
    }

    QString GetActingRecipeName()
    {
        return acting_recipe_name_;
    }

    std::vector<std::vector<double>> GetParams()
    {
        return param_values_;
    }

private slots:
    void on_pushButtonLoad_clicked();

    void on_pushButtonSave_clicked();

    void on_pushButtonDelete_clicked();

    void on_pushButtonRun_clicked();

    void on_pushButtonStop_clicked();

    void on_pushButtonUpdateParams_clicked();

private:
    void InitRecipeTable();

    void InitImageParamsTable();

    void InitPressureParamsTable();

private:
    std::vector<QString> recipe_names_;
    Ui::DialogRecipeMgr *ui;
    RecipeAction action_;
    QString acting_recipe_name_;
    std::vector<std::vector<double>> param_values_;
};

#endif // DIALOG_RECIPE_MGR_H

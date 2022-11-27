#include "dialog_recipe_mgr.h"
#include "ui_dialog_recipe_mgr.h"
#include <QDateTime>
#include <QFont>


DialogRecipeMgr::DialogRecipeMgr(QWidget *parent, const std::vector<QString>& recipe_names,
                                 const std::vector<std::vector<double>>& params_value,
                                 RecipeCategory category) :
    QDialog(parent), recipe_names_(recipe_names),
    ui(new Ui::DialogRecipeMgr), action_(RecipeAction::NONE),
    param_values_(params_value)
{
    ui->setupUi(this);
    InitRecipeTable();
    if (category == RecipeCategory::SAMPLING)
    {
        InitImageParamsTable();
    }
    else
    {
        InitPressureParamsTable();
    }
}

DialogRecipeMgr::~DialogRecipeMgr()
{
    delete ui;
}

void DialogRecipeMgr::InitRecipeTable()
{
    ui->tableWidgetRecipe->setColumnCount(2);
    ui->tableWidgetRecipe->setRowCount(recipe_names_.size());
    ui->tableWidgetRecipe->setHorizontalHeaderLabels(QStringList({"名称", "时间"}));
    ui->tableWidgetRecipe->horizontalHeader()->setVisible(true);
    ui->tableWidgetRecipe->verticalHeader()->setVisible(false);
    ui->tableWidgetRecipe->horizontalHeader()->setDefaultSectionSize(180);
    ui->tableWidgetRecipe->verticalHeader()->setDefaultSectionSize(25);
    ui->tableWidgetRecipe->setAlternatingRowColors(true);
    int index = 0;
    for (auto& name : recipe_names_)
    {
        ui->tableWidgetRecipe->setItem(index, 0, new QTableWidgetItem(name));

        ui->tableWidgetRecipe->item(index, 0)->setFont(QFont("tahoma", 9, QFont::Black));
        ui->tableWidgetRecipe->item(index, 0)->setFlags(Qt::ItemIsEnabled);
        int pos = name.lastIndexOf("_");
        if (pos <= 0)
        {
            continue;
        }
        QString time = QDateTime::fromSecsSinceEpoch(
                    name.mid(pos + 1, name.length() - pos - 1).toULong()).toString("yyyy-MM-dd hh:mm:ss");
        ui->tableWidgetRecipe->setItem(index, 1, new QTableWidgetItem(time));
        ui->tableWidgetRecipe->item(index, 1)->setFlags(Qt::ItemIsSelectable);
        ui->tableWidgetRecipe->item(index, 1)->setFont(QFont("tahoma", 9, QFont::Normal));
        index++;
    }
    connect(ui->tableWidgetRecipe, &QTableWidget::itemClicked, this,
            [&] () {this->setWindowTitle(QString("配方管理 -> ") + ui->tableWidgetRecipe->currentItem()->text());});
}

void DialogRecipeMgr::InitImageParamsTable()
{
    std::vector<QString> param_names = {"ROI_X", "ROI_Y", "ROI_SIDE", "LOWER_THRESHOLD",
                                       "UPPER_THRESHOLD", "DIRECTION", "MIN_LEN", "MIN_COUNT", "MIN_RATIO", "TIME_LIMIT"};

    if (param_values_.empty())
    {
        param_values_ = {{200, 100, 280, 15, 30, 15, 10, 10, 0.8, 10.0},
                               {200, 100, 280, 15, 30, 15, 10, 10, 0.8, 10.0}};
    }

    ui->tableWidgetParams->setColumnCount(3);
    ui->tableWidgetParams->setRowCount(param_names.size());
    ui->tableWidgetParams->setHorizontalHeaderLabels(QStringList({"名称", "V0值", "V1值"}));
    ui->tableWidgetParams->horizontalHeader()->setVisible(true);
    ui->tableWidgetParams->verticalHeader()->setVisible(false);
    ui->tableWidgetParams->horizontalHeader()->setDefaultSectionSize(180);
    ui->tableWidgetParams->verticalHeader()->setDefaultSectionSize(25);
    ui->tableWidgetParams->setAlternatingRowColors(true);

    int index = 0;
    for (auto& name : param_names)
    {
        ui->tableWidgetParams->setItem(index, 0, new QTableWidgetItem(name));
        ui->tableWidgetParams->item(index, 0)->setFont(QFont("tahoma", 9, QFont::Black));
        ui->tableWidgetParams->item(index, 0)->setFlags(Qt::ItemIsEnabled);

        ui->tableWidgetParams->setItem(index, 1, new QTableWidgetItem(
                                           QString::number(param_values_.at(0).at(index))));
        ui->tableWidgetParams->item(index, 1)->setFont(QFont("tahoma", 9, QFont::Normal));
        ui->tableWidgetParams->setItem(index, 2, new QTableWidgetItem(
                                           QString::number(param_values_.at(1).at(index))));
        ui->tableWidgetParams->item(index, 2)->setFont(QFont("tahoma", 9, QFont::Normal));
        index++;
    }
}

void DialogRecipeMgr::InitPressureParamsTable()
{
    std::vector<QString> param_names = {"LOWER_PRESSURE", "DROP_RATIO", "TIME_LIMIT"};

    if (param_values_.empty())
    {
        param_values_ = {{1.5, 0.5, 30.0}, {1.5, 0.5, 30.0}};
    }

    ui->tableWidgetParams->setColumnCount(3);
    ui->tableWidgetParams->setRowCount(param_names.size());
    ui->tableWidgetParams->setHorizontalHeaderLabels(QStringList({"名称", "P0值", "P1值"}));
    ui->tableWidgetParams->horizontalHeader()->setVisible(true);
    ui->tableWidgetParams->verticalHeader()->setVisible(false);
    ui->tableWidgetParams->horizontalHeader()->setDefaultSectionSize(180);
    ui->tableWidgetParams->verticalHeader()->setDefaultSectionSize(25);
    ui->tableWidgetParams->setAlternatingRowColors(true);

    int index = 0;
    for (auto& name : param_names)
    {
        ui->tableWidgetParams->setItem(index, 0, new QTableWidgetItem(name));
        ui->tableWidgetParams->item(index, 0)->setFont(QFont("tahoma", 9, QFont::Black));
        ui->tableWidgetParams->item(index, 0)->setFlags(Qt::ItemIsEnabled);

        ui->tableWidgetParams->setItem(index, 1, new QTableWidgetItem(
                                           QString::number(param_values_.at(0).at(index))));
        ui->tableWidgetParams->item(index, 1)->setFont(QFont("tahoma", 9, QFont::Normal));
        ui->tableWidgetParams->setItem(index, 2, new QTableWidgetItem(
                                           QString::number(param_values_.at(1).at(index))));
        ui->tableWidgetParams->item(index, 2)->setFont(QFont("tahoma", 9, QFont::Normal));
        index++;
    }
}

void DialogRecipeMgr::on_pushButtonLoad_clicked()
{
    if (ui->tableWidgetRecipe->currentItem() &&
            ui->tableWidgetRecipe->currentItem()->text().isEmpty())
    {
        return;
    }
    acting_recipe_name_ = ui->tableWidgetRecipe->currentItem()->text();
    action_ = RecipeAction::LOAD;
    this->close();
}

void DialogRecipeMgr::on_pushButtonSave_clicked()
{
    if (ui->lineEditSave->text().isEmpty() || ui->lineEditSave->text().length() > 20)
    {
        return;
    }
    acting_recipe_name_ = ui->lineEditSave->text().toLower();
    action_ = RecipeAction::SAVE;
    this->close();
}

void DialogRecipeMgr::on_pushButtonDelete_clicked()
{
    if (ui->tableWidgetRecipe->currentItem() &&
            ui->tableWidgetRecipe->currentItem()->text().isEmpty())
    {
        return;
    }
    acting_recipe_name_ = ui->tableWidgetRecipe->currentItem()->text();
    action_ = RecipeAction::DELETE;
    this->close();
}

void DialogRecipeMgr::on_pushButtonRun_clicked()
{
    if (ui->tableWidgetRecipe->currentItem() &&
            ui->tableWidgetRecipe->currentItem()->text().isEmpty())
    {
        return;
    }
    acting_recipe_name_ = ui->tableWidgetRecipe->currentItem()->text();
    action_ = RecipeAction::RUN;
    this->close();
}

void DialogRecipeMgr::on_pushButtonStop_clicked()
{
    if (ui->tableWidgetRecipe->currentItem() &&
            ui->tableWidgetRecipe->currentItem()->text().isEmpty())
    {
        return;
    }
    acting_recipe_name_ = ui->tableWidgetRecipe->currentItem()->text();
    action_ = RecipeAction::STOP;
    this->close();
}

void DialogRecipeMgr::on_pushButtonUpdateParams_clicked()
{
    param_values_.clear();
    param_values_.resize(2);

    for (int i = 0; i < ui->tableWidgetParams->rowCount(); i++)
    {
        bool ok1, ok2;
        double v1 = ui->tableWidgetParams->item(i, 1)->text().toDouble(&ok1);
        double v2 = ui->tableWidgetParams->item(i, 2)->text().toDouble(&ok2);
        if (ok1 && ok2)
        {
            param_values_.at(0).push_back(v1);
            param_values_.at(1).push_back(v2);
        }
        else
        {
            param_values_.at(0).push_back(0);
            param_values_.at(1).push_back(0);
        }
    }
    action_ = RecipeAction::UPDATE_PARAMS;
    this->close();
}

#include "dialog_recipe_mgr.h"
#include "ui_dialog_recipe_mgr.h"
#include <QDateTime>
#include <QFont>


DialogRecipeMgr::DialogRecipeMgr(QWidget *parent, const std::vector<QString>& recipe_names) :
    QDialog(parent), recipe_names_(recipe_names),
    ui(new Ui::DialogRecipeMgr), action_(RecipeAction::NONE)
{
    ui->setupUi(this);
    InitRecipeTable();
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

#include "dialog_recipe_mgr.h"
#include "ui_dialog_recipe_mgr.h"
#include <QDateTime>
#include <QFont>


DialogRecipeMgr::DialogRecipeMgr(QWidget *parent, const std::vector<QString>& recipe_names) :
    QDialog(parent), recipe_names_(recipe_names),
    ui(new Ui::DialogRecipeMgr)
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
    ui->tableWidgetRecipe->setRowCount(20);
    ui->tableWidgetRecipe->setHorizontalHeaderLabels(QStringList({"名称", "时间"}));
    ui->tableWidgetRecipe->horizontalHeader()->setVisible(true);
    ui->tableWidgetRecipe->verticalHeader()->setVisible(false);
    ui->tableWidgetRecipe->horizontalHeader()->setDefaultSectionSize(150);
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
}

#include <qevent.h>
#include <QtGui> // for QStringListModel
#include <QAbstractItemView>
#include "dialog_setposition.h"
#include "ui_dialog_setposition.h"

DialogSetPosition::DialogSetPosition(QWidget *parent, int position_num, int position) :
    QDialog(parent),
    ui(new Ui::DialogSetPosition), new_value_(0/* invalid position */)
{
    ui->setupUi(this);
    Qt::WindowFlags flags= this->windowFlags();
    this->setWindowFlags(flags&~Qt::WindowContextHelpButtonHint);

    // check and initialize
    auto model = new QStringListModel(ui->listView);
    QStringList data;
    for (int i = 0; i < position_num; i++)
    {
        data << QString::number(i + 1, 10, 0); // (num, base, precision)
    }
    model->setStringList(data);
    ui->listView->setModel(model);
    if (position > 0 && position <= position_num)
    {
        QModelIndex index = model->index(position - 1); // listview index from 0
        ui->listView->setCurrentIndex(index);
        new_value_ = position;
    }
    // no editable
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // slot
    connect(ui->listView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
       this, SLOT(handleSelectionChanged(QItemSelection)));
}

DialogSetPosition::~DialogSetPosition()
{
    if (ui->listView->model() != nullptr)
    {
        delete ui->listView->model();
    }
    delete ui;
}

void DialogSetPosition::keyPressEvent(QKeyEvent* e)
{
    assert(e != nullptr);
    if( e->key() == Qt::Key_Return)
    {
        this->accept();
    }
    else if (e->key() == Qt::Key_Escape)
    {
        this->reject();
    }
    else
    {
        QDialog::keyPressEvent(e);
    }
}

void DialogSetPosition::handleSelectionChanged(const QItemSelection& selection)
{
   if(!selection.indexes().isEmpty())
   {
      new_value_ = selection.indexes().first().row() + 1;
   }
}

void DialogSetPosition::on_listView_doubleClicked(const QModelIndex &index)
{
    new_value_ = index.row() + 1;
    this->accept();
}

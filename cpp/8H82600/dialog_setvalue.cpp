#include <QRegExpValidator>
#include <qevent.h>
#include "dialog_setvalue.h"
#include "ui_dialog_setvalue.h"

DialogSetValue::DialogSetValue(QWidget *parent, const QString& current, MeasurementUnit unit,
                               int high_limit, int low_limit) :
    QDialog(parent),
    ui(new Ui::DialogSetValue),
    new_value_(0.0), high_limit_(high_limit), low_limit_(low_limit)
{
    ui->setupUi(this);
    Qt::WindowFlags flags= this->windowFlags();
    this->setWindowFlags(flags&~Qt::WindowContextHelpButtonHint);

    // get controls
    auto text_current_sv = this->findChild<QTextEdit*>("textEditCurrentSV");
    assert(text_current_sv != nullptr);
    auto text_sv = this->findChild<QLineEdit*>("lineEditSV");
    assert(text_sv != nullptr);
    auto text_unit = this->findChild<QTextEdit*>("textEditUnit");
    assert(text_unit != nullptr);

    // set properties
    text_current_sv->setStyleSheet("background-color:lightyellow");
    QRegExp float_regex("100|([0-9]{0,4}[\\.][0-9]{1})");
    text_sv->setValidator(new QRegExpValidator(float_regex, this));
    text_sv->setFocus();
    text_unit->setStyleSheet("background:transparent;border-width:0;border-style:outset");

    // set values
    bool is_numeric = false;
    current.toFloat(&is_numeric);
    if (is_numeric)
    {
       text_current_sv->setText(current);
       text_sv->setText(current);
    }
    switch (unit)
    {
    case MeasurementUnit::BARA:
        text_unit->setText("barA");
        break;
    case MeasurementUnit::BARG:
        text_unit->setText("barG");
        break;
    case MeasurementUnit::SCCM:
        text_unit->setText("SCCM");
        break;
    case MeasurementUnit::DEGREE:
        text_unit->setText(u8"°C");
        break;
    default:
        text_unit->setText("-");
        break;
    }
}

DialogSetValue::~DialogSetValue()
{
    delete ui;
}

void DialogSetValue::keyPressEvent(QKeyEvent* e)
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

void DialogSetValue::on_lineEditSV_textChanged(const QString &arg1)
{
    bool is_numeric = false;
    float f = arg1.toFloat(&is_numeric);
    if (is_numeric)
    {
        if (f <= high_limit_ && f >= low_limit_) // Check value range
        {
            new_value_ = f;
        }
    }
}

#include <QLabel>
#include "form_analysis.h"
#include "ui_form_analysis.h"
#include "events.h"
#include "resourcedef.h"

FormAnalysis::FormAnalysis(QWidget *parent) :
    FormCommon(parent, "analysis", QString::fromUtf8("分析接口")),
    ui(new Ui::FormAnalysis)
{
    ui->setupUi(this);
    InitUiState();
}

FormAnalysis::~FormAnalysis()
{
    delete ui;
}

bool FormAnalysis::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormAnalysis::InitUiState()
{
    ui->label_TICA3110->installEventFilter(this);
    ui->label_TICA3120->installEventFilter(this);
    ui->label_TICA3130->installEventFilter(this);
    ui->label_TICA3140->installEventFilter(this);

    ui->label_GC1->installEventFilter(this);
    ui->label_GC2->installEventFilter(this);
}

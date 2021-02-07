#ifndef FORM_LIQUIDDISTRIBUTOR_H
#define FORM_LIQUIDDISTRIBUTOR_H

#include <QWidget>
#include <form_common.h>

namespace Ui {
class FormLiquidDistributor;
}

enum class LiquidDistributorGroup
{
    SAMPLING = 0,
    COLLECTION = 1
};

class FormLiquidDistributor : public FormCommon
{
    Q_OBJECT

public:
    explicit FormLiquidDistributor(QWidget *parent = nullptr,
                                   const QString& object_name = QString(),
                                   const QString& display_name = QString(),
                                   LiquidDistributorGroup group = LiquidDistributorGroup::SAMPLING);
    ~FormLiquidDistributor();

private:
    Ui::FormLiquidDistributor *ui;
    LiquidDistributorGroup group_;

    const int ROW_COUNT = 32 + 1; // 1 seperator row
    const int COL_COUNT = 20 + 1; // 1 seperator column
    const int COL_POS = 0;
    const int COL_CHANNEL = 1;
    const int COL_FLOW_LIMIT = 2;
    const int COL_TIME = 3;
    const int COL_PURGE = 4;
    const int COL_PURGE_PORT = 5;
};

#endif // FORM_LIQUIDDISTRIBUTOR_H

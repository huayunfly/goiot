// @purpose: common form, dealing with the event dispatch, UI interaction.
// @date: 2020.08.22
// @author: huayunfly at 126.com

#ifndef FORM_COMMON_H
#define FORM_COMMON_H

#include <QWidget>

class FormCommon : public QWidget
{
    // About qmake error: undefined reference to 'vtable for FormCommon',
    // add Q_OBJECT to use signal and slot and recompile the whole project.
    Q_OBJECT

public:
    explicit FormCommon(QWidget *parent = nullptr) : QWidget(parent)
    {
    }

    virtual ~FormCommon()
    {

    }

protected:
    // Event handlers
    bool event(QEvent *event) override;
};

#endif // FORM_COMMON_H

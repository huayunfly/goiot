// @purpose: common form, dealing with the event dispatch, UI interaction.
// @date: 2020.08.22
// @author: huayunfly at 126.com

#ifndef FORM_COMMON_H
#define FORM_COMMON_H

#include <unordered_map>
#include <QWidget>
#include "data_model.h"

// Virtual device type
enum class VDeviceType
{
    ONOFF = 0,
    SVALVE = 1
};

typedef struct tagUiStateDef
{
    tagUiStateDef() : normal_pixmap(QString()), active_pixmap(QString()),
        error_pixmap(QString()), high_limit(0), low_limit(0), device_type(VDeviceType::ONOFF), measure_unit(MeasurementUnit::NONE)
    {

    }

    tagUiStateDef(const QString& normal, const QString& active, const QString& error, int high, int low, VDeviceType dtype, MeasurementUnit unit) :
        normal_pixmap(normal), active_pixmap(active), error_pixmap(error), high_limit(high), low_limit(low), device_type(dtype), measure_unit(unit)
    {

    }

    QString normal_pixmap;
    QString active_pixmap;
    QString error_pixmap;
    int high_limit;
    int low_limit;
    VDeviceType device_type;
    MeasurementUnit measure_unit;
} UiStateDef;

class FormCommon : public QWidget
{
    // <summary>
    // About qmake error: undefined reference to 'vtable for FormCommon',
    // add Q_OBJECT to use signal and slot and recompile the whole project.
    // </summary>
    Q_OBJECT

public:
    explicit FormCommon(QWidget *parent = nullptr);

    virtual ~FormCommon()
    {
    }

    // <summary>
    // Get the page's display name.
    // </summary>
    // <returns>Display name</returns>
    virtual QString GetDisplayName()
    {
        return QString("void");
    }


    // <summary>
    // Initialize the ui_state_map_.
    // </summary>
    virtual void InitUiState()
    {

    }

protected:
    // Event handlers
    bool event(QEvent *event) override;

    // <summary>
    // Get Ui state by name.
    // </summary>
    // <param name="ui_name">Ui name</param>
    // <returns>Ui state if it is found, otherwise an empty one.</returns>
    UiStateDef GetUiState(const QString& ui_name);


    // <summary>
    // Ui sets value using setvalue dialog and write to data manager.
    // </summary>
    // <param name="sender">Widget sender.</sender>
    void UiSetValue(QWidget* sender);

    // <summary>
    // Ui sets value using onoff dialog and write to data manager.
    // </summary>
    // <param name="sender">Widget sender.</sender>
    void UiSetState(QWidget* sender);

protected:
    std::unordered_map<QString/* ui name */, UiStateDef> ui_state_map_;
};

#endif // FORM_COMMON_H

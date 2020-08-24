// @purpose UI data model
// @author huayunfly at 126.com
// @date 2020.08.22
#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <unordered_map>
#include <QWidget>

enum class MeasurementUnit
{
    NONE = 0,
    DEGREE = 1,
    BARA = 2,
    BARG = 3,
    SCCM = 4,
};

enum class WidgetType
{
    TEXT = 0,
    STATE = 1,
};

typedef struct tagUiInfo
{
    tagUiInfo() : parent(nullptr), ui_name(QString()), type(WidgetType::TEXT),
        decimals(0), high_limit(0), low_limit(0)
    {

    }

    tagUiInfo(QWidget* p, const QString& n, WidgetType t, int d, int high, int low) :
        parent(p), ui_name(n), type(t), decimals(d), high_limit(high), low_limit(low)
    {

    }
    QWidget* parent;
    QString ui_name;
    WidgetType type;
    int decimals;
    int high_limit;
    int low_limit;
} UiInfo;

typedef struct tagDataDef
{
    tagDataDef() : pv_id(std::string()), sv_read_id(std::string()), sv_write_id(std::string())
    {

    }

    tagDataDef(const std::string& pv, const std::string& sv_r, const std::string& sv_w) :
        pv_id(pv), sv_read_id(sv_r), sv_write_id(sv_w)
    {

    }
    std::string pv_id;
    std::string sv_read_id;
    std::string sv_write_id;
} DataDef;

class DataModel
{
public:
    DataModel() : data_to_ui_map_(), ui_to_data_map_()
    {

    }
    DataModel(const DataModel& model) = delete;
    DataModel operator=(const DataModel& model) = delete;

    // <summary>
    // Add a item pair into the dato-to-ui map. No thread safe.
    // </summary>
    // <param name="data_id">A DataInfo id</param>
    // <param name="ui_info">UiInfo</param>
    void SetDataToUiMap(const std::string& data_id, UiInfo ui_info);

    // <summary>
    // Set the ui-to-data map. No thread safe.
    // ui name rule: parent_ui_objectName().ui_name
    // <summary>
    // <param name="ui_key">An ui key</param>
    // <param name="data_def">DataDef</param>
    void SetUiToDataMap(const QString& ui_key, DataDef data_def);

    // <summary>
    // Get UiInfo by a DataInfo id.
    // </summary>
    // <param name="data_id">A DataInfo id</param>
    // <returns>An UiInfo if it is found, otherwise an empty UiInfo.</returns>
    UiInfo GetUiInfo(const std::string& data_id);

    // <summary>
    // Get DataInfo id by an ui name.
    // </summary>
    // <param name="ui_name">An Ui name</param>
    // <returns>A DataInfo id if it is found, otherwise an empty string.</returns>
    DataDef GetDataDef(const QString& ui_name);

private:
    std::unordered_map<std::string/* data id */, UiInfo> data_to_ui_map_;
    std::unordered_map<QString/* ui name */, DataDef/* data definition */> ui_to_data_map_;
};

#endif // DATAMODEL_H

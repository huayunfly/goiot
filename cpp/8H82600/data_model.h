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
    tagUiInfo() : parent(nullptr), ui_name(QString()), decimals(0), type(WidgetType::TEXT)
    {

    }

    tagUiInfo(QWidget* p, const QString& n, int d, WidgetType t) :
        parent(p), ui_name(n), decimals(d), type(t)
    {

    }
    QWidget* parent;
    QString ui_name;
    int decimals;
    WidgetType type;
} UiInfo;

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
    // Set the ui-to-data map from the dato-to-ui map. No thread safe.
    // ui key rule: parent_ui_objectName().ui_name
    // <summary>
    void SetUiToDataMap();

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
    std::string GetDataId(const QString& ui_name);

private:
    std::unordered_map<std::string/* data id */, UiInfo> data_to_ui_map_;
    std::unordered_map<QString/* ui name */, std::string/* data id */> ui_to_data_map_;
};

#endif // DATAMODEL_H

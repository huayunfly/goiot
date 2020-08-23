#include "data_model.h"

void DataModel::SetDataToUiMap(const std::string& data_id, UiInfo ui_info)
{
    data_to_ui_map_.emplace(data_id, ui_info);
}

void DataModel::SetUiToDataMap()
{
    ui_to_data_map_.clear();
    for (const auto& pair : data_to_ui_map_)
    {
        // ui key rule: parent_name.ui_name
        ui_to_data_map_.emplace(pair.second.parent->accessibleName() + "." + pair.second.ui_name, pair.first);
    }
}

UiInfo DataModel::GetUiInfo(const std::string& data_id)
{
    const auto iter = data_to_ui_map_.find(data_id);
    if (iter != data_to_ui_map_.cend())
    {
        return iter->second;
    }
    return UiInfo();
}

std::string DataModel::GetDataId(const QString& ui_name)
{
    const auto iter = ui_to_data_map_.find(ui_name);
    if (iter != ui_to_data_map_.end())
    {
        return iter->second;
    }
    return std::string();
}

#include "data_model.h"

void DataModel::SetDataToUiMap(const std::string& data_id, UiInfo ui_info)
{
    data_to_ui_map_.emplace(data_id, ui_info);
}

void DataModel::SetUiToDataMap(const QString& ui_key, DataDef data_def)
{
    ui_to_data_map_.emplace(ui_key, data_def);
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


DataDef DataModel::GetDataDef(const QString& ui_name)
{
    const auto iter = ui_to_data_map_.find(ui_name);
    if (iter != ui_to_data_map_.end())
    {
        return iter->second;
    }
    return DataDef();
}

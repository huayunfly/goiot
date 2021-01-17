#include "data_model.h"

void DataModel::SetDataToUiMap(const std::string& data_id, UiInfo ui_info)
{
    data_to_ui_map_.emplace(data_id, std::vector<UiInfo>({ui_info}));
}

void DataModel::SetDataToUiMap(const std::string& data_id,
                               std::initializer_list<UiInfo> ui_info_list)
{
    data_to_ui_map_.emplace(data_id, ui_info_list);
}

void DataModel::SetUiToDataMap(const QString& ui_key, DataDef data_def)
{
    ui_to_data_map_.emplace(ui_key, data_def);
}

std::vector<UiInfo> DataModel::GetUiInfo(const std::string& data_id, bool& ok)
{
    ok = false;
    const auto iter = data_to_ui_map_.find(data_id);
    if (iter != data_to_ui_map_.cend())
    {
        ok = true;
        return iter->second;
    }
    return std::vector<UiInfo>();
}


DataDef DataModel::GetDataDef(const QString& ui_name, bool& ok)
{
    ok = false;
    const auto iter = ui_to_data_map_.find(ui_name);
    if (iter != ui_to_data_map_.end())
    {
        ok = true;
        return iter->second;
    }
    return DataDef();
}

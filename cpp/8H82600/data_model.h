// @purpose UI data model
// @author huayunfly at 126.com
// @date 2020.08.22
#ifndef DATAMODEL_H
#define DATAMODEL_H

enum class MeasurementUnit
{
    NONE = 0,
    DEGREE = 1,
    BARA = 2,
    BARG = 3,
    SCCM = 4,
};

class DataModel
{
public:
    DataModel();
    DataModel(const DataModel& model) = delete;
    DataModel operator=(const DataModel& model) = delete;
};

#endif // DATAMODEL_H

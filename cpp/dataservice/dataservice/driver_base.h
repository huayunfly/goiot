#pragma once

 // @purpose: DriverBase Interface
 // @author: huayunfly@126.com
 // @date: 2020.03.14
 // @copyright: GNU
 // @version: 0.1

#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include <system_error>
#include "ThreadSafeQueue.h"

// Test to see if we are building a DLL.
// If we are, specify that we are exporting
// to the DLL, otherwise don't worry (we
// will manually import the functions).
#ifdef BUILD_DLL
#define DLLAPI __declspec(dllexport)
#else
#define DLLAPI
#endif // BUILD_DLL

typedef int RESULT_DSAPI;

namespace goiot
{
	enum class ReadWritePrivilege
	{
		READ_ONLY = 0,
		WRITE_ONLY = 1,
		READ_WRITE = 2
	};

	enum class DataFlowType
	{
		REFRESH = 0,
		READ,
		WRITE,
		ASYNC_READ,
		ASYNC_WRITE,
		READ_RETURN,
		WRITE_RETURN
	};

	// register: driver-specified, DF (float), WUB (16bits unsigned byte), WB (16bits signed byte), DUB (32bits unsigned byte), DB (32bits signed byte)
	enum class DataType
	{
		DF = 0,
		WUB,
		WB,
		DUB,
		DB
	};

	// modbus register zone
	enum class DataZone
	{
		OUTPUT_RELAY = 0,
		INPUT_RELAY = 1,
		INPUT_REGISTER = 3,
		OUTPUT_REGISTER = 4
	};

	struct ConnectionInfo
	{
		ConnectionInfo() : port("COM1"), baud(9600), parity('N'), data_bit(8), stop_bit(1), response_to_msec(500)
		{
		}
		std::string port;
		int baud;
		char parity; // 'N', 'O', 'E'
		int data_bit;
		int stop_bit;
		uint32_t response_to_msec; // in micro second
	};

	struct DataInfo
	{
		DataInfo() : id(""), name(""), address(0), register_address(0), read_write_priviledge(ReadWritePrivilege::READ_ONLY),
			data_flow_type(DataFlowType::READ), data_type(DataType::WB), data_zone(DataZone::OUTPUT_REGISTER), int_value(0), float_value(0), char_value(""), 
			timestamp(std::chrono::system_clock().now().time_since_epoch().count()), result(0)
		{

		}

		DataInfo(const std::string& new_id) : id(new_id), name(""), address(0), register_address(0), read_write_priviledge(ReadWritePrivilege::READ_ONLY),
			data_flow_type(DataFlowType::READ), data_type(DataType::WB), data_zone(DataZone::OUTPUT_REGISTER), int_value(0), float_value(0), char_value(""),
			timestamp(std::chrono::system_clock().now().time_since_epoch().count()), result(0)
		{

		}

		// For data copy.
		DataInfo(const std::string& new_id, const std::string& new_name, int new_address, int new_register_address, ReadWritePrivilege new_read_write_priviledge,
			DataFlowType new_data_flow_type, DataType new_data_type, DataZone new_data_zone, int new_int_value, double new_float_value, std::string& new_char_value,
			double new_timestamp) : 
			id(new_id), name(new_name), address(new_address), register_address(new_register_address), read_write_priviledge(new_read_write_priviledge),
			data_flow_type(new_data_flow_type), data_type(new_data_type), data_zone(new_data_zone), int_value(new_int_value), float_value(new_float_value), char_value(new_char_value),
			timestamp(new_timestamp), result(0)
		{

		}


		std::string id;
		std::string name;
		int address;
		int register_address;
		ReadWritePrivilege read_write_priviledge; // 0: read, 1: write
		DataFlowType data_flow_type;
		DataType data_type;
		DataZone data_zone;
		double float_value;
		int int_value;
		std::string char_value;
		double timestamp; // in microsecond -> std::chrono::system_clock::now().time_since_epoch().count();
		int result; // Complies with std::error_code
	};

    // This is the base class for the class retrieved from the DLL.
    class DriverBase {
    public:
        virtual ~DriverBase() = default;

        virtual RESULT_DSAPI GetDescription(std::string& description) = 0;

        virtual RESULT_DSAPI InitDriver(const std::string& config, std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> response_queue) = 0;

        virtual RESULT_DSAPI UnitDriver() = 0;
    };

    class DeviceObject {
    public:
        virtual ~DeviceObject() = default;

        virtual RESULT_DSAPI GetDeviceType(std::string& deviceType) = 0;

        virtual RESULT_DSAPI GetDeviceNo(int& deviceNo) = 0;

        virtual RESULT_DSAPI InitDevice() = 0;

        virtual RESULT_DSAPI UninitDevice() = 0;

		// Write data
        // @param<idList>: id list
        // @param<dataList>: out ref, data list in stringstream
        // @param<resultList>: out ref, result list in int
        // @return: error code in int
		virtual RESULT_DSAPI Read(const std::vector<std::string>& idList,
			std::vector<std::stringstream>& dataList, std::vector<RESULT_DSAPI>& resultList) = 0;

        // Write data
        // @param<idList>: id list
        // @param<dataList>: data list in stringstream
        // @param<resultList>: out ref, result list in int
        // @return: error code in int
        virtual RESULT_DSAPI Write(const std::vector<std::string>& idList,
            const std::vector<std::stringstream>& dataList, std::vector<RESULT_DSAPI>& resultList) = 0;
    };
}



// DLL export funcs

// Get an instance of the derived class
// contained in the DLL.
DLLAPI std::unique_ptr<goiot::DriverBase> GetObj(void);

// Get the name of the plugin. This can
// be used in various associated messages.
DLLAPI std::string GetName(void);

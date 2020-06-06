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
		ConnectionInfo() : baud(9600), parity('N'), data_bit(8), stop_bit(1)
		{
		}
		int baud;
		char parity; // 'N', 'O', 'E'
		int data_bit;
		int stop_bit;
	};

	struct DataInfo
	{
		DataInfo() : id(""), name(""), address(0), register_address(0), read_write_priviledge(ReadWritePrivilege::READ_ONLY),
			data_flow_type(DataFlowType::READ), data_type(DataType::WB), data_zone(DataZone::OUTPUT_REGISTER), float_value(0), char_value(""), 
			timestamp(std::chrono::system_clock().now().time_since_epoch().count())
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
		std::string char_value;
		double timestamp; // in microsecond -> std::chrono::system_clock::now().time_since_epoch().count();
	};

    // This is the base class for the class
// retrieved from the DLL. This is used simply
// so that I can show how various types should
// be retrieved from a DLL. This class is to
// show how derived classes can be taken from
// a DLL.
    class DriverBase {
    public:
        virtual ~DriverBase() = default;

        virtual RESULT_DSAPI GetDescription(std::string& description) = 0;

        virtual RESULT_DSAPI InitDriver(const std::string& config) = 0;

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

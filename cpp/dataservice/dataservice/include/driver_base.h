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

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
#include <functional>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
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
	// BT (bit), STR(string)
	enum class DataType
	{
		DF = 0,
		WUB,
		WB,
		DUB,
		DB,
		BT,
		STR
	};

	// modbus register zone
	enum class DataZone
	{
		OUTPUT_RELAY = 0,
		INPUT_RELAY = 1,
		INPUT_REGISTER = 3,
		OUTPUT_REGISTER = 4,
		PLC_DB = 5
	};

	// modbus float decode format
	enum class FloatDecode
	{
		ABCD = 0, // without conversion
		DCBA = 1, // inversed format
		BADC = 2, // swapped bytes
		CDAB = 3  // swapped words
	};

	struct ConnectionInfo
	{
		ConnectionInfo() : port("COM1"), baud(9600), parity('N'), 
			data_bit(8), stop_bit(1), rack(0), slot(0), response_to_msec(500)
		{
		}
		std::string port;
		int baud;
		char parity; // 'N', 'O', 'E'
		int data_bit;
		int stop_bit;
		int rack;
		int slot;
		uint32_t response_to_msec; // in micro second
	};

	struct DataInfo
	{
		DataInfo() : id(""), name(""), address(0), register_address(0), read_write_priviledge(ReadWritePrivilege::READ_ONLY),
			data_flow_type(DataFlowType::READ), data_type(DataType::WB), data_zone(DataZone::OUTPUT_REGISTER), float_decode(FloatDecode::ABCD), int_value(0), float_value(0), char_value(""),
			timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0), result(0), ratio(1.0)
		{

		}

		DataInfo(const std::string& new_id) : id(new_id), name(""), address(0), register_address(0), read_write_priviledge(ReadWritePrivilege::READ_ONLY),
			data_flow_type(DataFlowType::READ), data_type(DataType::WB), data_zone(DataZone::OUTPUT_REGISTER), float_decode(FloatDecode::ABCD), int_value(0), float_value(0), char_value(""),
			timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0), result(0), ratio(1.0)
		{

		}

		// For data copy.
		DataInfo(const std::string& new_id, const std::string& new_name, int new_address, int new_register_address, ReadWritePrivilege new_read_write_priviledge,
			DataFlowType new_data_flow_type, DataType new_data_type, DataZone new_data_zone, FloatDecode new_float_decode, int new_int_value, double new_float_value, const std::string& new_char_value,
			double new_timestamp, int result_code = 0, float new_ratio = 1.0) : 
			id(new_id), name(new_name), address(new_address), register_address(new_register_address), read_write_priviledge(new_read_write_priviledge),
			data_flow_type(new_data_flow_type), data_type(new_data_type), data_zone(new_data_zone), float_decode(new_float_decode), int_value(new_int_value), float_value(new_float_value), char_value(new_char_value),
			timestamp(new_timestamp), result(result_code), ratio(new_ratio)
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
		FloatDecode float_decode; // float decode with value 0(ABCD), 1(DCBA), 2(BADC), 3(CDAB)
		int int_value;
		double float_value;
		std::string char_value;
		double timestamp; // in microsecond -> std::chrono::system_clock::now().time_since_epoch().count();
		int result; // Complies with std::error_code
		float ratio; // Read data from the device * ratio, write data to the device / ratio, ratio can not be zero
	};

	template <typename T>
	class DataEntryCache
	{
	public:
		DataEntryCache() : data_entry_hash_(), entry_mutex_()
		{

		}

		DataEntryCache(const DataEntryCache&) = delete;
		DataEntryCache& operator=(const DataEntryCache&) = delete;

		/// <summary>
		/// Find an entry. If the entry is not found, it will return an empty entry.
		/// </summary>
		/// <param name="entry_key">Entry key.</param>
		/// <returns>A found entry or an empty entry.</returns>
		DataInfo FindEntry(const std::string& entry_key) const
		{
			std::shared_lock<std::shared_mutex> lk(entry_mutex_);
			auto it = data_entry_hash_.find(entry_key); // Fix: const_iterator
			return (it == data_entry_hash_.end()) ? T() : it->second;
		}

		/// <summary>
		/// Update or add an entry.
		/// </summary>
		/// <param name="entry_key">The entry key.</param>
		/// <param name="entry">The entry</param>
		void UpateOrAddEntry(const std::string& entry_key, const T& entry)
		{
			std::lock_guard<std::shared_mutex> lk(entry_mutex_);
			data_entry_hash_[entry_key] = entry;
		}

		/// <summary>
		/// Add an entry. If an entry with the key is existed, an excepion will be thrown out.
		/// </summary>
		/// <param name="entry_key">The entry key.</param>
		/// <param name="entry">The entry.</param>
		void AddEntry(const std::string& entry_key, const T& entry)
		{
			std::lock_guard<std::shared_mutex> lk(entry_mutex_);
			auto it = data_entry_hash_.find(entry_key);
			if (it != data_entry_hash_.end())
			{
				throw std::out_of_range("Data cache contains the entry key already.");
			}
			else
			{
				data_entry_hash_[entry_key] = entry;
			}
		}

		/// <summary>
		/// Get entry key set. (Not thread safe)
		/// </summary>
		/// <returns></returns>
		std::unordered_set<std::string> GetEntryKeys()
		{
			std::shared_lock<std::shared_mutex> lk(entry_mutex_);
			std::unordered_set<std::string> entry_keys;
			for (auto& entry : data_entry_hash_)
			{
				entry_keys.insert(entry.first);
			}
			return entry_keys;
		}

	private:
		std::unordered_map<std::string, T> data_entry_hash_;
		mutable std::shared_mutex entry_mutex_;
	};

    // This is the base class for the class retrieved from the DLL.
    class DriverBase {
    public:
        virtual ~DriverBase() = default;

        virtual RESULT_DSAPI GetDescription(std::string& description) = 0;

		virtual RESULT_DSAPI GetID(std::string& id) = 0;

        virtual RESULT_DSAPI InitDriver(const std::string& config, std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> response_queue,
			std::function<void(const DataInfo&)> set_data_info) = 0;

        virtual RESULT_DSAPI UnitDriver() = 0;

		virtual RESULT_DSAPI AsyncWrite(const std::vector<DataInfo>& data_info_vec) = 0;

		virtual RESULT_DSAPI AsyncRead(const std::vector<std::string> id_vec, 
			std::function<void(std::vector<DataInfo>&&)> read_callback) = 0;
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

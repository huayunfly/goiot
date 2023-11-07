#include "pch.h"
#include <iostream>
#include <cassert>
#include <functional>
#include <algorithm>
#include "DriverWorker.h"


namespace goiot
{
	std::string SqliteDriverWorker::NULL_STRING = "NULL";
	std::map<std::string, std::string> SqliteDriverWorker::_global_data_cache;

	int SqliteDriverWorker::OpenConnection()
	{
		_connection_manager = nullptr;
		{
			sqlite3* db = nullptr;
			sqlite3_open(_connection_details.port.c_str(), &db);
			_connection_manager.reset(db, 
				[](sqlite3* p) { sqlite3_close(p); std::cout << "free sqlite3 ptr."; });
		}
		if (!_connection_manager)
		{
			std::cerr << "sqlite_driver Connection failed: " << std::endl;
			return ENOTCONN;
		}
		else
		{
			std::cout << "sqlite_driver Connected to: " << _connection_details.port << std::endl;
		}
		// sqlite tables
		CreateOrTrimDBTable();
		return 0;
	}

	void SqliteDriverWorker::CloseConnection()
	{
		_connection_manager.reset();
#ifdef _DEBUG
		std::cout << "sqlite_driver worker closed connection.";
#endif // _DEBUG
	}

	void SqliteDriverWorker::Start()
	{
		_refresh = true;
		_threads.emplace_back(&SqliteDriverWorker::Refresh, this);
		_threads.emplace_back(&SqliteDriverWorker::Request_Dispatch, this);
		_threads.emplace_back(&SqliteDriverWorker::Response_Dispatch, this);
	}

	void SqliteDriverWorker::Stop()
	{
		_refresh = false;
		_in_queue.Close();
		_out_queue.Close();
		for (auto& entry : _threads)
		{
			entry.join();
		}
	}

	void SqliteDriverWorker::Refresh()
	{
		while (_refresh)
		{
			try
			{
				std::vector<DataInfo> data_info_vec;
				for (auto data_info : _data_map)
				{
					if (data_info.second.read_write_priviledge != ReadWritePrivilege::WRITE_ONLY)
					{
						data_info_vec.emplace_back(data_info.second.id,
							data_info.second.name, data_info.second.address, data_info.second.register_address,
							data_info.second.read_write_priviledge, DataFlowType::REFRESH, data_info.second.data_type,
							data_info.second.data_zone, data_info.second.float_decode, data_info.second.dword_decode, data_info.second.byte_value, data_info.second.int_value, data_info.second.float_value,
							data_info.second.char_value, std::chrono::duration_cast<std::chrono::milliseconds>(
								std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0);
					}
				}
				if (data_info_vec.size() > 0)
				{
					_in_queue.Put(std::make_shared<std::vector<DataInfo>>(data_info_vec),
						true, std::chrono::milliseconds(1000));
				}
			}
			catch (const QFull&)
			{
				std::cerr << "sqlite_driver:In-queue is full" << std::endl;
			}
			catch (const std::exception& e) {
				std::cerr << "sqlite_driver:EXCEPTION: " << e.what() << std::endl;
			}
			catch (...) {
				std::cerr << "sqlite_driver:EXCEPTION (unknown)" << std::endl;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(_connection_details.refresh_interval_msec));
		}
	}

	void SqliteDriverWorker::Request_Dispatch()
	{
		while (true)
		{
			try
			{
				std::shared_ptr<std::vector<DataInfo>> data_info_vec;
				_in_queue.Get(data_info_vec);
				if (data_info_vec == nullptr) // Improve for a robust SENTINEL
				{
					break; // Exit
				}
				std::shared_ptr<std::vector<DataInfo>> rp_data_info_vec;
				int result_code = 0;

				if (data_info_vec->empty())
				{
					continue;
				}
				if (data_info_vec->at(0).data_flow_type == DataFlowType::REFRESH)
				{
					rp_data_info_vec = ReadMultiTableData(data_info_vec); // Modify data_info_vec directly, may be improve.
				}
				else if (data_info_vec->at(0).data_flow_type == DataFlowType::ASYNC_WRITE)
				{
					rp_data_info_vec = WriteData(data_info_vec);
				}
				else
				{
					throw std::invalid_argument("Unsupported data flow");
				}
				_out_queue.Put(rp_data_info_vec, true, std::chrono::milliseconds(1000));
			}
			catch (const QFull&)
			{
				std::cerr << "sqlite_driver:Out-queue is full" << std::endl;
			}
			catch (const std::exception& e) {
				std::cerr << "sqlite_driver:EXCEPTION: " << e.what() << std::endl;
			}
			catch (...) {
				std::cerr << "sqlite_driver:EXCEPTION (unknown)" << std::endl;
			}
		}
	}

	// Dispatch worker deals with the out_queue request, which may trasnfer data to the DataService.
	void SqliteDriverWorker::Response_Dispatch()
	{
		while (true)
		{
			std::shared_ptr<std::vector<DataInfo>> data_info_vec;
			_out_queue.Get(data_info_vec);
			if (data_info_vec == nullptr) // Improve for a robust SENTINEL
			{
				break; // Exit
			}
			try
			{
				_driver_manager_reponse_queue->PutNoWait(data_info_vec);
			}
			catch (const QFull&)
			{
				assert(false);
				std::cout << "sqlite_driver:driver_manager_response_queue_ is full." << std::endl;
			}
		}
	}

	// Puts asynchronous read request to the in_queue.
	void SqliteDriverWorker::AsyncRead(const std::vector<DataInfo>& data_info_vec, int trans_id)
	{
		if (data_info_vec.size() > 0)
		{
			assert(data_info_vec.at(0).data_flow_type == DataFlowType::ASYNC_READ);
			_in_queue.Put(std::make_shared<std::vector<DataInfo>>(data_info_vec));
		}
	}

	// Puts asynchronous write request to the out_queue. Return non zero if the queue is full.
	int SqliteDriverWorker::AsyncWrite(const std::vector<DataInfo>& data_info_vec, int trans_id)
	{
		if (data_info_vec.size() > 0)
		{
			assert(data_info_vec.at(0).data_flow_type == DataFlowType::ASYNC_WRITE);
			try
			{
				_in_queue.Put(std::make_shared<std::vector<DataInfo>>(data_info_vec),
					true, std::chrono::milliseconds(200)); // prevent blocking DataService poll dispatch()
			}
			catch (const QFull&)
			{
#ifdef _DEBUG
				std::cout << "Sqlite in_queue_ is full." << std::endl;
#endif // _DEBUG
				return ETIMEDOUT;
			}
		}
		return 0;
	}

	void SqliteDriverWorker::DataInfo2DBTableInfo(const std::map<std::string, DataInfo> data_map,
		std::map<std::string, std::list<std::string>>& db_table_info)
	{
		db_table_info.clear();
		// Parse data ID such as sqlite.expinfo.recordpath
		char delim = '.';
		for (auto& elem : data_map)
		{
			std::size_t tail = elem.first.length() - 1;
			std::size_t pos = elem.first.find(delim);
			if (std::string::npos == pos || tail == pos)
			{
				continue;
			}
			std::size_t start = pos + 1;
			pos = elem.first.find(delim, start);
			if (std::string::npos == pos || tail == pos)
			{
				continue;
			}
			std::size_t end = pos - 1;
			std::string table_name = elem.first.substr(start, end - start + 1);
			start = pos + 1;
			std::string col = elem.first.substr(start, tail - start + 1);
			// Insert
			auto found = db_table_info.find(table_name);
			if (db_table_info.end() == found)
			{
				db_table_info.insert(std::make_pair(table_name, std::move(std::list<std::string>{col})));
			}
			else
			{
				// data ID is unique, so we insert directly.
				found->second.push_back(col);
			}
		}
	}


	void SqliteDriverWorker::DataInfo2DBTableInfo(const std::map<std::string, std::string> data_map,
		std::map<std::string, std::list<std::pair<std::string, std::string>>>& db_table_info) 
	{
		db_table_info.clear();
		// Parse data ID such as sqlite.expinfo.recordpath
		char delim = '.';
		for (auto& elem : data_map)
		{
			std::size_t tail = elem.first.length() - 1;
			std::size_t pos = elem.first.find(delim);
			if (std::string::npos == pos || tail == pos)
			{
				continue;
			}
			std::size_t start = pos + 1;
			pos = elem.first.find(delim, start);
			if (std::string::npos == pos || tail == pos)
			{
				continue;
			}
			std::size_t end = pos - 1;
			std::string table_name = elem.first.substr(start, end - start + 1);
			start = pos + 1;
			std::string col = elem.first.substr(start, tail - start + 1);
			// Insert
			auto found = db_table_info.find(table_name);
			if (db_table_info.end() == found)
			{
				db_table_info.insert(std::make_pair(table_name, 
					std::move(std::list<std::pair<std::string, std::string>>{std::make_pair(elem.first, col)})));
			}
			else
			{
				// data ID is unique, so we insert directly.
				found->second.push_back(std::make_pair(elem.first, col));
			}
		}
	}

	void SqliteDriverWorker::CreateOrTrimDBTable()
	{
		if (!_connection_manager)
		{
			return;
		}

		// Manipulate DB
		ClearGlobalDataCache();
		char* err_msg = nullptr;
		for (auto& elem : _table_info)
		{
			std::stringstream ss;
			ss << "CREATE TABLE IF NOT EXISTS " << elem.first << "(";
			for (auto& col : elem.second)
			{
				ss << col << " TEXT,";
			}
			ss << ");";
			std::string sql = ss.str();
			sql.replace(sql.size() - 3, 3, ");");
			int result = sqlite3_exec(
				_connection_manager.get(), sql.c_str(), &SqliteDriverWorker::SqlCallback, nullptr, &err_msg);
			if (SQLITE_OK != result)
			{
				std::cout << err_msg << std::endl;
				sqlite3_free(err_msg);
				continue;
			}
			// Table is empty or not
			ss.str("");
			ss << "SELECT EXISTS(SELECT 1 FROM " << elem.first << ");";
			sql = ss.str();
			char** db_result = nullptr;
			int rows = 0;
			int cols = 0;
			result = sqlite3_get_table(
				_connection_manager.get(), sql.c_str(), &db_result, &rows, &cols, &err_msg);
			if (SQLITE_OK != result)
			{
				std::cout << err_msg << std::endl;
				sqlite3_free(err_msg);
				continue;
			}
			int count = 0;
			std::stringstream ss_count;
			ss_count << db_result[1];
			ss_count >> count;
			if (0 == count)
			{
				ss.str("");
				ss << "INSERT INTO " << elem.first << "(";
				for (auto& item : elem.second)
				{
					ss << item << ",";
				}
				ss << ") VALUES (";
				for (auto& item : elem.second)
				{
					ss << "NULL,";
				}
				ss << ");";
				sql = ss.str();
				sql.replace(sql.find(",)"), 2, ")");
				sql.replace(sql.find(",)"), 2, ")");
				result = sqlite3_exec(
					_connection_manager.get(), sql.c_str(), &SqliteDriverWorker::SqlCallback, nullptr, &err_msg);
				if (SQLITE_OK != result)
				{
					std::cout << err_msg << std::endl;
					sqlite3_free(err_msg);
				}
			}
			sqlite3_free_table(db_result);
		}
	}

	int SqliteDriverWorker::SqlCallback(void* data, int argc, char** argv, char** col_name)
	{
		for (int i = 0; i < argc; i++) 
		{
			_global_data_cache.insert(
				std::make_pair<std::string, std::string>(col_name[i], argv[i] ? argv[i] : NULL_STRING));
		}
		return 0;
	}

	std::shared_ptr<std::vector<DataInfo>> SqliteDriverWorker::ReadMultiTableData(
		std::shared_ptr<std::vector<DataInfo>> data_info_vec)
	{
		if (_connection_manager)
		{
			char* err_msg = nullptr;
			std::map<std::string, std::string> new_map;
			for (auto& elem : _table_info)
			{
				ClearGlobalDataCache();
				std::stringstream ss;
				ss << "SELECT ";
				for (auto& col_name : elem.second)
				{
					ss << col_name << ", ";
				}
				ss << "FROM " << elem.first << " LIMIT 1;";
				std::string sql = ss.str();
				sql.replace(sql.find_last_of(","), 1, "");
				int result = sqlite3_exec(
					_connection_manager.get(), sql.c_str(), SqlCallback, nullptr, &err_msg);
				if (SQLITE_OK != result)
				{
					std::cout << err_msg << std::endl;
					sqlite3_free(err_msg);
					continue;
				}
				std::string prefix = _driver_id + "." + elem.first + ".";
				for (auto& item_data : _global_data_cache)
				{
					new_map.insert(std::make_pair(prefix + item_data.first, item_data.second));
				}
			}
			for (std::size_t i = 0; i < data_info_vec->size(); i++)
			{
				data_info_vec->at(i).data_flow_type = DataFlowType::READ_RETURN;
				auto found = new_map.find(data_info_vec->at(i).id);
				if (new_map.end() == found || NULL_STRING == found->second)
				{
					data_info_vec->at(i).result = ENODATA;
				}
				else
				{
					data_info_vec->at(i).result = 0;
					std::stringstream ss(found->second);
					switch (data_info_vec->at(i).data_type)
					{
					case DataType::BT:
						ss >> data_info_vec->at(i).byte_value;
						break;
					case DataType::BB:
						ss >> data_info_vec->at(i).byte_value;
						break;
					case DataType::WB:
					case DataType::WUB:
						ss >> data_info_vec->at(i).int_value;
						break;
					case DataType::DB:
					case DataType::DUB:
						ss >> data_info_vec->at(i).int_value;
						break;
					case DataType::DF:
						ss >> data_info_vec->at(i).float_value;
						break;
					case DataType::STR:
						ss >> data_info_vec->at(i).char_value;
						break;
					default:
						throw std::invalid_argument("Unsupported data type");
					}
				}
				data_info_vec->at(i).timestamp =
					std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
			}
		}
		else
		{
			for (std::size_t i = 0; i < data_info_vec->size(); i++)
			{
				data_info_vec->at(i).data_flow_type = DataFlowType::READ_RETURN;
				data_info_vec->at(i).result = ENOTCONN;
				data_info_vec->at(i).timestamp =
					std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
			}
		}
		return data_info_vec;
	}

	std::shared_ptr<std::vector<DataInfo>> SqliteDriverWorker::WriteData(
		std::shared_ptr<std::vector<DataInfo>> data_info_vec)
	{
		if (_connection_manager)
		{
			std::map<std::string, std::string> dataid_value_map;
			for (std::size_t i = 0; i < data_info_vec->size(); i++)
			{
				auto found = _data_map.find(data_info_vec->at(i).id);
				if (_data_map.end() == found)
				{
					data_info_vec->at(i).data_flow_type = DataFlowType::WRITE_RETURN;
					data_info_vec->at(i).result = ENODATA;
					data_info_vec->at(i).timestamp =
						std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
				}
				else
				{
					data_info_vec->at(i).result = 0;
					std::stringstream ss;
					switch (data_info_vec->at(i).data_type)
					{
					case DataType::BT:
						ss << data_info_vec->at(i).byte_value;
						break;
					case DataType::BB:
						ss << data_info_vec->at(i).byte_value;
						break;
					case DataType::WB:
					case DataType::WUB:
						ss << data_info_vec->at(i).int_value;
						break;
					case DataType::DB:
					case DataType::DUB:
						ss << data_info_vec->at(i).int_value;
						break;
					case DataType::DF:
						ss << data_info_vec->at(i).float_value;
						break;
					case DataType::STR:
						ss << data_info_vec->at(i).char_value;
						break;
					default:
						throw std::invalid_argument("Unsupported data type");
					}
					dataid_value_map.insert(std::make_pair(data_info_vec->at(i).id, ss.str()));
				}
			}
			std::map<std::string, std::list<std::pair<std::string, std::string>>> table_info;
			DataInfo2DBTableInfo(dataid_value_map, table_info);
			// UPDATE
			char* err_msg = nullptr;
			for (auto& elem : table_info)
			{
				std::stringstream ss;
				ss << "UPDATE " << elem.first << " SET ";
				for (auto& item : elem.second)
				{
					ss << item.second << "=" << dataid_value_map[item.first] << ","; // UPDATE all rows
				}
				std::string sql = ss.str();
				sql.replace(sql.find_last_of(","), 1, ";");
				int result = sqlite3_exec(
					_connection_manager.get(), sql.c_str(), SqlCallback, nullptr, &err_msg);
				if (SQLITE_OK != result)
				{
					std::cout << err_msg << std::endl;
					sqlite3_free(err_msg);
					for (auto& item : elem.second)
					{
						auto pos = std::find_if(data_info_vec->begin(), data_info_vec->end(),
							[&item](DataInfo& datainfo) { return datainfo.id == item.first; });
						(*pos).data_flow_type = DataFlowType::WRITE_RETURN;
						(*pos).result = ENODATA;
						(*pos).timestamp =
							std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
					}
				}
				else
				{
					for (auto& item : elem.second)
					{
						auto pos = std::find_if(data_info_vec->begin(), data_info_vec->end(),
							[&item](DataInfo& datainfo) { return datainfo.id == item.first; });
						(*pos).data_flow_type = DataFlowType::WRITE_RETURN;
						(*pos).result = 0;
						(*pos).timestamp =
							std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
					}
				}
			}
		}
		else
		{
			for (std::size_t i = 0; i < data_info_vec->size(); i++)
			{
				data_info_vec->at(i).data_flow_type = DataFlowType::WRITE_RETURN;
				data_info_vec->at(i).result = ENOTCONN;
				data_info_vec->at(i).timestamp =
					std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
			}
		}
		return data_info_vec;
	}
}
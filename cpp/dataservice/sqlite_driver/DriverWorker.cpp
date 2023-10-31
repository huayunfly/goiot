#include "pch.h"
#include <iostream>
#include <cassert>
#include "DriverWorker.h"


namespace goiot
{
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
		return 0;
	}

	void SqliteDriverWorker::CloseConnection()
	{
		_connection_manager.reset();
#ifdef _DEBUG
		std::cout << "s7_driver worker closed connection.";
#endif // _DEBUG
	}

	void SqliteDriverWorker::Start()
	{

	}

	void SqliteDriverWorker::Stop()
	{

	}

	void SqliteDriverWorker::Refresh()
	{
	}
}
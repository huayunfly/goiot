#pragma once

#include "../dataservice/driver_base.h"
#include "S7DriverWorker.h"

namespace goiot
{
	class S7Driver : public DriverBase
	{
	public:
		S7Driver() : driver_worker_(), id_(), worker_ready_(false)
		{

		}

		S7Driver(const S7Driver&) = delete;
		S7Driver& operator=(const S7Driver&) = delete;

		// ͨ�� DriverBase �̳�
		virtual RESULT_DSAPI GetDescription(std::string& description) override
		{
			description = "S7";
			return 0;
		}

		virtual RESULT_DSAPI GetID(std::string& id) override
		{
			id = id_;
			return 0;
		}

		virtual RESULT_DSAPI InitDriver(const std::string& config, std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> response_queue, std::function<void(const DataInfo&)> set_data_info) override;
		virtual RESULT_DSAPI UnitDriver() override;
		virtual RESULT_DSAPI AsyncWrite(const std::vector<DataInfo>& data_info_vec) override;
		virtual RESULT_DSAPI AsyncRead(const std::vector<std::string> id_vec, std::function<void(std::vector<DataInfo>&&)> read_callback) override;

	private:
		std::unique_ptr<S7DriverWorker> driver_worker_;
		std::string id_;
		bool worker_ready_;
	};
}




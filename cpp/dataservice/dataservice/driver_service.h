/**
 * driver_service.h: A driver manager service, dealing with load configuration,
 * load drivers, read/write/refresh data.
 *
 * @author Yun Hua
 * @version 0.1 2020.03.22
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include "driver_base.h"

namespace goiot {
	class DriverMgrService
	{
	public:
		DriverMgrService(const std::wstring& module_path) : module_path_(module_path), drivers_(), driver_descriptions_()
		{

		}

		DriverMgrService(const DriverMgrService& service) = delete;
		DriverMgrService& operator=(const DriverMgrService& service) = delete;

		~DriverMgrService()
		{

		}


	public:
		// Load json configuration from the default position
		// @return <error_code>
		int LoadJsonConfig();
		// Get driver plugins according to the json config.
		// @return <error_code>
		int GetPlugins();

	private:
		const static std::wstring CONFIG_FILE;
		const static std::wstring DRIVER_DIR;
		std::wstring module_path_; // Not include the suffix "/"
		std::vector<std::shared_ptr<std::tuple<std::string, std::string>>> driver_descriptions_;
		std::vector<std::unique_ptr<DriverBase>> drivers_;
	};
}



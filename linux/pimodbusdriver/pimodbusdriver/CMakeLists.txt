# CMakeList.txt: pimodbusdriver 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
set(3rdLib "/home/pi/Documents/3rdlib")

include_directories("${3rdLib}/jsoncpp/1.9.7/include" "${3rdLib}/libmodbus/3.1.11/include")

link_directories("${3rdLib}/jsoncpp/1.9.7/bin" "${3rdLib}/libmodbus/3.1.11/bin")

# 将源代码添加到此项目的可执行文件。
add_library(pimodbusdriver SHARED "dllmain.cpp" "ModbusRtuDriver.cpp" "ModbusRtuDriver.h" "DriverWorker.cpp" "DriverWorker.h")

target_link_libraries(pimodbusdriver jsoncpp modbus)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET pimodbusdriver PROPERTY CXX_STANDARD 17)
endif()

# TODO: 如有需要，请添加测试并安装目标。

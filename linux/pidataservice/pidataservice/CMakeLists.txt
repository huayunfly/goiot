# CMakeList.txt: pidataservice 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

set(3rdLib "/home/pi/Documents/3rdlib")

include_directories("${3rdLib}/jsoncpp/1.9.7/include" "${3rdLib}/hiredis/include")

#link_libraries(
#	"${3rdLib}/jsoncpp/1.9.7/bin/libjsoncpp.so" 
#	"${3rdLib}/hiredis/bin/libhiredis.so"
#)

link_directories("${3rdLib}/jsoncpp/1.9.7/bin" "${3rdLib}/hiredis/bin")

# 将源代码添加到此项目的可执行文件。
add_executable (dataservice "dataservice.cpp" "driver_service.cpp" "driver_service.h")

target_link_libraries(dataservice jsoncpp hiredis)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET dataservice PROPERTY CXX_STANDARD 17)
endif()

# TODO: 如有需要，请添加测试并安装目标。

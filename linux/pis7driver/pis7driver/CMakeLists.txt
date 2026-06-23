# CMakeList.txt: pis7driver 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
set(3rdLib "/home/pi/Documents/3rdlib")

include_directories("${3rdLib}/jsoncpp/1.9.7/include" "${3rdLib}/snap7/1.4.3/include")

link_directories("${3rdLib}/jsoncpp/1.9.7/bin" "${3rdLib}/snap7/1.4.3/bin")

# 动态库生成生成位置无关代码的编译选项 或 add_compile_options(-fPIC)
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 将源代码添加到此项目的可执行文件。
add_library(pis7driver SHARED "dllmain.cpp" "S7Driver.cpp" "S7Driver.h" "S7DriverWorker.cpp" "S7DriverWorker.h" "snap7.cpp")

target_link_libraries(pis7driver jsoncpp snap7)

# 注意boost::asio C++ 20标准编译错误，其支持20 coroutine冲突
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET pis7driver PROPERTY CXX_STANDARD 17)
endif()

# TODO: 如有需要，请添加测试并安装目标。


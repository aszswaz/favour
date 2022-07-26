cmake_minimum_required(VERSION 3.21)
project(favour)

set(CMAKE_CXX_STANDARD 14)
set(JUNIT_TEST ${PROJECT_NAME}_test)
# 把编译命令输出到 compile_commands.json，vim 的 ccls 可以通过这个文件找依赖
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(lib)
include_directories(src)
include_directories(junit)

# 程序源代码文件
set(
        SRC_FILES
        src/gui/PollingTrayIcon.cpp
        src/exception/CurlException.hpp
        src/config/config.cpp
        src/config/hook.cpp
        src/kibana/kibana.cpp
        src/utils/utils.cpp
)

# junit 测试代码
set (
    JUNIT_SRC
    ${SRC_FILES}
    unit/kibana/KibanaManagerTest.cpp
    unit/utils/UtilsTest.cpp
)

# 主程序入口
add_executable(
        ${PROJECT_NAME}
        src/main.cpp
        ${SRC_FILES}
)

# 单元测试
add_executable(
        ${JUNIT_TEST}
        unit/main.cpp
        ${JUNIT_SRC}
)

### 添加依赖
# Qt5
find_package(Qt5 COMPONENTS Widgets REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)
target_link_libraries(${JUNIT_TEST} PRIVATE Qt5::Widgets)

# curl
find_package(CURL REQUIRED)
if (CURL_FOUND)
    message(STATUS "curl version: ${CURL_VERSION_STRING}")
    include_directories(${CURL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CURL_LIBRARIES})
    target_link_libraries(${JUNIT_TEST} PRIVATE ${CURL_LIBRARIES})
endif (CURL_FOUND)

# pthread
find_package(Threads REQUIRED)
if (Threads_FOUND)
    message(STATUS "Thread library: ${CMAKE_THREAD_LIBS_INIT}")
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
    target_link_libraries(${JUNIT_TEST} PRIVATE Threads::Threads)
endif()

### 动态设置宏
# PROFILE 是从命令行获取的变量，如果命令行没有指定这个变量，则赋予默认值
if (NOT DEFINED PROFILE)
    set(PROFILE DEV)
endif()
message("PROFILE: " ${PROFILE})

# 给 GCC 设置宏
add_definitions(-DPROFILE=${PROFILE})

# 开发环境的宏配置
if (PROFILE STREQUAL "DEV")
    # 开启 DEBUG
    set(CMAKE_BUILD_TYPE "Debug")
endif()


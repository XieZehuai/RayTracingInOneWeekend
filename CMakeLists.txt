cmake_minimum_required(VERSION 3.0.0)
project(RayTracingInOneWeekend VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(RayTracingInOneWeekend main.cpp)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置构建类型
# set(CMAKE_BUILD_TYPE "Release")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

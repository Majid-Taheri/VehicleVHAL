cmake_minimum_required(VERSION 3.10)


project(VehicleVHAL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(/usr/local/Cellar/nlohmann-json/3.11.3/include)
include_directories(/usr/local/Cellar/yaml-cpp/0.8.0/include)

set(SOURCES
    VehicleImpl.cpp
    VehicleClient.cpp
)

add_executable(VehicleVHAL ${SOURCES})

target_include_directories(VehicleVHAL PRIVATE /usr/local/Cellar/nlohmann-json/3.11.3/include)

find_library(YAML_CPP_LIB yaml-cpp HINTS /usr/local/Cellar/yaml-cpp/0.8.0/lib)

find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

target_link_libraries(VehicleVHAL PRIVATE ${YAML_CPP_LIB} spdlog::spdlog fmt::fmt)

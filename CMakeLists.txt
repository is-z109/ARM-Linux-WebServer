cmake_minimum_required(VERSION 3.15)

project(ARM_cserve)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

set(SOURCES
    src/main.cpp
    src/log.cpp
    src/tool.cpp
    src/GPIO.cpp
)

add_executable(carmserve_arm ${SOURCES})

target_link_libraries(carmserve_arm
    Threads::Threads
)

target_include_directories(carmserve_arm PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third_party
)
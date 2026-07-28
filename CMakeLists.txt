cmake_minimum_required(VERSION 3.20)

project(crystal-dashboard)


set(CMAKE_CXX_STANDARD 20)


find_package(CURL REQUIRED)


add_executable(
    dashboard
    src/main.cpp
)


target_link_libraries(
    dashboard
    CURL::libcurl
)
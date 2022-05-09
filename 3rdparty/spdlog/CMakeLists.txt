# Copyright(c) 2019 spdlog authors Distributed under the MIT License (http://opensource.org/licenses/MIT)

cmake_minimum_required(VERSION 3.10...3.21)

project(spdlog LANGUAGES CXX)
message(STATUS "Build spdlog: ${SPDLOG_VERSION}")

# ---------------------------------------------------------------------------------------
# Header only version
# ---------------------------------------------------------------------------------------
add_library(spdlog_header_only INTERFACE)
add_library(spdlog::spdlog_header_only ALIAS spdlog_header_only)

target_include_directories(spdlog_header_only INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
                                                        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

# ---------------------------------------------------------------------------------------
# Add required libraries for Android CMake build
# ---------------------------------------------------------------------------------------
if(ANDROID)
    target_link_libraries(spdlog PUBLIC log)
    target_link_libraries(spdlog_header_only INTERFACE log)
endif()

cmake_minimum_required(VERSION 3.24)
project(FreeDDecoder VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Prevent Windows min/max macros from conflicting with std::min/std::max
add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)

# --- Dependencies ---
find_package(imgui CONFIG REQUIRED)

# Exclude oscpp test targets
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
include(cmake/FetchOscpp.cmake)

# --- Sources ---
set(SOURCES
    src/main.cpp
    src/app.cpp
    src/freed/freed_protocol.cpp
    src/freed/freed_receiver.cpp
    src/osc/osc_sender.cpp
    src/gui/gui_manager.cpp
    src/gui/dx11_backend.cpp
    src/utils/network_utils.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${oscpp_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    imgui::imgui
    d3d11.lib
    dxgi.lib
    ws2_32.lib
)

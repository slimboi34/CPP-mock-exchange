cmake_minimum_required(VERSION 3.14)
project(MockExchange VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler flags for high performance
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -Wall -Wextra -Wpedantic")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

# The core library
add_library(exchange_core
    src/OrderBook.cpp
    src/MatchingEngine.cpp
)
target_include_directories(exchange_core PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Main GUI executable
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.4
)
FetchContent_MakeAvailable(imgui)

# Find GLFW installed via homebrew
find_package(glfw3 3.3 REQUIRED)

# The GUI App
add_executable(mock_exchange_gui 
    src/main.cpp 
    src/ExchangeApp.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(mock_exchange_gui PRIVATE 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(mock_exchange_gui PRIVATE 
    exchange_core 
    glfw
)

# Apple specifics for OpenGL
if (APPLE)
    target_link_libraries(mock_exchange_gui PRIVATE "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()

# Tests
enable_testing()
add_subdirectory(tests)

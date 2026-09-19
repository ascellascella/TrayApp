cmake_minimum_required(VERSION 3.16)
project(TrayApp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(UNICODE _UNICODE)

add_executable(TrayApp WIN32
src/main.cpp
src/TrayIcon.cpp
res/app.rc
)

target_include_directories(TrayApp PRIVATE src)

target_link_libraries(TrayApp PRIVATE
user32
shell32
gdi32
)

set_target_properties(TrayApp PROPERTIES
WIN32_EXECUTABLE TRUE
VS_DPI_AWARE "PerMonitor"
)
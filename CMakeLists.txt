cmake_minimum_required(VERSION 3.16)
project(TrayApp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_definitions(UNICODE _UNICODE)
add_compile_options(/utf-8)

# ---------- MIDL: генерация RPC-кода для x64 ----------
set(MIDL_OUTPUTS
    ${CMAKE_BINARY_DIR}/tray_c.c
    ${CMAKE_BINARY_DIR}/tray_s.c
    ${CMAKE_BINARY_DIR}/tray_h.h
)

add_custom_command(
    OUTPUT ${MIDL_OUTPUTS}
    COMMAND midl /nologo /x64 /out ${CMAKE_BINARY_DIR}
            /header tray_h.h
            ${CMAKE_SOURCE_DIR}/idl/tray.idl
    DEPENDS ${CMAKE_SOURCE_DIR}/idl/tray.idl
    VERBATIM
)

add_custom_target(GenerateRpc DEPENDS ${MIDL_OUTPUTS})

# ---------- GUI приложение ----------
add_executable(TrayApp WIN32
    src/main.cpp
    src/TrayIcon.cpp
    src/rpc_client.cpp
    res/app.rc
    ${CMAKE_BINARY_DIR}/tray_c.c
)
add_dependencies(TrayApp GenerateRpc)
target_include_directories(TrayApp PRIVATE src ${CMAKE_BINARY_DIR})
target_link_libraries(TrayApp PRIVATE user32 shell32 gdi32 rpcrt4 advapi32)

set_target_properties(TrayApp PROPERTIES
    WIN32_EXECUTABLE TRUE
    VS_DPI_AWARE "PerMonitor"
)

# ---------- Служба ----------
add_executable(TrayService
    service/service.cpp
    service/session.cpp
    service/rpc.cpp
    ${CMAKE_BINARY_DIR}/tray_s.c
)
add_dependencies(TrayService GenerateRpc)
target_compile_definitions(TrayService PRIVATE UNICODE _UNICODE)
target_link_options(TrayService PRIVATE /ENTRY:wmainCRTStartup)
target_include_directories(TrayService PRIVATE
    ${CMAKE_SOURCE_DIR}/service
    ${CMAKE_BINARY_DIR}
)
target_link_libraries(TrayService PRIVATE user32 advapi32 rpcrt4 wtsapi32 userenv)
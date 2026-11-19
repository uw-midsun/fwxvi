cmake_minimum_required(VERSION 3.16)

project(TelemetryReplay LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(telemetry
    src/main.cpp
    src/telemetry/LogParser.cpp
    src/gui/MainWindow.cpp
)

target_link_libraries(telemetry PRIVATE Qt6::Widgets)

qt_add_resources(telemetry "can_database"
    PREFIX "/"
    FILES can/tools/system_dbc.dbc
)

qt_add_resources(telemetry "branding"
    PREFIX "/"
    FILES assets/team-logo.png
)

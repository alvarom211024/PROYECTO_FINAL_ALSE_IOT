cmake_minimum_required(VERSION 3.10)

project(Proyecto_Final_ALSE_IOT LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

find_package(Qt5 COMPONENTS Widgets Core Gui REQUIRED)
find_package(OpenSSL REQUIRED)

find_library(SQLITE3_LIBRARY sqlite3)
find_path(SQLITE3_INCLUDE_DIR sqlite3.h)

include_directories(
    include
    ${SQLITE3_INCLUDE_DIR}
)

set(SOURCES
    src/main.cpp
    src/User.cpp
    src/Device.cpp
    src/Event.cpp
    src/DatabaseManager.cpp
    src/DeviceSimulator.cpp
    src/mainwindow.cpp
)

set(HEADERS
    include/User.h
    include/Device.h
    include/Event.h
    include/DatabaseManager.h
    include/DeviceSimulator.h
    include/mainwindow.h
)

set(UI_FILES
    ui/mainwindow.ui
)

add_executable(Proyecto_Final_ALSE_IOT
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
)

target_link_libraries(Proyecto_Final_ALSE_IOT
    Qt5::Widgets
    ${SQLITE3_LIBRARY}
    OpenSSL::Crypto
)

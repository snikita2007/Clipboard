cmake_minimum_required(VERSION 3.16)
project(Clipboard)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

file(GLOB PROJECT_SOURCES *.cpp *.h *.qrc)

#Mac
if(APPLE)
    list(APPEND PROJECT_SOURCES mac_utils.mm)
endif()

add_executable(Clipboard ${PROJECT_SOURCES})

target_link_libraries(Clipboard PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui)

if(WIN32)
    # Только для Windows
    target_link_libraries(Clipboard PRIVATE dwmapi)
endif()

if(APPLE)
    # Только для Mac
    target_link_libraries(Clipboard PRIVATE "-framework AppKit")
endif()

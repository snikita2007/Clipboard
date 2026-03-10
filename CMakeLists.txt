cmake_minimum_required(VERSION 3.16)
project(Clipboard)                   

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)


set(CMAKE_PREFIX_PATH "C:/msys64/mingw64")


find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    main.cpp
    ClipboardManager.h
)

add_executable(Clipboard ${PROJECT_SOURCES})

target_link_libraries(Clipboard PRIVATE Qt6::Widgets)


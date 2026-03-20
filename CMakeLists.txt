cmake_minimum_required(VERSION 3.16)
project(Clipboard)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON) 

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

set(PROJECT_SOURCES main.cpp backend.cpp backend.h windoweffects.h)

qt_add_executable(Clipboard ${PROJECT_SOURCES})

if(WIN32)
    target_sources(Clipboard PRIVATE windoweffects_win.cpp)
    target_link_libraries(Clipboard PRIVATE dwmapi)
endif()

if(APPLE)
    enable_language(OBJCXX)
    target_sources(Clipboard PRIVATE windoweffects_mac.mm)
    target_link_libraries(Clipboard PRIVATE "-framework AppKit")
endif()

qt_add_qml_module(Clipboard
    URI Clipboard
    VERSION 1.0
    QML_FILES 
        main.qml
        Button.qml
)

target_link_libraries(Clipboard PRIVATE 
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick  Qt6::QuickControls2
)

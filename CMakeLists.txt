cmake_minimum_required(VERSION 3.16)
project(ClipboardProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON) 
set(CMAKE_AUTORCC ON)
set(QT_DEFAULT_MAJOR_VERSION 6) 

cmake_minimum_required(VERSION 3.16)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Network Widgets Concurrent Bluetooth)

if(NOT ANDROID) 
    add_subdirectory(3rdparty/QHotkey) 
endif()

set(PROJECT_SOURCES 
    main.cpp 
    backend/window_effects/WindowEffects.h
    backend/ip.cpp
    backend/socket.cpp
    backend/backend.h
    backend/NetworkManager.cpp
    backend/NetworkManager.h
    backend/server.h
    backend/ip.h
    backend/bluetooth.cpp
    backend/bluetooth_client.cpp
    backend/bluetooth_client.h
    backend/bluetooth.h
    backend/backend.h
    backend/backend.cpp
    backend/NetworkManager.h
    backend/NetworkManager.cpp
)

qt_add_executable(ClipboardApp MACOSX_BUNDLE ${PROJECT_SOURCES}
    resources.qrc
)

target_include_directories(ClipboardApp PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/window_effects
    ${CMAKE_CURRENT_SOURCE_DIR}/backend
)

if(ANDROID)
    set_property(TARGET ClipboardApp PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    target_sources(ClipboardApp PRIVATE backend/window_effects/windoweffects_android.cpp)
endif()

if(WIN32)
    target_sources(ClipboardApp PRIVATE backend/window_effects/windoweffects_win.cpp)
    target_link_libraries(ClipboardApp PRIVATE dwmapi)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_sources(ClipboardApp PRIVATE backend/window_effects/WindowEffects_linux.cpp)
endif()

if(APPLE)
    enable_language(OBJCXX)
    add_definitions(-DPLATFORM_OSX)
    target_sources(ClipboardApp PRIVATE 
        backend/window_effects/windoweffects_mac.mm
        backend/window_effects/QtLiquidGlass.mm 
        backend/backend.mm
    )
    
    target_link_libraries(ClipboardApp PRIVATE 
        "-framework AppKit" 
        "-framework Foundation"
        "-framework QuartzCore"
    )
    
    set_target_properties(ClipboardApp PROPERTIES
        MACOSX_BUNDLE_BUNDLE_IDENTIFIER "com.clipboard.app"
    )
endif()

qt_add_qml_module(ClipboardApp
    URI Clipboard
    VERSION 1.0
    NO_PLUGIN
    QML_FILES 
        ui/main.qml
        ui/components/MyButton.qml
        ui/components/TitleBar.qml
        ui/components/MobileBottomNav.qml
        ui/Settings.qml
        ui/History.qml
    SOURCES
        backend/backend.h 
        backend/backend.cpp
)

target_link_libraries(ClipboardApp PRIVATE 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Qml 
    Qt6::Quick  
    Qt6::QuickControls2
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
    Qt6::Bluetooth
)

if(NOT ANDROID) 
    target_link_libraries(ClipboardApp PRIVATE qhotkey) 
endif()
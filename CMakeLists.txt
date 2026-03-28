cmake_minimum_required(VERSION 3.16)
project(ClipboardProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON) 

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

set(PROJECT_SOURCES 
    main.cpp 
    backend/backend.cpp
    backend/backend.h  
    backend/window_effects/WindowEffects.h
)

qt_add_executable(ClipboardApp MACOSX_BUNDLE ${PROJECT_SOURCES})

target_include_directories(ClipboardApp PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/window_effects
    ${CMAKE_CURRENT_SOURCE_DIR}/backend
)

if(WIN32)
    target_sources(ClipboardApp PRIVATE backend/window_effects/windoweffects_win.cpp)
    target_link_libraries(ClipboardApp PRIVATE dwmapi)
endif()

if(APPLE)
    enable_language(OBJCXX)
    add_definitions(-DPLATFORM_OSX)
    target_sources(ClipboardApp PRIVATE 
        backend/window_effects/windoweffects_mac.mm
        backend/window_effects/QtLiquidGlass.mm 
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
        ui/Settings.qml
)

target_link_libraries(ClipboardApp PRIVATE 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Qml 
    Qt6::Quick  
    Qt6::QuickControls2
)
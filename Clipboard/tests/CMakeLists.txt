qt_add_executable(tst_bluetooth_startup
    tst_bluetooth_startup.cpp
    ../backend/ble/bluetooth_reconnect.cpp
    ../backend/ble/bluetooth_client.cpp
    ../backend/ble/bluetooth.cpp
    ../backend/ble/bluetooth_transfer.cpp
    ../backend/ble/switch.cpp
    ../backend/ble/socket.cpp
    ../backend/ble/server.h
    ../resources.qrc
)
target_include_directories(tst_bluetooth_startup PRIVATE ../backend ../backend/ble)
target_compile_definitions(tst_bluetooth_startup PRIVATE CLIPBOARD_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
target_link_libraries(tst_bluetooth_startup PRIVATE
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::Bluetooth Qt6::Network Qt6::Test
)
add_test(NAME bluetooth_startup COMMAND tst_bluetooth_startup)
set_tests_properties(bluetooth_startup PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_CONTROLS_STYLE=Basic"
)

qt_add_executable(tst_bluetooth_transfer
    tst_bluetooth_transfer.cpp
    ../backend/backend.cpp
    ../backend/clipboard_files.cpp
    ../backend/ble/bluetooth_transfer.cpp
    ../backend/ble/bluetooth_reconnect.cpp
    ../backend/ble/bluetooth_client.cpp
    ../backend/ble/bluetooth.cpp
    ../backend/ble/switch.cpp
    ../backend/ble/socket.cpp
    ../backend/ble/server.h
)
target_include_directories(tst_bluetooth_transfer PRIVATE ../backend ../backend/ble)
target_link_libraries(tst_bluetooth_transfer PRIVATE
    Qt6::Core Qt6::Gui Qt6::Quick Qt6::Bluetooth Qt6::Network Qt6::Test
)
if(APPLE)
    target_sources(tst_bluetooth_transfer PRIVATE ../backend/backend.mm)
    target_link_libraries(tst_bluetooth_transfer PRIVATE "-framework AppKit")
endif()
add_test(NAME bluetooth_transfer COMMAND tst_bluetooth_transfer)
set_tests_properties(bluetooth_transfer PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
set_target_properties(tst_bluetooth_startup tst_bluetooth_transfer PROPERTIES WIN32_EXECUTABLE FALSE)

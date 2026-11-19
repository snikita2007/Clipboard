# CMake generated Testfile for 
# Source directory: D:/clipboard/Clipboard/tests
# Build directory: D:/clipboard/build-msvc/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(bluetooth_startup "D:/clipboard/build-msvc/tests/tst_bluetooth_startup.exe")
set_tests_properties(bluetooth_startup PROPERTIES  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_CONTROLS_STYLE=Basic" _BACKTRACE_TRIPLES "D:/clipboard/Clipboard/tests/CMakeLists.txt;17;add_test;D:/clipboard/Clipboard/tests/CMakeLists.txt;0;")
add_test(bluetooth_transfer "D:/clipboard/build-msvc/tests/tst_bluetooth_transfer.exe")
set_tests_properties(bluetooth_transfer PROPERTIES  ENVIRONMENT "QT_QPA_PLATFORM=offscreen" _BACKTRACE_TRIPLES "D:/clipboard/Clipboard/tests/CMakeLists.txt;42;add_test;D:/clipboard/Clipboard/tests/CMakeLists.txt;0;")

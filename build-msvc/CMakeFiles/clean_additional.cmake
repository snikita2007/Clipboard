# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "3rdparty\\QHotkey\\CMakeFiles\\qhotkey_autogen.dir\\AutogenUsed.txt"
  "3rdparty\\QHotkey\\CMakeFiles\\qhotkey_autogen.dir\\ParseCache.txt"
  "3rdparty\\QHotkey\\qhotkey_autogen"
  "CMakeFiles\\ClipboardApp_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\ClipboardApp_autogen.dir\\ParseCache.txt"
  "ClipboardApp_autogen"
  "tests\\CMakeFiles\\tst_bluetooth_startup_autogen.dir\\AutogenUsed.txt"
  "tests\\CMakeFiles\\tst_bluetooth_startup_autogen.dir\\ParseCache.txt"
  "tests\\CMakeFiles\\tst_bluetooth_transfer_autogen.dir\\AutogenUsed.txt"
  "tests\\CMakeFiles\\tst_bluetooth_transfer_autogen.dir\\ParseCache.txt"
  "tests\\tst_bluetooth_startup_autogen"
  "tests\\tst_bluetooth_transfer_autogen"
  )
endif()

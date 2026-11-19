# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\ClipboardApp_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\ClipboardApp_autogen.dir\\ParseCache.txt"
  "ClipboardApp_autogen"
  )
endif()

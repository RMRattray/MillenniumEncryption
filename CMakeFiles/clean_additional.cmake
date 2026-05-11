# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "src\\app\\CMakeFiles\\millennium_serpent_autogen.dir\\AutogenUsed.txt"
  "src\\app\\CMakeFiles\\millennium_serpent_autogen.dir\\ParseCache.txt"
  "src\\app\\millennium_serpent_autogen"
  )
endif()

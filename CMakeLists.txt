cmake_minimum_required (VERSION 2.8.11)
project (UsdEmbree)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# cross platform
set(LIB_PREFIX "")
set(LIB_DYNAMIC_EXT "")
set(LIB_STATIC_EXT "")
if(UNIX)
  if(APPLE)
    set(LIB_PREFIX "lib")
    set(LIB_DYNAMIC_EXT ".dylib")
    set(LIB_STATIC_EXT ".a")
  else()

  endif()
else()

endif()
# usd 
set(USD_DIR "/Users/benmalartre/Documents/RnD/USD_BUILD")
set(USD_INCLUDE_DIR "${USD_DIR}/include")
set(USD_LIBRARY_DIR "${USD_DIR}/lib")
list(APPEND USD_LIB_NAMES 
  tf
  vt
  sdf
  sdr
  usd
  usdGeom
)

set(USD_LIBRARIES "")
foreach(USD_LIB_NAME IN LISTS USD_LIB_NAMES)
  set(USD_LIBRARIES 
    ${USD_LIBRARIES} 
      "${USD_LIBRARY_DIR}/${LIB_PREFIX}${USD_LIB_NAME}${LIB_DYNAMIC_EXT}"
  )

  message("${USD_LIBRARY_DIR}/
    ${LIB_PREFIX}${USD_LIB_NAME}${LIB_DYNAMIC_EXT}")
endforeach()

# python
find_package (Python2 COMPONENTS Interpreter Development)
message("PYTHON : ")
message("   ${Python2_INCLUDE_DIRS}")
message("   ${Python2_LIBRARY_DIRS}")

# boost
set(BOOST_INCLUDE_DIR "{USD_INCLUDE_DIR}")
set(BOOST_LIBRARY_DIR "{USD_LIBRARY_DIR}")
list(APPEND BOOST_LIB_NAMES 
  atomic
  date_time
  filesystem
  program_options
  python
  regex
  system
  thread
)

message(${BOOST_LIB_NAMES})
set(BOOST_LIBRARIES "")
foreach(BOOST_LIB_NAME IN LISTS BOOST_LIB_NAMES)
  set(BOOST_LIBRARIES 
    ${BOOST_LIBRARIES} 
      "${USD_LIBRARY_DIR}/${LIB_PREFIX}boost_${BOOST_LIB_NAME}${LIB_DYNAMIC_EXT}"
  )

  message("${USD_LIBRARY_DIR}/
    ${LIB_PREFIX}boost_${BOOST_LIB_NAME}${LIB_DYNAMIC_EXT}")
endforeach()

# embree

add_subdirectory (src)

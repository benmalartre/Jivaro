cmake_minimum_required (VERSION 3.0.0)
set(CMAKE_OSX_ARCHITECTURES arm64)
set(ONLY_ACTIVE_ARCH        YES)
project (jivaro)

# Require C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()


if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:inline-")
else()
  set(CMAKE_CXX_FLAGS "-Wall -Wextra")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

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
	set(LIB_PREFIX "")
    set(LIB_DYNAMIC_EXT ".lib")
    set(LIB_STATIC_EXT ".lib")
  add_definitions("/MP")
endif()

# nfd (native file dialog)
set(NFD_INCLUDE_DIR "./utils/nfd/include")

# glfw
if(NOT DEFINED GLFW_DIR)
	message(FATAL_ERROR "No GLFW_DIR variable set! GLFW installation path must be passed to cmake!!!")
endif()
set(GLFW_INCLUDE_DIR "${GLFW_DIR}/include")
find_library(GLFW NAMES glfw3 PATHS "${GLFW_DIR}/build/src" "${GLFW_DIR}/build/src/Release")

# usd 
if(NOT DEFINED USD_DIR)
	message(FATAL_ERROR "No USD_DIR variable set! USD installation path must be passed to cmake!!!")
endif()

# eigen
set(EIGEN_INCLUDE_DIR "/Users/malartrebenjamin/Documents/RnD/eigen-3.4.0")

#set(USD_DIR "/Users/benmalartre/Documents/RnD/USD_BUILD")
set(USD_INCLUDE_DIR "${USD_DIR}/include")
set(USD_LIBRARY_DIR "${USD_DIR}/lib")
list(APPEND USD_LIB_NAMES 
  trace
  work
  arch
  kind
  ar
  gf
  glf
  tf
  kind
  sdf
  exec
  plug
  garch
  ndr
  sdr
  sdf
  pcp
  usd
  hd
  hdSt
  hdx
  hgi
  hgiInterop
  hf
  hio
  usdGeom
  usdLux
  usdShade
  usdExec
  usdUI
  usdImaging
  usdImagingGL
  usdAnimX
  vt
  cameraUtil
  pxOsd
)

set(USD_LIBRARIES "")
foreach(USD_LIB_NAME IN LISTS USD_LIB_NAMES)
if(UNIX)
  set(USD_LIBRARIES 
    ${USD_LIBRARIES} 
      "${USD_LIBRARY_DIR}/${LIB_PREFIX}usd_${USD_LIB_NAME}${LIB_DYNAMIC_EXT}")
else()
  set(USD_LIBRARIES 
    ${USD_LIBRARIES} 
	  "${USD_LIBRARY_DIR}/${LIB_PREFIX}usd_${USD_LIB_NAME}${LIB_DYNAMIC_EXT}")
endif()

endforeach()

# python
find_package(Python3 COMPONENTS Interpreter Development)

# OpenGL
find_package(OpenGL REQUIRED)

# boost
if(UNIX)
  if(APPLE)
    set(BOOST_INCLUDE_DIR "{USD_INCLUDE_DIR}")
  else()

  endif()
else()
	set(BOOST_INCLUDE_DIR "${USD_INCLUDE_DIR}/boost-1_70")
endif()


set(BOOST_LIBRARY_DIR "{USD_LIBRARY_DIR}")
list(APPEND BOOST_LIB_NAMES 
  atomic
  #program_options
  regex
)

set(BOOST_LIBRARIES "")
set(BOOST_VERSION "")
if(NOT UNIX)
	set(BOOST_VERSION "-vc142-mt-x64-1_70")
endif()
foreach(BOOST_LIB_NAME IN LISTS BOOST_LIB_NAMES)
  set(BOOST_LIBRARIES 
    ${BOOST_LIBRARIES} 
      "${USD_LIBRARY_DIR}/${LIB_PREFIX}boost_${BOOST_LIB_NAME}${BOOST_VERSION}${LIB_DYNAMIC_EXT}"
  )
endforeach()

# tbb
set(TBB_INCLUDE_DIR "${USD_DIR}/include/tbb")
set(TBB_LIBRARY_DIR "${USD_DIR}/lib")

list(APPEND TBB_LIB_NAMES 
  tbb
  #tbbmalloc_proxy
  tbbmalloc
)

set(TBB_LIBRARIES "")
foreach(TBB_LIB_NAME IN LISTS TBB_LIB_NAMES)
  set(TBB_LIBRARIES 
    ${TBB_LIBRARIES} 
      "${TBB_LIBRARY_DIR}/${LIB_PREFIX}${TBB_LIB_NAME}${LIB_DYNAMIC_EXT}"
  )
endforeach()

# openvdb
# set(OPENVDB_INCLUDE_DIR "${USD_DIR}/include/openvdb")
# set(OPENVDB_LIBRARY_DIR "${USD_DIR}/lib")

#list(APPEND OPENVDB_LIB_NAMES 
#  openvdb
#)

set(OPENVDB_LIBRARIES "")
foreach(OPENVDB_LIB_NAME IN LISTS OPENVDB_LIB_NAMES)
  set(OPENVDB_LIBRARIES 
    ${OPENVDB_LIBRARIES} 
      "${OPENVDB_LIBRARY_DIR}/${LIB_PREFIX}${OPENVDB_LIB_NAME}${LIB_DYNAMIC_EXT}"
  )
endforeach()

# opensubdiv
set(OPENSUBDIV_INCLUDE_DIR "${USD_DIR}/include/opensubdiv")
set(OPENSUBDIV_LIBRARY_DIR "${USD_DIR}/lib")

list(APPEND OPENSUBDIV_LIB_NAMES 
  osdCPU
  osdGPU
)

set(OPENSUBDIV_LIBRARIES "")
foreach(OPENSUBDIV_LIB_NAME IN LISTS OPENSUBDIV_LIB_NAMES)
  set(OPENSUBDIV_LIBRARIES 
    ${OPENSUBDIV_LIBRARIES} 
      "${OPENSUBDIV_LIBRARY_DIR}/${LIB_PREFIX}${OPENSUBDIV_LIB_NAME}${LIB_DYNAMIC_EXT}"
  )
endforeach()


# system
IF(APPLE)
  SET(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} 
    "-framework Cocoa -framework IOKit -framework AppKit")
ENDIF(APPLE)

if(WIN32)
  add_definitions("/wd4244")
  add_definitions("/wd4305")
endif(WIN32)

add_subdirectory (src)
add_subdirectory (tests)
add_subdirectory (fonts)

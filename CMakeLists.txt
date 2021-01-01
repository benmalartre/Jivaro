cmake_minimum_required (VERSION 2.8.11)
project (AMINA)

# Require C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

#set(USD_DIR "/Users/benmalartre/Documents/RnD/USD_BUILD")
set(USD_INCLUDE_DIR "${USD_DIR}/include")
set(USD_LIBRARY_DIR "${USD_DIR}/lib")
list(APPEND USD_LIB_NAMES 
  trace
  work
  arch
  ar
  gf
  glf
  tf
  sdf
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
  hf
  hio
  usdGeom
  usdLux
  usdShade
  usdUI
  usdImaging
  usdImagingGL
  usdAnimX
  vt
)

set(USD_LIBRARIES "")
foreach(USD_LIB_NAME IN LISTS USD_LIB_NAMES)
  set(USD_LIBRARIES 
    ${USD_LIBRARIES} 
      "${USD_LIBRARY_DIR}/${LIB_PREFIX}${USD_LIB_NAME}${LIB_DYNAMIC_EXT}"
  )
endforeach()

# python
find_package(Python2 COMPONENTS Interpreter Development)

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
  program_options
  python
  regex
)

set(BOOST_LIBRARIES "")
set(BOOST_VERSION "")
if(NOT UNIX)
	set(BOOST_VERSION "-vc141-mt-x64-1_70")
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
  tbbmalloc_proxy
  tbbmalloc
)

set(TBB_LIBRARIES "")
foreach(TBB_LIB_NAME IN LISTS TBB_LIB_NAMES)
  set(TBB_LIBRARIES 
    ${TBB_LIBRARIES} 
      "${TBB_LIBRARY_DIR}/${LIB_PREFIX}${TBB_LIB_NAME}${LIB_DYNAMIC_EXT}"
  )
endforeach()

# OpenGL
find_package(OpenGL REQUIRED)

# GLFW_INCLUDE_DIR
#find_library(GLEW GLEW PATHS "${USD_LIBRARY_DIR}")

#
if(UNIX)
  if(APPLE)
    set(GLEW_LIBRARY "${USD_LIBRARY_DIR}/libGLEW.2.0.0.dylib")
  else()

  endif()
else()
	set(GLEW_LIBRARY "${USD_LIBRARY_DIR}/glew32.lib")
endif()

# system
IF(APPLE)
  SET(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} 
    "-framework Cocoa -framework IOKit")
ENDIF(APPLE)

add_subdirectory (src)
#add_subdirectory (tests)

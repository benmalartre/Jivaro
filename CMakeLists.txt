cmake_minimum_required (VERSION 2.8.11)
project (AMINA)

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
  work
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

set(BOOST_LIBRARIES "")
foreach(BOOST_LIB_NAME IN LISTS BOOST_LIB_NAMES)
  set(BOOST_LIBRARIES 
    ${BOOST_LIBRARIES} 
      "${USD_LIBRARY_DIR}/${LIB_PREFIX}boost_${BOOST_LIB_NAME}${LIB_DYNAMIC_EXT}"
  )
endforeach()

# embree
set(EMBREE_DIR "/Users/benmalartre/Documents/RnD/embree")
list(APPEND EMBREE_INCLUDE_DIRS 
  "${EMBREE_DIR}/embree/include"
  "${EMBREE_DIR}/embree/common"
  "${EMBREE_DIR}/embree/tutorials/common"
)
set(EMBREE_LIBRARY_DIR "${EMBREE_DIR}/build")
list(APPEND EMBREE_LIB_NAMES 
  sys
  math
  noise
  image
  #lexers
  #lights
  #scenegraph
  #simd
  tasking
  texture
  #tutorial
)

set(EMBREE_LIBRARIES "")
foreach(EMBREE_LIB_NAME IN LISTS EMBREE_LIB_NAMES)
  set(EMBREE_LIBRARIES 
    ${EMBREE_LIBRARIES} 
      "${EMBREE_LIBRARY_DIR}/${LIB_PREFIX}${EMBREE_LIB_NAME}${LIB_STATIC_EXT}"
  )
endforeach()

# jpeg
find_package(JPEG)
include_directories(${JPEG_INCLUDE_DIR})
set(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} ${JPEG_LIBRARIES})
set(ADDITIONAL_SOURCES ${ADDITIONAL_SOURCES} jpeg.cpp)

# png
find_package(PNG)
include_directories(${PNG_INCLUDE_DIR})
set(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} ${PNG_LIBRARIES})
set(ADDITIONAL_SOURCES ${ADDITIONAL_SOURCES} png.cpp)


set(EMBREE_LIBRARIES 
    ${EMBREE_LIBRARIES} 
      "${EMBREE_LIBRARY_DIR}/${LIB_PREFIX}embree3${LIB_DYNAMIC_EXT}"
)
message("EMBREE LIBRARY : ${EMBREE_LIBRARY_DIR}/${LIB_PREFIX}embree3${LIB_DYNAMIC_EXT}")

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

# gl3w 
set(GL3W_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src/gl3w/include")
message("GL3W DIRECTORY :  ${GL3W_INCLUDE_DIR}")

# glfw
set(GLFW_DIR "/Users/benmalartre/Documents/RnD/glfw")
set(GLFW_INCLUDE_DIR "${GLFW_DIR}/include")
find_library(GLFW glfw3 PATHS "${GLFW_DIR}/build/src/Release")


#target_link_libraries(${CMAKE_PROJECT_NAME}
#    ${GLFW}
#    )
#set(GLFW_LIBRARY_DIR "${GLFW_DIR}/build/src/Release")
#set(GLFW_LIBRARY "${GLFW_LIBRARY_DIR}/libglfw3.a")

# system
IF(APPLE)
  SET(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} 
    "-framework Cocoa -framework IOKit -framework CoreVideo")
ENDIF(APPLE)

add_subdirectory (src)

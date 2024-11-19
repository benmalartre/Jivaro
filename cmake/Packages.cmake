add_definitions(
#  -DTBB_USE_DEBUG
  -DNODEFAULTLIB  
)


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

# usd 
if(NOT DEFINED USD_DIR)
	message(FATAL_ERROR "No USD_DIR variable set! USD installation path must be passed to cmake!!!")
endif()

include(${USD_DIR}/pxrConfig.cmake)

# python
if(ENABLE_PYTHON_SUPPORT)
 message("LOOK FOR PYTHON 3 LIBRARIES")
  find_package(Python3 COMPONENTS Development)
endif()

# OpenGL
find_package(OpenGL REQUIRED)

# boost
if(UNIX)
  if(APPLE)
    set(BOOST_INCLUDE_DIR "{USD_INCLUDE_DIR}")
  else()

  endif()
else()
	set(BOOST_INCLUDE_DIR "${USD_INCLUDE_DIR}/boost-1_76")
endif()

set(BOOST_LIBRARY_DIR "{USD_LIBRARY_DIR}")
list(APPEND BOOST_LIB_NAMES 
  atomic
  regex
  system
)
#if(ENABLE_PYTHON_SUPPORT)
#  list(APPEND BOOST_LIB_NAMES python310)
#endif()

set(BOOST_LIBRARIES "")
set(BOOST_VERSION "")
if(NOT UNIX)
	set(BOOST_VERSION "-vc142-mt-x64-1_76")
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


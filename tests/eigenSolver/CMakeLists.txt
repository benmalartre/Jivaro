set(TARGET eigenSolver)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(Threads REQUIRED)

add_definitions(
  -DTASKING_TBB
  -DNOMINMAX
)

set(PUBLIC_HEADERS

)

add_executable(${TARGET}
  ../../src/geometry/matrix.cpp
  main.cpp
)

target_include_directories(${TARGET} 
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${USD_INCLUDE_DIR}
    ${EIGEN_INCLUDE_DIR}
    ${BOOST_INCLUDE_DIR}

)

target_link_libraries(${TARGET}
  ${USD_LIBRARIES}
  ${BOOST_LIBRARIES}
)

#if (APPLE)
#    set_target_properties(${TARGET} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Tests")
#    set_target_properties(${TARGET} PROPERTIES
#                          MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
#                          MACOSX_BUNDLE_LONG_VERSION_STRING "1.0.2343"
#                          MACOSX_BUNDLE_INFO_PLIST "/Users/benmalartre/Documents/RnD/glfw/CMake/Info.plist.in")
#endif()
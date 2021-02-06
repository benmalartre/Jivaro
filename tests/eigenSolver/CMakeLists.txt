set(TARGET eigenSolver)

find_package(Threads REQUIRED)

add_definitions(
  -DTASKING_TBB

)

set(PUBLIC_HEADERS

)

add_executable(${TARGET}
  main.cpp

)

target_include_directories(${TARGET} 
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${USD_INCLUDE_DIR}
    ${EIGEN_INCLUDE_DIR}

)

target_link_libraries(${TARGET}
  ${USD_LIBRARIES}
)

#if (APPLE)
#    set_target_properties(${TARGET} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Tests")
#    set_target_properties(${TARGET} PROPERTIES
#                          MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
#                          MACOSX_BUNDLE_LONG_VERSION_STRING "1.0.2343"
#                          MACOSX_BUNDLE_INFO_PLIST "/Users/benmalartre/Documents/RnD/glfw/CMake/Info.plist.in")
#endif()
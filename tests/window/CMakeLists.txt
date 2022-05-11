set(TARGET tests)

find_package(Threads REQUIRED)

add_definitions(
  -DTASKING_TBB

)

set(PUBLIC_HEADERS

)

add_executable(${TARGET}
  view.cpp
  window.cpp
  main.cpp

)

target_include_directories(${TARGET} 
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${USD_INCLUDE_DIR}
    ${Python2_INCLUDE_DIRS}
    ${EMBREE_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIR}
    ${TBB_INCLUDE_DIR}

)

target_link_libraries(${TARGET}
  ${OPENGL_LIBRARY}
#  ${Python2_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${USD_LIBRARIES}
  ${EMBREE_LIBRARIES}
  ${TBB_LIBRARIES}
  ${GLFW}
  ${ADDITIONAL_LIBRARIES}
  
)

#if (APPLE)
#    set_target_properties(${TARGET} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Tests")
#    set_target_properties(${TARGET} PROPERTIES
#                          MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
#                          MACOSX_BUNDLE_LONG_VERSION_STRING "1.0.2343"
#                          MACOSX_BUNDLE_INFO_PLIST "/Users/benmalartre/Documents/RnD/glfw/CMake/Info.plist.in")
#endif()
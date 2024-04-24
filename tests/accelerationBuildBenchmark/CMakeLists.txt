set(TARGET accelerationBuildBenchmark)


add_definitions(
  -DTASKING_TBB
  -DNOMINMAX
)

set(PUBLIC_HEADERS

)

add_executable(${TARGET}
  ../../src/utils/timer.cpp
  ../../src/acceleration/intersector.h>
  ../../src/acceleration/bvh.cpp
  ../../src/acceleration/grid3d.cpp
  ../../src/acceleration/octree.cpp
  ../../src/geometry/utils.cpp
  ../../src/geometry/point.cpp
  ../../src/geometry/triangle.cpp
  ../../src/geometry/halfEdge.cpp
  ../../src/geometry/geometry.cpp
  ../../src/geometry/deformable.cpp
  ../../src/geometry/implicit.cpp
  ../../src/geometry/points.cpp
  ../../src/geometry/mesh.cpp
  ../../src/geometry/curve.cpp
  main.cpp
)

target_include_directories(${TARGET} 
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${USD_INCLUDE_DIR}
    ${BOOST_INCLUDE_DIR}
    ${TBB_INCLUDE_DIR}

)

target_link_libraries(${TARGET}
  ${USD_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
)

#if (APPLE)
#    set_target_properties(${TARGET} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Tests")
#    set_target_properties(${TARGET} PROPERTIES
#                          MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
#                          MACOSX_BUNDLE_LONG_VERSION_STRING "1.0.2343"
#                          MACOSX_BUNDLE_INFO_PLIST "/Users/benmalartre/Documents/RnD/glfw/CMake/Info.plist.in")
#endif()
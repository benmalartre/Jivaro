set(TARGET jivaro)

# custom usd plugins
set(USD_PLUGINS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/plugins")
set(USD_PLUGINS_LIBRARY_DIR "${CMAKE_BINARY_DIR}/plugins")

list(APPEND USD_PLUGINS_LIB_NAMES 
  usdPbd
)

set(USD_PLUGINS_LIBRARIES "")
foreach(USD_PLUGINS_LIB_NAME IN LISTS USD_PLUGINS_LIB_NAMES)
  set(USD_PLUGINS_LIBRARIES 
    ${USD_PLUGINS_LIBRARIES} 
      "${USD_PLUGINS_LIBRARY_DIR}/${USD_PLUGINS_LIB_NAME}/${LIB_PREFIX}${USD_PLUGINS_LIB_NAME}${LIB_DYNAMIC_EXT}"
  )
endforeach()

set(USDIMAGINGGL_ENGINE_ENABLE_SCENE_INDEX true)

  add_definitions(
    -DTASKING_TBB
    -DNOMINMAX
  #  -DTBB_USE_DEBUG
  )


add_executable(${TARGET}
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_impl_glfw.cpp
  imgui/imgui_impl_opengl3.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_stdlib.cpp
  imgui/imgui.cpp

  utils/files.cpp
  utils/shaders.cpp
  utils/icons.cpp
  utils/prefs.cpp
  utils/timer.cpp
  utils/usd.cpp

  ui/ui.cpp
  ui/fonts.cpp
  ui/demo.cpp
  ui/icon.cpp
  ui/textFilter.cpp
  ui/utils.cpp
  ui/popup.cpp
  ui/debug.cpp
  ui/fileBrowser.cpp
  ui/contentBrowser.cpp
  ui/splitter.cpp
  ui/tab.cpp
  ui/menu.cpp
  ui/commands.cpp
  ui/graphEditor.cpp
  ui/viewport.cpp
  ui/timeline.cpp
  ui/toolbar.cpp
  ui/explorer.cpp
  ui/propertyEditor.cpp
  ui/attributeEditor.cpp
  ui/curveEditor.cpp

  graph/graph.cpp
  graph/execution.cpp
  graph/hierarchy.cpp

  exec/sceneIndex.cpp
  exec/execution.cpp
  exec/menu.cpp
  #exec/node.cpp 
  #exec/network.cpp 

  #embree/oiio.cpp
  #embree/jpeg.cpp
  #embree/utils.cpp
  #embree/sphere.cpp
  #embree/mesh.cpp
  #embree/subdiv.cpp
  #embree/instance.cpp
  #embree/context.cpp
  #embree/device.cpp

  geometry/utils.cpp
  geometry/matrix.cpp
  geometry/rbf.cpp
  geometry/decomposition.cpp
  geometry/subdiv.cpp
  geometry/tesselator.cpp
  geometry/kabsch.cpp
  geometry/sampler.cpp
  geometry/voxels.cpp
  geometry/shape.cpp
  geometry/location.cpp
  geometry/intersection.cpp
  geometry/point.cpp
  geometry/edge.cpp
  geometry/halfEdge.cpp
  geometry/triangle.cpp
  geometry/delaunay.cpp
  geometry/geometry.cpp
  geometry/implicit.cpp
  geometry/deformable.cpp
  geometry/points.cpp
  geometry/mesh.cpp
  geometry/curve.cpp
  geometry/instancer.cpp
  geometry/stroke.cpp
  geometry/perlin.cpp
  geometry/voronoi.cpp
  geometry/geodesic.cpp
  geometry/scene.cpp
  #geometry/vdb.cpp

  acceleration/morton.cpp
  acceleration/distance.cpp
  acceleration/intersector.cpp
  acceleration/grid3d.cpp
  acceleration/bvh.cpp
  acceleration/octree.cpp
  acceleration/hashGrid.cpp
  acceleration/kdtree.cpp

  pbd/contact.cpp
  pbd/collision.cpp
  pbd/mask.cpp
  pbd/particle.cpp
  pbd/force.cpp
  pbd/constraint.cpp
  pbd/solver.cpp
  pbd/menu.cpp

  command/inverse.cpp
  command/block.cpp
  command/router.cpp
  command/delegate.cpp
  command/manager.cpp

  app/time.cpp
  app/utils.cpp
  app/commands.cpp
  app/callbacks.cpp
  app/notice.cpp
  app/handle.cpp
  app/camera.cpp
  app/model.cpp
  app/index.cpp
  app/view.cpp
  app/layout.cpp
  app/window.cpp
  app/modal.cpp
  app/engine.cpp
  app/picking.cpp
  app/selection.cpp
  app/tools.cpp
  app/registry.cpp
  app/application.cpp

  #tests/location.cpp
  tests/utils.cpp
  tests/pbd.cpp
  tests/hair.cpp
  tests/particles.cpp
  tests/raycast.cpp
  tests/bvh.cpp
  tests/instancer.cpp
  tests/grid.cpp
  tests/points.cpp
  tests/velocity.cpp
  tests/pendulum.cpp
  tests/geodesic.cpp
  tests/push.cpp
  #tests/stageGraph.cpp
  #tests/stageUI.cpp
  main.cpp
)

# Application icon on windows
if(MSVC)
    target_sources(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/resources.rc)
endif()

target_include_directories(${TARGET} 
  PUBLIC
    ${USD_INCLUDE_DIR}
    ${USD_PLUGINS_INCLUDE_DIR}
    ${EMBREE_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIR}
    ${TBB_INCLUDE_DIR}
    ${OPENVDB_INCLUDE_DIR}
    ${OPENSUBDIV_INCLUDE_DIR}
    ${BOOST_INCLUDE_DIR}
    ${ICON_AWESOME_DIR}
    ${EIGEN_INCLUDE_DIR}
)

if(PYTHON_BUILD)
  target_include_directories(${TARGET}  PUBLIC ${Python3_INCLUDE_DIRS})
endif()

target_link_libraries(${TARGET}
  ${OPENGL_LIBRARY}
  ${BOOST_LIBRARIES}
  ${USD_LIBRARIES}
  ${USD_PLUGINS_LIBRARIES}
  ${EMBREE_LIBRARIES}
  ${TBB_LIBRARIES}
  ${OPENVDB_LIBRARIES}
  ${OPENSUBDIV_LIBRARIES}
  ${GLFW}
  ${ADDITIONAL_LIBRARIES}
)

if(PYTHON_BUILD)
  target_link_libraries(${TARGET} ${Python3_LIBRARIES})
endif()

add_compile_options(/C4244)

install(
  TARGETS ${TARGET}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)

install(
  DIRECTORY ../fonts ../icons DESTINATION bin
)

install(
  FILES ../plugins/plugInfo.json DESTINATION lib/usd
)

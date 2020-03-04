set(TARGET amnesia)

add_definitions(
  -DIMGUI_IMPL_OPENGL_LOADER_GL3W
  -DTASKING_TBB

)

set(PUBLIC_HEADERS
  graph/composerBindingAPI.h
  graph/connectableAPI.h
  graph/coordSysAPI.h

)

add_executable(${TARGET}
  gl3w/src/gl3w.c
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_test.cpp
  imgui/imgui_impl_glfw.cpp
  imgui/imgui_impl_opengl3.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui.cpp
  graph/deformableAPI.cpp
  graph/connectableAPI.cpp
  graph/coordSysAPI.cpp
  graph/input.cpp
  graph/output.cpp
  graph/node.cpp
  graph/graph.cpp
  graph/deformable.cpp
  graph/layer.cpp
  graph/stage.cpp
  graph/tokens.cpp
  graph/utils.cpp
  widgets/menu.cpp
  widgets/node.cpp
  widgets/graph.cpp
  widgets/viewport.cpp
  embree/oiio.cpp
  embree/jpeg.cpp
  embree/utils.cpp
  embree/sphere.cpp
  embree/mesh.cpp
  embree/context.cpp
  embree/device.cpp
  utils/glutils.cpp
  utils/utils.cpp
  app/view.cpp
  app/ui.cpp
  app/splitter.cpp
  app/window.cpp
  app/application.cpp
  tests/stageGraph.cpp
  tests/stageUI.cpp
  main.cpp
)

target_include_directories(${TARGET} 
  PUBLIC
    ${USD_INCLUDE_DIR}
    ${Python2_INCLUDE_DIRS}
    ${EMBREE_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIR}
    ${GL3W_INCLUDE_DIR}
    ${TBB_INCLUDE_DIR}
)

target_link_libraries(${TARGET}
  ${OPENGL_gl_LIBRARY}
  ${Python2_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${USD_LIBRARIES}
  ${EMBREE_LIBRARIES}
  ${TBB_LIBRARIES}
  ${GLFW}
  ${ADDITIONAL_LIBRARIES}
)
set(TARGET amina)

add_definitions(
  -DIMGUI_IMPL_OPENGL_LOADER_GL3W
  -DTASKING_TBB

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
  oiio.cpp
  jpeg.cpp
  mesh.cpp
  device.cpp
  utils.cpp
  view.cpp
  ui.cpp
  widgets/dummy.cpp
  widgets/viewport.cpp
  splitter.cpp
  window.cpp
  application.cpp
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
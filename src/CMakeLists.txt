set(TARGET amnesia)

add_definitions(
  -DIMGUI_IMPL_OPENGL_LOADER_GLEW
  -DTASKING_TBB
  -DNOMINMAX
  -DGRAPH_EXPORTS=1
)

set(PUBLIC_HEADERS
  graph/deformableAPI.h
  graph/connectableAPI.h
  graph/coordSysAPI.h

)

set(NATIVEFOLDERDIALOG_IMP "")
if(UNIX)
	if(APPLE)
		set(NATIVEFOLDERDIALOG_IMP "utils/nfd_cocoa.cpp")
	else()
		set(NATIVEFOLDERDIALOG_IMP "utils/nfd_gtk.cpp")
	endif()
else()	
	set(NATIVEFOLDERDIALOG_IMP "utils/nfd_win.cpp")
endif()
add_executable(${TARGET}
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_impl_glfw.cpp
  imgui/imgui_impl_opengl3.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui.cpp
  utils/files.cpp
  utils/shaders.cpp
  utils/screenSpaceQuad.cpp
  utils/icons.cpp

  utils/nfd_win.cpp
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
  widgets/dummy.cpp
  widgets/menu.cpp
  widgets/node.cpp
  widgets/graph.cpp
  widgets/viewport.cpp
  widgets/timeline.cpp
  widgets/toolbar.cpp
  widgets/explorer.cpp
  widgets/property.cpp
  widgets/curveEditor.cpp
  #embree/oiio.cpp
  #embree/jpeg.cpp
  #embree/utils.cpp
  #embree/sphere.cpp
  #embree/mesh.cpp
  #embree/subdiv.cpp
  #embree/instance.cpp
  #embree/context.cpp
  #embree/device.cpp
  app/camera.cpp
  app/view.cpp
  app/ui.cpp
  app/splitter.cpp
  app/window.cpp
  app/engine.cpp
  app/notice.cpp
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
    ${TBB_INCLUDE_DIR}
	${BOOST_INCLUDE_DIR}
)

target_link_libraries(${TARGET}
  ${OPENGL_LIBRARY}
  ${Python2_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${USD_LIBRARIES}
  ${EMBREE_LIBRARIES}
  ${TBB_LIBRARIES}
  ${GLFW}
  ${GLEW_LIBRARY}
  ${ADDITIONAL_LIBRARIES}
)

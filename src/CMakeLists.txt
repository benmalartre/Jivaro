set(TARGET amnesia)

add_definitions(
  -DTASKING_TBB
  -DNOMINMAX
  -DGRAPH_EXPORTS=1
  -DIMGUI_IMPL_OPENGL_LOADER_CUSTOM="${USD_INCLUDE_DIR}/pxr/imaging/garch/glApi.h"
)

set(PUBLIC_HEADERS
  graph/deformableAPI.h
  graph/connectableAPI.h
  graph/coordSysAPI.h

)

set(NATIVEFOLDERDIALOG_IMP "")
if(UNIX)
	if(APPLE)
		set(NATIVEFOLDERDIALOG_IMP "utils/nfd/nfd_cocoa.m")
	else()
		set(NATIVEFOLDERDIALOG_IMP "utils/nfd/nfd_gtk.cpp")
	endif()
else()	
	set(NATIVEFOLDERDIALOG_IMP "utils/nfd/nfd_win.cpp")
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
  utils/native.c
  ui/ui.cpp
  ui/filebrowser.cpp
  ui/splitter.cpp
  ui/dummy.cpp
  ui/menu.cpp
  ui/node.cpp
  ui/graph.cpp
  ui/viewport.cpp
  ui/timeline.cpp
  ui/toolbar.cpp
  ui/explorer.cpp
  ui/property.cpp
  ui/curveEditor.cpp
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
  geometry/shape.cpp
  geometry/intersector.cpp
  geometry/triangle.cpp
  geometry/mesh.cpp
  pbd/integration.cpp
  pbd/maths.cpp
  app/time.cpp
  app/handle.cpp
  app/camera.cpp
  app/view.cpp
  app/window.cpp
  app/modal.cpp
  app/engine.cpp
  app/notice.cpp
  app/picking.cpp
  app/selection.cpp
  app/tools.cpp
  app/application.cpp
  #tests/stageGraph.cpp
  #tests/stageUI.cpp
  main.cpp
)

target_include_directories(${TARGET} 
  PUBLIC
    ${NFD_INCLUDE_DIR}
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
  #${GLEW_LIBRARY}
  ${ADDITIONAL_LIBRARIES}
)

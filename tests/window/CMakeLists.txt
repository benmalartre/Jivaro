set(TARGET window)

find_package(Threads REQUIRED)

add_definitions(
  -DTASKING_TBB
  -DNOMINMAX
  -DTBB_USE_DEBUG
)

set(PUBLIC_HEADERS

)

add_executable(${TARGET}
  ../../src/imgui/imgui_draw.cpp
  ../../src/imgui/imgui_impl_glfw.cpp
  ../../src/imgui/imgui_impl_opengl3.cpp
  ../../src/imgui/imgui_widgets.cpp
  ../../src/imgui/imgui_tables.cpp
  ../../src/imgui/imgui_stdlib.cpp
  ../../src/imgui/imgui.cpp
  ../../src/ui/tab.cpp
  ../../src/ui/viewport.cpp
  ../../src/ui/contentBrowser.cpp
  ../../src/ui/graphEditor.cpp
  ../../src/ui/propertyEditor.cpp
  ../../src/ui/attributeEditor.cpp
  ../../src/app/view.cpp
  ../../src/app/window.cpp
  ../../src/app/time.cpp
  ../../src/app/application.cpp
  main.cpp

)

target_include_directories(${TARGET} 
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${USD_INCLUDE_DIR}
    ${EIGEN_INCLUDE_DIR}
    ${BOOST_INCLUDE_DIR}
    ${EMBREE_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIR}
    ${TBB_INCLUDE_DIR}

)

if(PYTHON_BUILD)
  target_include_directories(${TARGET}  PUBLIC ${Python3_INCLUDE_DIRS})
endif()

target_link_libraries(${TARGET}
  ${OPENGL_LIBRARY}
  ${BOOST_LIBRARIES}
  ${USD_LIBRARIES}
  ${EMBREE_LIBRARIES}
  ${TBB_LIBRARIES}
  ${GLFW}
  ${ADDITIONAL_LIBRARIES}
  
)

if(PYTHON_BUILD)
  target_link_libraries(${TARGET} ${Python3_LIBRARIES})
endif()

#if (APPLE)
#    set_target_properties(${TARGET} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Tests")
#    set_target_properties(${TARGET} PROPERTIES
#                          MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
#                          MACOSX_BUNDLE_LONG_VERSION_STRING "1.0.2343"
#                          MACOSX_BUNDLE_INFO_PLIST "/Users/benmalartre/Documents/RnD/glfw/CMake/Info.plist.in")
#endif()
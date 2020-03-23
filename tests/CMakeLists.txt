set(TARGET tests)

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
    ${USD_INCLUDE_DIR}
    ${Python2_INCLUDE_DIRS}
    ${EMBREE_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIR}
    ${GL3W_INCLUDE_DIR}
    ${TBB_INCLUDE_DIR}

)

target_link_libraries(${TARGET}
  ${OPENGL_LIBRARY}
  ${Python2_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${USD_LIBRARIES}
  ${EMBREE_LIBRARIES}
  ${TBB_LIBRARIES}
  ${GLFW}
  ${ADDITIONAL_LIBRARIES}
  
)
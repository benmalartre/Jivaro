add_executable(hello
  hello_world.cpp
)

target_include_directories(hello 
  PUBLIC
    ${USD_INCLUDE_DIR}
    ${Python2_INCLUDE_DIRS}
    ${EMBREE_INCLUDE_DIR}
)

target_link_libraries(hello
  ${Python2_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${USD_LIBRARIES}
  ${EMBREE_LIBRARIES}
)
set(TARGET test)

if(WIN32)
    set(WINLIBS Shlwapi.lib)
endif()

add_executable(${TARGET}
  main.cpp

)

target_include_directories(${TARGET} 
  PUBLIC
	"C:/Users/graph/Documents/bmal/src/Amnesie/tests/type"
    ${USD_INCLUDE_DIR}
    ${Python2_INCLUDE_DIRS}
    ${TBB_INCLUDE_DIR}
	${BOOST_INCLUDE_DIR}
)

target_link_libraries(${TARGET}
  "C:/Users/graph/Documents/bmal/src/Amnesie/build/tests/type/Release/testType.lib"
  ${WINLIBS}
  ${Python2_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${USD_LIBRARIES}
  ${ADDITIONAL_LIBRARIES}
  ${TBB_LIBRARIES}
)


#if (APPLE)
#    set_target_properties(${TARGET} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Tests")
#    set_target_properties(${TARGET} PROPERTIES
#                          MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
#                          MACOSX_BUNDLE_LONG_VERSION_STRING "1.0.2343"
#                          MACOSX_BUNDLE_INFO_PLIST "/Users/benmalartre/Documents/RnD/glfw/CMake/Info.plist.in")
#endif()
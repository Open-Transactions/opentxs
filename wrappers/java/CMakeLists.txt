## Work in progress
##
message("** Java Wrapper")

set(OTX_WRAPPER_NAME "JOpentxs")

set(CMAKE_SWIG_FLAGS -package org.opentransactions.otapi)

if(CMAKE_CROSSCOMPILING)
  set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
  set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER )
  find_package(SWIG 3.0.0 REQUIRED)
  set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY )
  set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
else()
  find_package(SWIG 3.0.0 REQUIRED)
endif()


include(${SWIG_USE_FILE})

include_directories(
  ${SWIG_INCLUDE_DIRS}
)

find_package(JNI)

include_directories(
  ${JAVA_INCLUDE_PATH})

## Java sources: place all our java code in subdir to keep it clean
set(JAVA_SOURCE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/otx_java CACHE INTERNAL "")
set(JAVA_BINARY_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/otx_java CACHE INTERNAL "")
file(MAKE_DIRECTORY ${JAVA_SOURCE_DIRECTORY})
file(MAKE_DIRECTORY ${JAVA_BINARY_DIRECTORY})

set(CMAKE_SWIG_OUTDIR ${JAVA_BINARY_DIRECTORY})

set_source_files_properties(OTX_.i PROPERTIES CPLUSPLUS ON)
swig_add_module(${OTX_WRAPPER_NAME} java OTX_.i)
swig_link_libraries(${OTX_WRAPPER_NAME} ${JAVA_LIBRARIES})

set(MOD_NAME ${SWIG_MODULE_${OTX_WRAPPER_NAME}_REAL_NAME})

target_link_libraries(${MOD_NAME} ${OTX_LINK_LIBS})
set_target_properties(${MOD_NAME}
                      PROPERTIES NO_SONAME ON OUTPUT_NAME ${OTX_WRAPPER_NAME})
set_lib_property(${MOD_NAME})
install(TARGETS ${MOD_NAME}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT java)


# Copyright (c) Monetas AG, 2014

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PEDANTIC_CXX_FLAGS}")

set(dependency_include_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/../deps/
)

include_directories(SYSTEM
  ${dependency_include_dir}
  ${dependency_include_dir}/ChaiScript/include
  ${dependency_include_dir}/lucre/include
)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

if(KEYRING_GNOME)
  set(keyring libgnome-keyring.so)
endif()

add_subdirectory(storage)
add_subdirectory(core)
add_subdirectory(cash)
add_subdirectory(ext)
add_subdirectory(network)
add_subdirectory(client)
add_subdirectory(server)

set(object-deps
  $<TARGET_OBJECTS:opentxs-client>
  $<TARGET_OBJECTS:opentxs-server>
)

set(MODULE_NAME opentxs)

add_library(${MODULE_NAME} SHARED
  ${object-deps}
)

add_library(${MODULE_NAME}_static STATIC
  ${object-deps}
)

if(NOT WIN32)
  set_target_properties(${MODULE_NAME}_static PROPERTIES OUTPUT_NAME ${MODULE_NAME})
endif()

target_link_libraries(${MODULE_NAME}
  PUBLIC
  opentxs-core
)

if(WIN32)
  add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/${MODULE_NAME}.dll"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/${MODULE_NAME}.dll")

  add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/${MODULE_NAME}.pdb"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/${MODULE_NAME}.pdb")
endif()

if(NOT WIN32)
  include(GNUInstallDirs)
  install(TARGETS ${MODULE_NAME} ${MODULE_NAME}_static
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          EXPORT opentxsTargets
          COMPONENT main)
endif()

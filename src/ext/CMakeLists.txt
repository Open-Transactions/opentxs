# Copyright (c) Monetas AG, 2014

set(cxx-sources
  OTSocket.cpp
  Socket_ZMQ4.cpp
  OTPayment.cpp
)

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/ext/*.hpp")

include_directories(SYSTEM
  ${ZMQ_INCLUDE_DIR}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/ext
)

set(MODULE_NAME opentxs-ext)
if(WIN32)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/windll.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
    @ONLY
  )

  # we have an extra Windows-specific file to compile
  add_library(${MODULE_NAME}
    ${cxx-sources}
    ${cxx-headers}
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
  )
else()
  add_library(${MODULE_NAME}
    ${cxx-sources}
    ${cxx-headers}
  )
endif()

target_link_libraries(opentxs-ext opentxs-cash opentxs-core ${ZMQ_LIBRARY})
set_target_properties(opentxs-ext PROPERTIES VERSION ${VERSION_STRING} SOVERSION ${VERSION_MAJOR})

if(WIN32)
  add_custom_command(TARGET opentxs-ext POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/opentxs-ext.dll"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/opentxs-ext.dll")

  add_custom_command(TARGET opentxs-ext POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/opentxs-ext.pdb"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/opentxs-ext.pdb")
endif()

if(NOT WIN32)
  include(GNUInstallDirs)
  install(TARGETS opentxs-ext
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          EXPORT opentxsTargets
          COMPONENT main)
endif()

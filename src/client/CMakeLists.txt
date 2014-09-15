# Copyright (c) Monetas AG, 2014

set(cxx-sources
  Helpers.cpp
  TransportCallback.cpp
  OTMessageBuffer.cpp
  OTMessageOutbuffer.cpp
  OpenTransactions.cpp
  ot_commands_ot.cpp
  ot_made_easy_ot.cpp
  OT_ME.cpp
  ot_otapi_ot.cpp
  ot_utility_ot.cpp
  OTAPI.cpp
  OTAPI_Exec.cpp
  OTClient.cpp
  OTMeCpp.cpp
  OTRecord.cpp
  OTRecordList.cpp
  OTServerConnection.cpp
  OTWallet.cpp
)

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/*.hpp")

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client
)

set(MODULE_NAME opentxs-client)
if(WIN32)
  # suppress "decorated name length exceeded, name was truncated" warnings 
  add_definitions("/wd4503")

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

target_link_libraries(opentxs-client opentxs-ext opentxs-cash opentxs-basket opentxs-core)
set_target_properties(opentxs-client PROPERTIES VERSION ${VERSION_STRING} SOVERSION ${VERSION_MAJOR})

if(WIN32)
  add_custom_command(TARGET opentxs-client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/opentxs-client.dll"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/opentxs-client.dll")

  add_custom_command(TARGET opentxs-client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/opentxs-client.pdb"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/opentxs-client.pdb")
endif()

if(NOT WIN32)
  include(GNUInstallDirs)
  install(TARGETS opentxs-client
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          EXPORT opentxsTargets
          COMPONENT main)
endif()


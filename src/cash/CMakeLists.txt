# Copyright (c) Monetas AG, 2014

set(cxx-sources
  Mint.cpp
  MintLucre.cpp
  DigitalCash.cpp
  Purse.cpp
  Token.cpp
  TokenLucre.cpp
)

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/cash/*.hpp")

set(dependency_include_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/
)

include_directories(SYSTEM
  ${dependency_include_dir}
  ${dependency_include_dir}/lucre/include/
)

include_directories(SYSTEM
  ${OPENSSL_INCLUDE_DIR}
)

set(MODULE_NAME opentxs-cash)
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

target_link_libraries(opentxs-cash opentxs-core lucre ${OPENSSL_LIBRARIES})
set_target_properties(opentxs-cash PROPERTIES VERSION "${VERSION_STRING}" SOVERSION "${VERSION_MAJOR}")

if(WIN32)
  add_custom_command(TARGET opentxs-cash POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/opentxs-cash.dll"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/opentxs-cash.dll")

  add_custom_command(TARGET opentxs-cash POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/opentxs-cash.pdb"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/opentxs-cash.pdb")
endif()

if(NOT WIN32)
  include(GNUInstallDirs)
  install(TARGETS opentxs-cash
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          EXPORT opentxsTargets
          COMPONENT main)
endif()

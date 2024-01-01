# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(ottest-basic OBJECT "")
add_library(ottest-lowlevel OBJECT "")
libopentxs_configure_cxx_target(ottest-basic)
libopentxs_configure_cxx_target(ottest-lowlevel)
target_link_libraries(opentxs-testlib PUBLIC opentxs-protobuf)

if(OT_PCH)
  add_library(ottest-pch OBJECT "pch.cpp")
  libopentxs_configure_cxx_target(ottest-pch)
  target_link_libraries(ottest-pch PUBLIC opentxs-testlib)
  set_target_properties(
    ottest-pch PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests
                          UNITY_BUILD OFF
  )
  target_precompile_headers(
    ottest-pch
    PUBLIC
    <gmock/gmock.h>
    <gtest/gtest.h>
    <opentxs/opentxs.hpp>
  )
endif()

if(WIN32)
  add_library(ottest-deps SHARED "pch.cpp")
  libopentxs_configure_cxx_target(ottest-deps)
  target_link_libraries(ottest-deps PUBLIC opentxs::libopentxs opentxs-testlib)

  add_custom_command(
    TARGET ottest-deps
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:ottest-deps>
            $<TARGET_FILE_DIR:ottest-deps>
    COMMAND_EXPAND_LISTS
  )
endif()

function(
  add_opentx_test_target
  target_name
  cxx-sources
)
  add_executable(${target_name} ${cxx-sources})
  libopentxs_configure_cxx_target(${target_name})
  target_link_libraries(${target_name} PUBLIC opentxs-testlib)
  set_target_properties(
    ${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                              ${PROJECT_BINARY_DIR}/tests UNITY_BUILD OFF
  )

  if(WIN32)
    set_target_properties(${target_name} PROPERTIES LINK_OPTIONS /STACK:2097152)
  endif()

  if(OT_PCH)
    target_precompile_headers(
      ${target_name}
      REUSE_FROM
      ottest-pch
    )
  endif()

  if(NOT MSVC)
    target_compile_options(
      ${target_name} PRIVATE -Wno-reserved-macro-identifier
    )
  endif()

  add_test(
    ${target_name}
    ${PROJECT_BINARY_DIR}/tests/${target_name}
    --gtest_output=xml:gtestresults.xml
  )
endfunction()

function(
  add_opentx_test
  target_name
  file_name
)
  set(cxx-sources "${file_name}" $<TARGET_OBJECTS:ottest-basic>)
  add_opentx_test_target("${target_name}" "${cxx-sources}")
endfunction()

function(
  add_opentx_low_level_test
  target_name
  file_name
)
  set(cxx-sources "${file_name}" $<TARGET_OBJECTS:ottest-lowlevel>)
  add_opentx_test_target("${target_name}" "${cxx-sources}")
endfunction()

add_subdirectory(ottest)

add_subdirectory(dummy)
add_subdirectory(integration)
add_subdirectory(uncategorized)
add_subdirectory(unit)

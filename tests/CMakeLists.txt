# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

function(
  add_opentx_test_target
  target_name
  cxx-sources
)
  include_directories(
    ${opentxs_SOURCE_DIR}/include
    ${opentxs_SOURCE_DIR}/tests
    ${GTEST_INCLUDE_DIRS}
  )

  add_executable(
    ${target_name} ${cxx-sources} $<TARGET_OBJECTS:opentxs-test-helpers>
  )
  target_sources(
    ${target_name} PRIVATE "${opentxs_SOURCE_DIR}/tests/OTTestEnvironment.cpp"
  )
  target_include_directories(
    ${target_name} SYSTEM
    PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
  )

  if(OT_QT_EXPORT)
    target_sources(${target_name} PRIVATE "${opentxs_SOURCE_DIR}/tests/Qt.cpp")
    target_link_libraries(${target_name} PRIVATE Qt::Core)
  else()
    target_sources(
      ${target_name} PRIVATE "${opentxs_SOURCE_DIR}/tests/no-Qt.cpp"
    )
  endif()

  target_link_libraries(
    ${target_name}
    PRIVATE
      opentxs::libopentxs
      Boost::program_options
      opentxs-test-helpers
      GTest::gmock
  )

  target_include_directories(
    ${target_name} SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
  )
  set_target_properties(
    ${target_name}
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests
               POSITION_INDEPENDENT_CODE 1
  )

  if(${CMAKE_CXX_COMPILER_ID}
     MATCHES
     Clang
  )
    target_compile_options(
      ${target_name} PRIVATE -Wno-suggest-destructor-override
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
  set(cxx-sources "${opentxs_SOURCE_DIR}/tests/main.cpp" "${file_name}")

  add_opentx_test_target("${target_name}" "${cxx-sources}")
endfunction()

function(
  add_opentx_low_level_test
  target_name
  file_name
)
  set(cxx-sources
      "${opentxs_SOURCE_DIR}/tests/lowlevel.cpp"
      "${file_name}"
      "${opentxs_SOURCE_DIR}/tests/OTLowLevelTestEnvironment.cpp"
  )

  add_opentx_test_target("${target_name}" "${cxx-sources}")
endfunction()

add_library(
  opentxs-test-helpers OBJECT
  "Basic.cpp"
  "Basic.hpp"
  "Cli.hpp"
)

if(OT_PCH)
  target_precompile_headers(
    opentxs-test-helpers
    PRIVATE
    <opentxs/opentxs.hpp>
    <gtest/gtest.h>
  )
endif()

target_include_directories(
  opentxs-test-helpers PUBLIC "${opentxs_SOURCE_DIR}/tests/"
)
target_include_directories(
  opentxs-test-helpers SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/"
          "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
)
target_link_libraries(
  opentxs-test-helpers
  PUBLIC
    opentxs::libopentxs
    Boost::filesystem
    Boost::program_options
)

if(OT_QT_EXPORT)
  target_link_libraries(opentxs-test-helpers PRIVATE Qt::Core)
endif()

if(OT_USE_VCPKG_TARGETS)
  target_link_libraries(opentxs-test-helpers PUBLIC GTest::gtest)
else()
  target_link_libraries(opentxs-test-helpers PUBLIC GTest::GTest)
endif()

set_target_properties(
  opentxs-test-helpers PROPERTIES POSITION_INDEPENDENT_CODE 1
)

add_subdirectory(blockchain)
add_subdirectory(blind)
add_subdirectory(client)
add_subdirectory(contact)
add_subdirectory(context)
add_subdirectory(core)
add_subdirectory(crypto)
add_subdirectory(identity)
add_subdirectory(integration)
add_subdirectory(network/zeromq)
add_subdirectory(otx)
add_subdirectory(paymentcode)
add_subdirectory(rpc)
add_subdirectory(ui)
add_subdirectory(dummy)

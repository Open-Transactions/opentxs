# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(data)
add_subdirectory(env)
add_subdirectory(fixtures)
add_subdirectory(mocks)

target_sources(
  opentxs-testlib
  PRIVATE
    "Basic.cpp"
    "Basic.hpp"
    "Cli.hpp"
)
target_include_directories(
  opentxs-testlib PUBLIC "${opentxs_SOURCE_DIR}/tests/"
)
target_include_directories(
  opentxs-testlib SYSTEM
  PUBLIC
    "${opentxs_SOURCE_DIR}/deps/"
    "${opentxs_SOURCE_DIR}/deps/frozen/include"
    "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
)
target_compile_definitions(
  opentxs-testlib
  PUBLIC
    "OT_QT=${OT_QT_EXPORT}"
    "OT_STORAGE_FS=${FS_EXPORT}"
    "OT_STORAGE_LMDB=${LMDB_EXPORT}"
    "OT_STORAGE_SQLITE=${SQLITE_EXPORT}"
)

if(OT_QT_EXPORT)
  target_sources(opentxs-testlib PRIVATE "Qt.cpp")
else()
  target_sources(opentxs-testlib PRIVATE "no-Qt.cpp")
endif()

if(OT_USE_VCPKG_TARGETS)
  target_link_libraries(opentxs-testlib PUBLIC GTest::gtest GTest::gmock)
else()
  target_link_libraries(opentxs-testlib PUBLIC GTest::GTest GTest::gmock)
endif()

set_target_properties(opentxs-testlib PROPERTIES POSITION_INDEPENDENT_CODE 1)

if(${CMAKE_CXX_COMPILER_ID}
   MATCHES
   Clang
)
  target_compile_options(
    opentxs-testlib PUBLIC -Wno-suggest-destructor-override
  )
endif()

if((NOT MSVC) AND (NOT WIN32))
  target_compile_options(opentxs-testlib PUBLIC -fPIC)
endif()

if(OT_PCH)
  target_precompile_headers(
    opentxs-testlib
    PRIVATE
    <gmock/gmock.h>
    <gtest/gtest.h>
  )
endif()

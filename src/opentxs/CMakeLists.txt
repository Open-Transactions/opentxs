# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(api)
add_subdirectory(blockchain)
add_subdirectory(contract)
add_subdirectory(crypto)
add_subdirectory(display)
add_subdirectory(identifier)
add_subdirectory(identity)
add_subdirectory(network)
add_subdirectory(otx)
add_subdirectory(protobuf)
add_subdirectory(rpc)
add_subdirectory(storage)

configure_file(
  "Context.version.cpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/Context.version.cpp"
  @ONLY
)

try_compile(
  OPENTXS_HAVE_UTC_CLOCK SOURCE_FROM_FILE
  "libopentxs-test-for-utc-clock.cpp"
  "${opentxs_SOURCE_DIR}/tools/cmake/libopentxs-test-for-utc-clock.cpp"
  LOG_DESCRIPTION
  "test for std::chrono::utc_clock"
  CXX_STANDARD
  20
  CXX_STANDARD_REQUIRED
  ON
  CXX_EXTENSIONS
  OFF
)

if(OPENTXS_HAVE_UTC_CLOCK)
  message(STATUS "standard library supports std::chrono::utc_clock")
  set(OPENTXS_TIME_TYPE "utc_clock")
else()
  message(STATUS "standard library does not support std::chrono::utc_clock")
  set(OPENTXS_TIME_TYPE "system_clock")
endif()

configure_file(
  "${opentxs_SOURCE_DIR}/include/opentxs/Time.hpp.in"
  "${opentxs_BINARY_DIR}/include/opentxs/Time.hpp"
  @ONLY
)

file(
  READ
  "Export.hpp.in"
  OPENTXS_CUSTOM_CONTENT_FROM_VARIABLE
)

include(GenerateExportHeader)
generate_export_header(
  libopentxs
  BASE_NAME
  opentxs
  EXPORT_FILE_NAME
  "${opentxs_BINARY_DIR}/include/opentxs/Export.hpp"
  EXPORT_MACRO_NAME
  OPENTXS_EXPORT
  CUSTOM_CONTENT_FROM_VARIABLE
  OPENTXS_CUSTOM_CONTENT_FROM_VARIABLE
)

libopentxs_add_enum_header("" "UnitType")
libopentxs_add_public_header("" "AccountType.hpp")
libopentxs_add_public_header("" "AddressType.hpp")
libopentxs_add_public_header("" "BlockchainProfile.hpp")
libopentxs_add_public_header("" "ConnectionMode.hpp")
libopentxs_add_public_header("" "Context.hpp")
libopentxs_add_public_header("" "Export.hpp")
libopentxs_add_public_header("" "Time.hpp")
libopentxs_add_public_header("" "Types.hpp")
libopentxs_add_public_header("" "WorkType.hpp")
libopentxs_add_public_header("" "opentxs.hpp")

if(OT_QT_EXPORT)
  libopentxs_add_public_header("" "Qt.hpp")
endif()

if(OT_WITH_QML)
  target_sources(opentxs-common PRIVATE "Context.qml.cpp")
  libopentxs_link_internal(Qt::Qml)
  libopentxs_link_external(Qt::Qml)
else()
  target_sources(opentxs-common PRIVATE "Context.no-qml.cpp")
endif()

target_sources(
  opentxs-common
  PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/Context.version.cpp"
    "BlockchainProfile.cpp"
    "ConnectionMode.cpp"
    "Context.cpp"
    "Time.cpp"
    "Types.cpp"
    "Types.internal.cpp"
    "Types.internal.hpp"
    "WorkType.cpp"
    "WorkType.internal.cpp"
    "WorkType.internal.hpp"
    "internal.factory.hpp"
    "strerror_r.hpp"
)
libopentxs_add_platform_specific("strerror_r")
target_include_directories(
  opentxs-common PRIVATE "${opentxs_BINARY_DIR}/include"
)

if(OT_WITH_TBB OR OT_PSTL_NEEDS_TBB)
  target_sources(opentxs-common PRIVATE "Context.tbb.cpp")
  libopentxs_link_internal(TBB::tbb)
  libopentxs_link_external(TBB::tbb)
else()
  target_sources(opentxs-common PRIVATE "Context.no-tbb.cpp")
endif()

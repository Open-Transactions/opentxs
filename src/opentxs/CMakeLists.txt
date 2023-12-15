# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(api)
add_subdirectory(blockchain)
add_subdirectory(identity)
add_subdirectory(network)
add_subdirectory(util)

configure_file(
  "Context.version.cpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/Context.version.cpp"
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

libopentxs_add_public_header("" "Context.hpp")
libopentxs_add_public_header("" "Export.hpp")
libopentxs_add_public_header("" "opentxs.hpp")

if(OT_QT_EXPORT)
  libopentxs_add_public_header("" "Qt.hpp")
endif()

target_sources(
  opentxs-common
  PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/Context.version.cpp"
    "Context.cpp"
    "internal.factory.hpp"
)
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

# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(WIN32)
  set(OT_MAPPED_RANGE_TYPE "std::span<char>")
  set(OT_MAPPED_FILE_TYPE "boost::iostreams::mapped_file_sink")
else()
  set(OT_MAPPED_RANGE_TYPE "std::span<const char>")
  set(OT_MAPPED_FILE_TYPE "boost::iostreams::mapped_file_source")
endif()

configure_file(
  "Types.hpp.in"
  "${opentxs_BINARY_DIR}/src/internal/util/storage/file/Types.hpp"
  @ONLY
)

configure_file(
  "BoostIostreams.hpp.in"
  "${opentxs_BINARY_DIR}/deps/BoostIostreams.hpp"
  @ONLY
)

libopentxs_link_external(Boost::iostreams)

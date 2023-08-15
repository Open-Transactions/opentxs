# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("util" "Log.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_BINARY_DIR}/deps/BoostStacktrace.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/Log.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/LogMacros.hpp"
    "Imp.cpp"
    "Imp.hpp"
    "Internal.cpp"
    "Log.cpp"
    "LogBuffer.cpp"
    "LogBuffer.hpp"
    "Logger.cpp"
    "Logger.hpp"
)

if(OT_BOOST_STACKTRACE_ADDR2LINE)
  target_compile_definitions(
    opentxs-common
    PRIVATE
      BOOST_STACKTRACE_USE_ADDR2LINE
      BOOST_STACKTRACE_ADDR2LINE_LOCATION="${BOOST_STACKTRACE_ADDR2LINE_LOCATION}"
  )
endif()

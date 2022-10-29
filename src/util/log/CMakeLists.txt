# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
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
set(cxx-install-headers "${opentxs_SOURCE_DIR}/include/opentxs/util/Log.hpp")

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/util
    COMPONENT dev
  )
endif()

# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(context)
add_subdirectory(crypto)
add_subdirectory(network)
add_subdirectory(session)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/api/Crypto.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/FactoryAPI.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/Legacy.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/Log.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/Settings.hpp"
    "Factory.cpp"
    "Factory.hpp"
    "Legacy.cpp"
    "Legacy.hpp"
    "Log.cpp"
    "Log.hpp"
    "Periodic.cpp"
    "Periodic.hpp"
    "Settings.cpp"
    "Settings.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Factory.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Periodic.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/Settings.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})
target_include_directories(
  opentxs-common SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/cs_libguarded/src"
)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api
    COMPONENT dev
  )
endif()

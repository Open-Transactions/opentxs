# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(client)
add_subdirectory(notary)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/api/session/Crypto.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/session/Endpoints.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/session/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/session/FactoryAPI.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/session/Session.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/session/Storage.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/session/Wallet.hpp"
    "base/Scheduler.cpp"
    "base/Scheduler.hpp"
    "base/Storage.cpp"
    "base/Storage.hpp"
    "base/ZMQ.cpp"
    "base/ZMQ.hpp"
    "Crypto.cpp"
    "Crypto.hpp"
    "Endpoints.cpp"
    "Endpoints.hpp"
    "Factory.cpp"
    "Factory.hpp"
    "Session.cpp"
    "Session.hpp"
    "Storage.cpp"
    "Storage.hpp"
    "Wallet.cpp"
    "Wallet.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/api/session/Crypto.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/session/Endpoints.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/session/Factory.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/session/Session.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/session/Storage.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/session/Wallet.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api/session
    COMPONENT dev
  )
endif()
# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("api/network" "Blockchain.hpp")
libopentxs_add_public_header("api/network" "BlockchainHandle.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/api/network/Blockchain.hpp"
    "Base.hpp"
    "Blockchain.cpp"
    "Blockchain.hpp"
    "BlockchainHandle.cpp"
    "BlockchainHandle.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "Factory.cpp"
      "Actor.cpp"
      "Actor.hpp"
      "Imp.cpp"
      "Imp.hpp"
      "StartupPublisher.cpp"
      "StartupPublisher.hpp"
  )
else()
  target_sources(opentxs-common PRIVATE "Null.cpp")
endif()

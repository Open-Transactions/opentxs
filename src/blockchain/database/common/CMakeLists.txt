# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_BINARY_DIR}/src/blockchain/database/common/StorageLevel.cpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/database/common/Common.hpp"
    "BlockFilter.cpp"
    "BlockFilter.hpp"
    "BlockHeaders.cpp"
    "BlockHeaders.hpp"
    "Blocks.hpp"
    "Bulk.cpp"
    "Bulk.hpp"
    "Common.cpp"
    "Config.cpp"
    "Config.hpp"
    "Database.cpp"
    "Database.hpp"
    "Peers.cpp"
    "Peers.hpp"
    "Sync.hpp"
    "Wallet.cpp"
    "Wallet.hpp"
)

if(OPENTXS_BLOCK_STORAGE_ENABLED)
  target_sources(
    opentxs-common
    PRIVATE
      "Blocks.cpp"
      "Sync.cpp"
      "SyncPrivate.cpp"
      "SyncPrivate.hpp"
  )
else()
  target_sources(opentxs-common PRIVATE "noStorage.cpp")
endif()

opentxs_link_internal(unofficial-sodium::sodium)
opentxs_link_internal(lmdb)
opentxs_link_external(unofficial-sodium::sodium)
opentxs_link_external(Boost::thread)
opentxs_link_external(lmdb)

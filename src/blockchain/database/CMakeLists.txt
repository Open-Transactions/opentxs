# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(common)
add_subdirectory(wallet)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/database/Block.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/database/Cfilter.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/database/Database.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/database/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/database/Header.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/database/Peer.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/database/Sync.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/database/Types.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/database/Wallet.hpp"
    "Blocks.cpp"
    "Blocks.hpp"
    "Database.cpp"
    "Database.hpp"
    "Filters.cpp"
    "Filters.hpp"
    "Headers.cpp"
    "Headers.hpp"
    "Sync.cpp"
    "Sync.hpp"
    "Wallet.cpp"
    "Wallet.hpp"
)
libopentxs_link_internal("${OT_LMDB_TARGET}")
libopentxs_link_external("${OT_LMDB_TARGET}")

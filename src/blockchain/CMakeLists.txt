# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(block)
add_subdirectory(bloom)
add_subdirectory(cfilter)
add_subdirectory(crypto)
add_subdirectory(node)
add_subdirectory(params)
add_subdirectory(protocol)
add_subdirectory(token)
add_subdirectory(work)

if(OT_BLOCKCHAIN_EXPORT)
  add_subdirectory(database)
endif()

libopentxs_add_public_header("blockchain" "Blockchain.hpp")
libopentxs_add_public_header("blockchain" "BlockchainType.hpp")
libopentxs_add_public_header("blockchain" "Category.hpp")
libopentxs_add_public_header("blockchain" "Types.hpp")

target_sources(opentxs-common PRIVATE "Common.cpp")
target_include_directories(
  opentxs-common PRIVATE "${opentxs_SOURCE_DIR}/generated/blockchain/xxd"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE "${opentxs_SOURCE_DIR}/src/internal/blockchain/Blockchain.hpp"
            "Blockchain.cpp"
  )
  libopentxs_link_internal("${OT_LMDB_TARGET}")
  libopentxs_link_external("${OT_LMDB_TARGET}")
endif()

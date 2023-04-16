# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(bitcoin)
add_subdirectory(block)
add_subdirectory(crypto)
add_subdirectory(ethereum)
add_subdirectory(node)
add_subdirectory(token)
add_subdirectory(work)

if(OT_BLOCKCHAIN_EXPORT)
  add_subdirectory(database)
  add_subdirectory(pkt)
endif()

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/Params.hpp"
    "Json.cpp"
    "Json.hpp"
    "Params.cpp"
)
target_include_directories(
  opentxs-common PRIVATE "${opentxs_SOURCE_DIR}/generated/blockchain/xxd"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/Blockchain.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/BlockchainType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/Types.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE "${opentxs_SOURCE_DIR}/src/internal/blockchain/Blockchain.hpp"
            "Blockchain.cpp"
  )
  opentxs_link_internal(lmdb)
  opentxs_link_external(lmdb)
else()
  # TODO move to appropriate subdirectories
  list(
    APPEND
    cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/Types.hpp"
  )
endif()

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain
    COMPONENT dev
  )
endif()

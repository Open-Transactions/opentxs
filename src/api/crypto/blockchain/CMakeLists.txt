# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("api/crypto" "Blockchain.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/api/crypto/Blockchain.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/crypto/blockchain/BalanceOracle.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/crypto/blockchain/Types.hpp"
    "AccountCache.cpp"
    "AccountCache.hpp"
    "Blockchain.cpp"
    "Blockchain.hpp"
    "Imp.cpp"
    "Imp.hpp"
)
libopentxs_parallel_algorithms()
libopentxs_link_internal(opentxs-bech32)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "BalanceOracle.cpp"
      "BalanceOracle.hpp"
      "Imp_blockchain.cpp"
      "Imp_blockchain.hpp"
      "NonNull.cpp"
  )
  libopentxs_link_internal("${OT_LMDB_TARGET}")
  libopentxs_link_internal("${OT_ZMQ_TARGET}")
  libopentxs_link_external("${OT_LMDB_TARGET}")
  libopentxs_link_external("${OT_ZMQ_TARGET}")
else()
  target_sources(opentxs-common PRIVATE "Null.cpp")
endif()

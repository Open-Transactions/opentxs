# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
    "Wallets.cpp"
    "Wallets.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Blockchain.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})
opentxs_link_internal(bech32)

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
  opentxs_link_internal(lmdb)
  opentxs_link_internal("${OT_ZMQ_TARGET}")
  opentxs_link_external(lmdb "${OT_ZMQ_TARGET}")
else()
  target_sources(opentxs-common PRIVATE "Null.cpp")
endif()

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api/crypto
    COMPONENT dev
  )
endif()

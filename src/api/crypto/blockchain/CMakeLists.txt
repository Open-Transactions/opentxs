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
target_link_libraries(opentxs-common PRIVATE bech32)
target_include_directories(
  opentxs-common SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/bech32/ref/c++"
                                "${opentxs_SOURCE_DIR}/deps/cs_libguarded/src"
)

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
  target_link_libraries(
    opentxs-common
    PRIVATE
      lmdb
      Boost::headers
      "${OT_ZMQ_TARGET}"
  )
  target_link_libraries(opentxs PUBLIC lmdb "${OT_ZMQ_TARGET}")
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

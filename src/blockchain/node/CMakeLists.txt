# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_BLOCKCHAIN_EXPORT)
  add_subdirectory(base)
  add_subdirectory(bitcoin)
  add_subdirectory(p2p)
  add_subdirectory(peermanager)
  add_subdirectory(wallet)
endif()

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/SendResult.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/Types.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/Factory.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/HeaderOracle.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/Node.hpp"
      "blockoracle/Cache.cpp"
      "blockoracle/Mem.cpp"
      "filteroracle/BlockIndexer.cpp"
      "filteroracle/BlockIndexer.hpp"
      "filteroracle/FilterCheckpoints.hpp"
      "filteroracle/FilterDownloader.hpp"
      "filteroracle/HeaderDownloader.hpp"
      "BlockOracle.cpp"
      "BlockOracle.hpp"
      "FilterOracle.cpp"
      "FilterOracle.hpp"
      "HeaderOracle.cpp"
      "HeaderOracle.hpp"
      "Mempool.cpp"
      "Mempool.hpp"
      "Node.cpp"
      "UpdateTransaction.cpp"
      "UpdateTransaction.hpp"
  )
  list(
    APPEND
    cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/BlockOracle.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/FilterOracle.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/HeaderOracle.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/Manager.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/TxoState.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/TxoTag.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/Wallet.hpp"
  )

  target_link_libraries(opentxs PUBLIC Boost::system)
  target_include_directories(
    opentxs-common SYSTEM
    PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
  )

  if(PACKETCRYPT_EXPORT)
    target_sources(
      opentxs-common PRIVATE "blockoracle/GetValidator_packetcrypt.cpp"
    )
  else()
    target_sources(opentxs-common PRIVATE "blockoracle/GetValidator.cpp")
  endif()
endif()

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/node
    COMPONENT dev
  )
endif()

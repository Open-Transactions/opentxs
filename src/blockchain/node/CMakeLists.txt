# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(blockoracle)
add_subdirectory(filteroracle)
add_subdirectory(headeroracle)
add_subdirectory(manager)
add_subdirectory(stats)
add_subdirectory(wallet)

if(OT_BLOCKCHAIN_EXPORT)
  add_subdirectory(peermanager)
endif()

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/SendResult.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/TxoState.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/TxoTag.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/Types.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/Config.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/Endpoints.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/Factory.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/Job.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/Mempool.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/SpendPolicy.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/Types.hpp"
      "Common.cpp"
      "Config.cpp"
      "Downloader.cpp"
      "Downloader.hpp"
      "Endpoints.cpp"
      "Job.cpp"
      "Mempool.cpp"
      "Mempool.hpp"
      "UpdateTransaction.cpp"
      "UpdateTransaction.hpp"
  )
  opentxs_link_external(Boost::system)

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

# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_BLOCKCHAIN_EXPORT)
  add_subdirectory(feeoracle)
  add_subdirectory(feesource)
  add_subdirectory(subchain)
  add_subdirectory(spend)
endif()

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/Wallet.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/Wallet.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/Account.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/Accounts.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/Factory.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/FeeOracle.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/FeeSource.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/Reorg.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/ReorgMaster.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/ReorgSlave.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/Types.hpp"
      "Account.cpp"
      "Account.hpp"
      "Accounts.cpp"
      "Accounts.hpp"
      "Actor.cpp"
      "Actor.hpp"
      "ReorgMaster.cpp"
      "ReorgMaster.hpp"
      "ReorgSlave.cpp"
      "ReorgSlave.hpp"
      "Shared.cpp"
      "Shared.hpp"
      "Wallet.cpp"
  )
  target_link_libraries(opentxs-common PRIVATE Boost::headers)
endif()

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/node
    COMPONENT dev
  )
endif()

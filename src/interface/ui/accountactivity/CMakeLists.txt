# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "AccountActivity.cpp"
    "AccountActivity.hpp"
    "BalanceItem.cpp"
    "BalanceItem.hpp"
    "ChequeBalanceItem.cpp"
    "ChequeBalanceItem.hpp"
    "CustodialAccountActivity.cpp"
    "CustodialAccountActivity.hpp"
    "TransferBalanceItem.cpp"
    "TransferBalanceItem.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/ui/AccountActivity.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/ui/BalanceItem.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "BlockchainAccountActivity.cpp"
      "BlockchainAccountActivity.hpp"
      "BlockchainBalanceItem.cpp"
      "BlockchainBalanceItem.hpp"
  )
else()
  target_sources(opentxs-common PRIVATE "no-blockchain.cpp")
endif()

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/interface/ui
    COMPONENT dev
  )
endif()

# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_BLOCKCHAIN_EXPORT)
  add_subdirectory(activity)
  add_subdirectory(ethereum)
  add_subdirectory(regtest)
  add_subdirectory(startstop)
endif()

add_opentx_test(ottest-blockchain-address Test_Address.cpp)

if(BIP32_EXPORT)
  add_opentx_test(ottest-blockchain-api Test_BlockchainAPI.cpp)
endif()

if(OT_BLOCKCHAIN_EXPORT)
  add_opentx_test(ottest-blockchain-bip44 Test_BIP44.cpp)
  add_opentx_test(ottest-blockchain-blockheader Test_BlockHeader.cpp)
  add_opentx_test(ottest-blockchain-blocks-bitcoin Test_BitcoinBlocks.cpp)
  add_opentx_test(ottest-blockchain-compactsize Test_CompactSize.cpp)
  add_opentx_test(ottest-blockchain-filters Test_Filters.cpp)
  add_opentx_test(ottest-blockchain-hash Test_NumericHash.cpp)
  add_opentx_test(ottest-blockchain-message Test_Message.cpp)
  add_opentx_test(ottest-blockchain-script-bitcoin Test_BitcoinScript.cpp)
  add_opentx_test(ottest-blockchain-api-sync-server Test_SyncServerDB.cpp)
  add_opentx_test(
    ottest-blockchain-transaction-bitcoin Test_BitcoinTransaction.cpp
  )
endif()

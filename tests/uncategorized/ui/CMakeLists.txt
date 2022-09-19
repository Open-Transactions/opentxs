# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_opentx_test(ottest-ui-account-tree Test_AccountTree.cpp)

if(OT_QT_EXPORT)
  add_opentx_test(ottest-ui-amountvalidator Test_AmountValidator.cpp)
endif()

add_opentx_test(
  ottest-ui-blockchain-account-status Test_BlockchainAccountStatus.cpp
)

if(SECP256K1_EXPORT AND BIP32_EXPORT)
  add_opentx_test(ottest-ui-contactlist Test_ContactList.cpp)
endif()

if(OT_BLOCKCHAIN_EXPORT)
  add_opentx_test(ottest-ui-blockchainselector Test_BlockchainSelector.cpp)
endif()

add_opentx_test(ottest-ui-items Test_Items.cpp)
add_opentx_test(ottest-ui-nym-list Test_NymList.cpp)
add_opentx_test(ottest-ui-seed-tree Test_SeedTree.cpp)

set_tests_properties(ottest-ui-account-tree PROPERTIES DISABLED TRUE)

if(OT_QT_EXPORT)
  set_tests_properties(ottest-ui-amountvalidator PROPERTIES DISABLED TRUE)
endif()

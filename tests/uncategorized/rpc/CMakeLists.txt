# Copyright (c) 2010-2023 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_ENABLE_RPC)
  # NOTE tests files must not access un-exported symbols, such as any class
  # defined in a .pb.h header or internal/ header
  #
  # add_opentx_test(ottest-rpc-async Test_Rpc_Async.cpp)
  # set_tests_properties(ottest-rpc-async PROPERTIES DISABLED TRUE)

  add_opentx_test(ottest-rpc-get-account-activity GetAccountActivity.cpp)
  add_opentx_test(ottest-rpc-get-account-balance GetAccountBalance.cpp)
  add_opentx_test(ottest-rpc-list-accounts ListAccounts.cpp)
  add_opentx_test(ottest-rpc-list-nyms ListNyms.cpp)
  add_opentx_test(ottest-rpc-send-payment SendPayment.cpp)

  if(OT_BLOCKCHAIN_EXPORT)
    add_opentx_test(
      ottest-rpc-send-payment-blockchain SendPayment_blockchain.cpp
    )
  endif()

  # NOTE tests files must not access un-exported symbols, such as any class
  # defined in a .pb.h header or internal/ header
  #
  # add_opentx_test(ottest-rpc-sync Test_Rpc.cpp)
  # set_tests_properties(ottest-rpc-sync PROPERTIES DISABLED TRUE)

  set_tests_properties(ottest-rpc-get-account-activity PROPERTIES DISABLED TRUE)
  set_tests_properties(ottest-rpc-get-account-balance PROPERTIES DISABLED TRUE)
  set_tests_properties(ottest-rpc-list-accounts PROPERTIES DISABLED TRUE)
  set_tests_properties(ottest-rpc-send-payment PROPERTIES DISABLED TRUE)
endif()

# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_opentx_test(ottest-blockchain-regtest-basic Basic.cpp)
add_opentx_test(
  ottest-blockchain-regtest-block-propagation BlockPropagation.cpp
)
add_opentx_test(ottest-blockchain-regtest-connection Connection.cpp)
add_opentx_test(ottest-blockchain-regtest-connection-tcp ConnectionTCP.cpp)
add_opentx_test(ottest-blockchain-regtest-connection-zmq ConnectionZMQ.cpp)
add_opentx_test(ottest-blockchain-regtest-generate-block Mine.cpp)
add_opentx_test(ottest-blockchain-regtest-payment-code PaymentCode.cpp)
add_opentx_test(ottest-blockchain-regtest-reorg Reorg.cpp)
add_opentx_test(ottest-blockchain-regtest-stress Stress.cpp)
add_opentx_test(ottest-blockchain-regtest-sync-client SyncClient.cpp)
add_opentx_test(ottest-blockchain-regtest-sync-server SyncServer.cpp)

set_tests_properties(ottest-blockchain-regtest-stress PROPERTIES DISABLED TRUE)

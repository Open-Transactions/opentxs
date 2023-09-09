# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_opentx_test(ottest-integration-blockchain-basic Basic.cpp)
add_opentx_test(
  ottest-integration-blockchain-block-propagation BlockPropagation.cpp
)
add_opentx_test(ottest-integration-blockchain-connection Connection.cpp)
add_opentx_test(ottest-integration-blockchain-connection-tcp ConnectionTCP.cpp)
add_opentx_test(ottest-integration-blockchain-connection-zmq ConnectionZMQ.cpp)
add_opentx_test(ottest-integration-blockchain-faucet Faucet.cpp)
add_opentx_test(ottest-integration-blockchain-generate-block Mine.cpp)
add_opentx_test(
  ottest-integration-blockchain-notification-extra ExtraNotification.cpp
)
add_opentx_test(
  ottest-integration-blockchain-notification-multiple MultipleNotification.cpp
)
add_opentx_test(ottest-integration-blockchain-payment-code PaymentCode.cpp)
add_opentx_test(ottest-integration-blockchain-reorg Reorg.cpp)
add_opentx_test(
  ottest-integration-blockchain-spend-unconfirmed UnconfirmedSpend.cpp
)
add_opentx_test(ottest-integration-blockchain-stress Stress.cpp)
add_opentx_test(ottest-integration-blockchain-sweep Sweep.cpp)
add_opentx_test(ottest-integration-blockchain-sync-client SyncClient.cpp)
add_opentx_test(ottest-integration-blockchain-sync-server SyncServer.cpp)

if(OT_ENABLE_MATTERFI)
  add_opentx_test(
    ottest-integration-blockchain-matterfi-notification Matterfi.cpp
  )
endif()

set_tests_properties(
  ottest-integration-blockchain-stress PROPERTIES DISABLED TRUE
)

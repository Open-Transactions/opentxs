# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_opentx_test(ottest-network-zeromq-dealer Test_DealerSocket.cpp)
add_opentx_test(ottest-network-zeromq-dealerreply Test_DealerReply.cpp)
add_opentx_test(ottest-network-zeromq-dealerrouter Test_DealerRouter.cpp)
add_opentx_test(ottest-network-zeromq-frame Test_Frame.cpp)
add_opentx_test(ottest-network-zeromq-listencallback Test_ListenCallback.cpp)
add_opentx_test(ottest-network-zeromq-message Test_Message.cpp)
add_opentx_test(ottest-network-zeromq-pair Test_PairSocket.cpp)
add_opentx_test(ottest-network-zeromq-publish Test_PublishSocket.cpp)
add_opentx_test(
  ottest-network-zeromq-publishsubscribe Test_PublishSubscribe.cpp
)
add_opentx_test(ottest-network-zeromq-pushpull Test_PushPull.cpp)
add_opentx_test(ottest-network-zeromq-pushsubscribe Test_PushSubscribe.cpp)
add_opentx_test(ottest-network-zeromq-reply Test_ReplySocket.cpp)
add_opentx_test(ottest-network-zeromq-replycallback Test_ReplyCallback.cpp)
add_opentx_test(ottest-network-zeromq-request Test_RequestSocket.cpp)
add_opentx_test(ottest-network-zeromq-requestreply Test_RequestReply.cpp)
add_opentx_test(ottest-network-zeromq-requestrouter Test_RequestRouter.cpp)
add_opentx_test(ottest-network-zeromq-router Test_RouterSocket.cpp)
add_opentx_test(ottest-network-zeromq-routerdealer Test_RouterDealer.cpp)
add_opentx_test(ottest-network-zeromq-routerrouter Test_RouterRouter.cpp)
add_opentx_test(ottest-network-zeromq-stress Test_Stress.cpp)
add_opentx_test(ottest-network-zeromq-subscribe Test_SubscribeSocket.cpp)

set_tests_properties(ottest-network-zeromq-stress PROPERTIES DISABLED TRUE)

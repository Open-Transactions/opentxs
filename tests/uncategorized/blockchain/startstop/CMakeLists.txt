# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_opentx_test(ottest-blockchain-startstop-all Test_all.cpp)
add_opentx_test(ottest-blockchain-startstop-bch Test_bch.cpp)
add_opentx_test(ottest-blockchain-startstop-bch_testnet3 Test_bch_testnet3.cpp)
add_opentx_test(ottest-blockchain-startstop-bch_testnet4 Test_bch_testnet4.cpp)
add_opentx_test(ottest-blockchain-startstop-btc Test_btc.cpp)
add_opentx_test(ottest-blockchain-startstop-btc_testnet3 Test_btc_testnet3.cpp)
add_opentx_test(ottest-blockchain-startstop-frontier Test_frontier.cpp)
add_opentx_test(ottest-blockchain-startstop-ltc Test_ltc.cpp)
add_opentx_test(ottest-blockchain-startstop-ltc_testnet4 Test_ltc_testnet4.cpp)
add_opentx_test(ottest-blockchain-startstop-pkt Test_pkt.cpp)
add_opentx_test(ottest-blockchain-startstop-ropsten Test_ropsten.cpp)
add_opentx_test(ottest-blockchain-startstop-regtest Test_regtest.cpp)

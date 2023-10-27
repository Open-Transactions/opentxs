# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_opentx_test(ottest-unit-blockchain-address Address.cpp)

if(OT_BLOCKCHAIN_EXPORT)
  add_opentx_low_level_test(ottest-unit-blockchain-chains ChainData.cpp)
  add_opentx_low_level_test(ottest-unit-blockchain-p2p-address P2PAddress.cpp)
  add_opentx_test(ottest-unit-blockchain-bip158 Bip158.cpp)
  add_opentx_test(ottest-unit-blockchain-cashtoken Cashtoken.cpp)
  add_opentx_test(ottest-unit-blockchain-genesis-blocks BlockChecker.cpp)
endif()

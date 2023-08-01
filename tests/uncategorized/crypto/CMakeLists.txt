# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_opentx_test(ottest-crypto-asymmetric Test_AsymmetricProvider.cpp)

if(BIP32_EXPORT)
  add_opentx_test(ottest-crypto-bip32 Test_BIP32.cpp)
endif()

add_opentx_test(ottest-crypto-bip39 Test_BIP39.cpp)
add_opentx_test(ottest-crypto-bitcoin Test_BitcoinProviders.cpp)
add_opentx_test(ottest-crypto-envelope Test_Envelope.cpp)
add_opentx_test(ottest-crypto-hash Test_Hash.cpp)
add_opentx_test(ottest-crypto-symmetric Symmetric.cpp)

add_opentx_low_level_test(ottest-crypto-seed Seed.cpp)

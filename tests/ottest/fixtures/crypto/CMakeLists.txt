# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-testlib
  PRIVATE
    "Hash.cpp"
    "Hash.hpp"
    "Symmetric.cpp"
    "Symmetric.hpp"
    "AsymmetricProvider.cpp"
    "AsymmetricProvider.hpp"
    "BIP32.cpp"
    "BIP32.hpp"
    "BIP39.cpp"
    "BIP39.hpp"
    "BitcoinProviders.cpp"
    "BitcoinProviders.hpp"
)

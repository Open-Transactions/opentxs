# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("blockchain/crypto" "AddressStyle.hpp")
libopentxs_add_public_header("blockchain/crypto" "Bip44Subchain.hpp")
libopentxs_add_public_header("blockchain/crypto" "Bip44Type.hpp")
libopentxs_add_public_header("blockchain/crypto" "HDProtocol.hpp")
libopentxs_add_public_header("blockchain/crypto" "SubaccountType.hpp")
libopentxs_add_public_header("blockchain/crypto" "Subchain.hpp")
libopentxs_add_public_header("blockchain/crypto" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "AddressStyle.cpp"
    "Bip44Subchain.cpp"
    "Bip44Type.cpp"
    "HDProtocol.cpp"
    "SubaccountType.cpp"
    "Subchain.cpp"
    "Types.cpp"
    "Types.internal.cpp"
    "Types.internal.hpp"
)

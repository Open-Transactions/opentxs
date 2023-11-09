# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(account)
add_subdirectory(element)
add_subdirectory(subaccount)
add_subdirectory(wallet)

libopentxs_add_public_header("blockchain/crypto" "AddressStyle.hpp")
libopentxs_add_public_header("blockchain/crypto" "HDProtocol.hpp")
libopentxs_add_public_header("blockchain/crypto" "SubaccountType.hpp")
libopentxs_add_public_header("blockchain/crypto" "Subchain.hpp")
libopentxs_add_public_header("blockchain/crypto" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/crypto/Factory.hpp"
    "Crypto.cpp"
    "Internal.cpp"
)

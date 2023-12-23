# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(block)
add_subdirectory(cfilter)
add_subdirectory(crypto)
add_subdirectory(node)
add_subdirectory(protocol)
add_subdirectory(token)

libopentxs_add_public_header("blockchain" "BlockchainType.hpp")
libopentxs_add_public_header("blockchain" "Category.hpp")
libopentxs_add_public_header("blockchain" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "BlockchainType.cpp"
    "Category.cpp"
    "Types.cpp"
)

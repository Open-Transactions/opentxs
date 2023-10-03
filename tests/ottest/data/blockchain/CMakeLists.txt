# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-testlib
  PRIVATE
    "Bip143.cpp"
    "Bip143.hpp"
    "Bip158.cpp"
    "Bip158.hpp"
    "Blocks.hpp"
    "Cashtoken.cpp"
    "Cashtoken.hpp"
    "Headers.cpp"
    "Headers.hpp"
    "bch_filter_1307544.cpp"
    "bch_filter_1307723.cpp"
    "btc_block_762580.cpp"
    "tn_btc_block_1489260.cpp"
    "tn_dash_block_7000.cpp"
)
target_include_directories(
  opentxs-testlib PRIVATE "${opentxs_SOURCE_DIR}/generated/tests/xxd"
)

if(OT_BLOCKCHAIN_EXPORT AND NOT MSVC)
  # NOTE the RLP test vectors can not be easily embedded when the compiler is
  # MSVC due to string literal length restrictions
  target_sources(
    opentxs-testlib
    PRIVATE
      "${opentxs_BINARY_DIR}/tests/ottest/data/blockchain/RLP_raw.cpp"
      "RLP.cpp"
      "RLP.hpp"
  )
endif()

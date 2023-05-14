# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(block)
add_subdirectory(header)
add_subdirectory(transaction)

libopentxs_add_public_header("blockchain/block" "Hash.hpp")
libopentxs_add_public_header("blockchain/block" "NumericHash.hpp")
libopentxs_add_public_header("blockchain/block" "Outpoint.hpp")
libopentxs_add_public_header("blockchain/block" "Position.hpp")
libopentxs_add_public_header("blockchain/block" "TransactionHash.hpp")
libopentxs_add_public_header("blockchain/block" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "Hash.cpp"
    "NumericHash.cpp"
    "NumericHashPrivate.cpp"
    "NumericHashPrivate.hpp"
    "Outpoint.cpp"
    "Position.cpp"
    "TransactionHash.cpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/block/Parser.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/block/Factory.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/block/Types.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/block/Validator.hpp"
      "Common.cpp"
      "Hash.cpp"
      "Parser.cpp"
  )
endif()

# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(block)
add_subdirectory(header)
add_subdirectory(input)
add_subdirectory(output)
add_subdirectory(parser)
add_subdirectory(script)
add_subdirectory(transaction)

libopentxs_add_public_header("blockchain/bitcoin/block" "Opcodes.hpp")
libopentxs_add_public_header("blockchain/bitcoin/block" "Pattern.hpp")
libopentxs_add_public_header("blockchain/bitcoin/block" "Position.hpp")
libopentxs_add_public_header("blockchain/bitcoin/block" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/bitcoin/block/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/bitcoin/block/Types.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(opentxs-common PRIVATE "Common.cpp")
endif()

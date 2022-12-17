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

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/bitcoin/block/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/bitcoin/block/Types.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/bitcoin/block/Opcodes.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/bitcoin/block/Pattern.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/bitcoin/block/Position.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/bitcoin/block/Types.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(opentxs-common PRIVATE "Common.cpp")
endif()

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/bitcoin/block
    COMPONENT dev
  )
endif()

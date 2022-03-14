# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(bitcoin)
add_subdirectory(pkt)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/block/Block.hpp"
    "Block.cpp"
    "Block.hpp"
    "Common.cpp"
    "Header.cpp"
    "Header.hpp"
    "Outpoint.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/Block.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/Header.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/Outpoint.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/Types.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/block
    COMPONENT dev
  )
endif()

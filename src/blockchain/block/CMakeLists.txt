# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(header)

target_sources(
  opentxs-common
  PRIVATE
    "Hash.cpp"
    "NumericHash.cpp"
    "NumericHashPrivate.cpp"
    "NumericHashPrivate.hpp"
    "Outpoint.cpp"
    "Position.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/Block.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/Hash.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/NumericHash.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/Outpoint.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/Position.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/Types.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/block/Block.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/block/Parser.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/block/Factory.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/block/Types.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/block/Validator.hpp"
      "Block.cpp"
      "Block.hpp"
      "Common.cpp"
      "Hash.cpp"
      "Parser.cpp"
  )
else()
  target_sources(opentxs-common PRIVATE "Disabled.cpp")
endif()

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/block
    COMPONENT dev
  )
endif()

# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/block/bitcoin/Bitcoin.hpp"
    "Bitcoin.cpp"
    "Block.cpp"
    "Block.hpp"
    "BlockParser.cpp"
    "BlockParser.hpp"
    "Header.cpp"
    "Header.hpp"
    "Input.cpp"
    "Input.hpp"
    "InputCache.cpp"
    "Inputs.cpp"
    "Inputs.hpp"
    "Output.cpp"
    "Output.hpp"
    "OutputCache.cpp"
    "Outputs.cpp"
    "Outputs.hpp"
    "Script.cpp"
    "Script.hpp"
    "Transaction.cpp"
    "Transaction.hpp"
    "TransactionCache.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Block.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Header.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Input.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Inputs.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Opcodes.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Output.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Outputs.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Script.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Transaction.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Types.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})
target_include_directories(
  opentxs-common SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/block/bitcoin
    COMPONENT dev
  )
endif()

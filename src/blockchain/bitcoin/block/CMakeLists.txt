# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(header)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/bitcoin/block/Block.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/bitcoin/block/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/bitcoin/block/Input.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/bitcoin/block/Inputs.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/bitcoin/block/Output.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/bitcoin/block/Outputs.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/bitcoin/block/Script.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/bitcoin/block/Transaction.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/bitcoin/block/Types.hpp"
    "Null.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/bitcoin/block/Block.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/bitcoin/block/Input.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/bitcoin/block/Inputs.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/bitcoin/block/Opcodes.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/bitcoin/block/Output.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/bitcoin/block/Outputs.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/bitcoin/block/Script.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/bitcoin/block/Transaction.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/bitcoin/block/Types.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "Block.cpp"
      "Block.hpp"
      "BlockParser.cpp"
      "BlockParser.hpp"
      "Common.cpp"
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
  target_include_directories(
    opentxs-common SYSTEM
    PRIVATE "${opentxs_SOURCE_DIR}/deps/cs_libguarded/src"
            "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
  )
else()
  target_sources(opentxs-common PRIVATE "Disabled.cpp")
endif()

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/bitcoin/block
    COMPONENT dev
  )
endif()

# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(blockfetcher)
add_subdirectory(cache)

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/BlockOracle.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/blockoracle/BlockBatch.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/blockoracle/BlockOracle.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/blockoracle/Types.hpp"
      "Actor.cpp"
      "Actor.hpp"
      "BlockBatch.cpp"
      "BlockBatch.hpp"
      "BlockOracle.cpp"
      "Shared.cpp"
      "Shared.hpp"
  )
  opentxs_link_external(Boost::system)

  if(PACKETCRYPT_EXPORT)
    target_sources(opentxs-common PRIVATE "GetValidator_packetcrypt.cpp")
  else()
    target_sources(opentxs-common PRIVATE "GetValidator.cpp")
  endif()
endif()

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/node
    COMPONENT dev
  )
endif()

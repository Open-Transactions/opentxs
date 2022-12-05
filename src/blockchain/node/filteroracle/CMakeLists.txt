# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/FilterOracle.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/filteroracle/FilterOracle.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/filteroracle/BlockIndexer.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/filteroracle/Types.hpp"
      "BlockIndexer.cpp"
      "BlockIndexer.hpp"
      "Data.cpp"
      "Data.hpp"
      "FilterOracle.cpp"
      "FilterOracle.hpp"
      "Shared.cpp"
      "Shared.hpp"
  )
  opentxs_link_external(Boost::system)
endif()

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/node
    COMPONENT dev
  )
endif()

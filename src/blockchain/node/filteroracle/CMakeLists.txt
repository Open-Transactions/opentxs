# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("blockchain/node" "FilterOracle.hpp")

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
  libopentxs_link_external(Boost::system)
endif()

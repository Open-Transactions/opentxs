# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/blockoracle/BlockFetcher.hpp"
      "Actor.cpp"
      "Actor.hpp"
      "BlockFetcher.cpp"
      "Data.cpp"
      "Shared.cpp"
      "Shared.hpp"
  )
  opentxs_link_external(Boost::system)
endif()

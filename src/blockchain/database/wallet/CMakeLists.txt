# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "Output.cpp"
    "Output.hpp"
    "OutputCache.cpp"
    "OutputCache.hpp"
    "Pattern.cpp"
    "Pattern.hpp"
    "Position.cpp"
    "Position.hpp"
    "Proposal.cpp"
    "Proposal.hpp"
    "Subchain.cpp"
    "Subchain.hpp"
    "SubchainCache.cpp"
    "SubchainCache.hpp"
    "SubchainID.cpp"
    "SubchainID.hpp"
    "SubchainPrivate.cpp"
    "SubchainPrivate.hpp"
    "Types.hpp"
)
target_link_libraries(opentxs-common PRIVATE Boost::headers)
target_include_directories(
  opentxs-common SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/cs_libguarded/src"
          "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
)

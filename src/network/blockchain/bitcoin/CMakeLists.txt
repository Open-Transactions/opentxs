# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(message)

libopentxs_add_public_header("network/blockchain/bitcoin" "CompactSize.hpp")

target_sources(opentxs-common PRIVATE "CompactSize.cpp" "CompactSize.hpp")

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/network/blockchain/bitcoin/Factory.hpp"
      "Inventory.cpp"
      "Inventory.hpp"
      "Peer.cpp"
      "Peer.hpp"
      "Peer.tpp"
      "Peer.tpp.cpp"
  )
endif()

libopentxs_link_external(Boost::system)

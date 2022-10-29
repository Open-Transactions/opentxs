# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(opentxs-common PRIVATE "CompactSize.cpp" "CompactSize.hpp")
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/blockchain/bitcoin/CompactSize.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/network/blockchain/bitcoin/Factory.hpp"
      "Peer.cpp"
      "Peer.hpp"
      "Peer.tpp"
  )
endif()

opentxs_link_external(Boost::system)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network/blockchain/bitcoin
    COMPONENT dev
  )
endif()

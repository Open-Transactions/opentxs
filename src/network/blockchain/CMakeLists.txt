# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(address)
add_subdirectory(bitcoin)

if(OT_BLOCKCHAIN_EXPORT)
  add_subdirectory(connectionmanager)
  add_subdirectory(otdht)
  add_subdirectory(peer)
endif()

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE "${opentxs_BINARY_DIR}/src/network/blockchain/Seednodes.cpp"
            "Seednodes.hpp"
  )
  libopentxs_link_external(Boost::system)
endif()

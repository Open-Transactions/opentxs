# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(bitcoin)

if(OT_BLOCKCHAIN_EXPORT)
  add_subdirectory(connectionmanager)
  add_subdirectory(otdht)
  add_subdirectory(peer)
endif()

libopentxs_add_public_header("network/blockchain" "Address.hpp")
libopentxs_add_public_header("network/blockchain" "Protocol.hpp")
libopentxs_add_public_header("network/blockchain" "Subchain.hpp")
libopentxs_add_public_header("network/blockchain" "Transport.hpp")
libopentxs_add_public_header("network/blockchain" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/network/blockchain/Address.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/blockchain/Factory.hpp"
    "Address.cpp"
    "Common.cpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_BINARY_DIR}/src/network/blockchain/Seednodes.cpp"
      "AddressPrivate.cpp"
      "AddressPrivate.hpp"
      "Seednodes.hpp"
  )
  libopentxs_link_external(Boost::system)
else()
  target_sources(opentxs-common PRIVATE "Disabled.cpp")
endif()

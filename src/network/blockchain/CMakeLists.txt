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

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/network/blockchain/Address.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/blockchain/Factory.hpp"
    "Address.cpp"
    "Common.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/blockchain/Address.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/blockchain/Protocol.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/blockchain/Subchain.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/blockchain/Transport.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/blockchain/Types.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common PRIVATE "AddressPrivate.cpp" "AddressPrivate.hpp"
  )
  opentxs_link_external(Boost::system)
else()
  target_sources(opentxs-common PRIVATE "Disabled.cpp")
endif()

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network/blockchain
    COMPONENT dev
  )
endif()

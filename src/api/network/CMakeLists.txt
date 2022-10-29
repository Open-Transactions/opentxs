# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(asio)
add_subdirectory(blockchain)
add_subdirectory(otdht)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/api/network/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/network/Network.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/network/Types.hpp"
    "Network.cpp"
    "Network.hpp"
    "ZAP.cpp"
    "ZAP.hpp"
    "ZMQ.cpp"
    "ZMQ.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/api/network/Network.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/network/ZAP.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/network/ZMQ.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})
opentxs_link_internal(OpenSSL::SSL)
opentxs_link_external(Boost::system)
opentxs_link_external(Boost::thread)
opentxs_link_external(OpenSSL::SSL)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api/network
    COMPONENT dev
  )
endif()

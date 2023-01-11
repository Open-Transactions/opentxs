# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/network/asio/HTTP.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/asio/HTTPS.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/asio/Types.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/asio/WebRequest.hpp"
    "Common.cpp"
    "Endpoint.cpp"
    "Endpoint.hpp"
    "HTTP.cpp"
    "HTTPS.cpp"
    "SSLCerts.cpp"
    "Socket.cpp"
    "Socket.hpp"
    "WebRequest.tpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/asio/Endpoint.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/asio/Socket.hpp"
)
opentxs_link_external(Boost::system)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network/asio
    COMPONENT dev
  )
endif()

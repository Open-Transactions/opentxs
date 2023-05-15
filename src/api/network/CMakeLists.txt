# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(asio)
add_subdirectory(blockchain)
add_subdirectory(otdht)

libopentxs_add_public_header("api/network" "Network.hpp")
libopentxs_add_public_header("api/network" "ZAP.hpp")
libopentxs_add_public_header("api/network" "ZMQ.hpp")

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
libopentxs_link_internal(OpenSSL::SSL)
libopentxs_link_external(Boost::system)
libopentxs_link_external(Boost::thread)
libopentxs_link_external(OpenSSL::SSL)

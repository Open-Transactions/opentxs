# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("api/network" "Asio.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/api/network/Factory.hpp"
    "Acceptor.cpp"
    "Acceptor.hpp"
    "Acceptors.cpp"
    "Acceptors.hpp"
    "Actor.cpp"
    "Actor.hpp"
    "AddressSites.cpp"
    "Asio.cpp"
    "Asio.hpp"
    "Context.cpp"
    "Context.hpp"
    "Data.cpp"
    "Data.hpp"
    "Shared.cpp"
    "Shared.hpp"
)
libopentxs_link_internal(OpenSSL::SSL)
libopentxs_link_external(Boost::system)
libopentxs_link_external(Boost::thread)
libopentxs_link_external(OpenSSL::SSL)

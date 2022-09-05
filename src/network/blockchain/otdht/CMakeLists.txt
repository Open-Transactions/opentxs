# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/network/blockchain/OTDHT.hpp"
    "Actor.cpp"
    "Actor.hpp"
    "Client.cpp"
    "Client.hpp"
    "Server.cpp"
    "Server.hpp"
    "OTDHT.cpp"
)
target_include_directories(
  opentxs-common SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/cs_libguarded/src"
)
target_link_libraries(opentxs-common PRIVATE Boost::headers)
target_link_libraries(opentxs PUBLIC Boost::system)

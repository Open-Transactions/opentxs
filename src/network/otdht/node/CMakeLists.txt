# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/network/otdht/Node.hpp"
    "Actor.cpp"
    "Actor.hpp"
    "Data.cpp"
    "Node.cpp"
    "Shared.cpp"
    "Shared.hpp"
)
libopentxs_link_external(Boost::system)

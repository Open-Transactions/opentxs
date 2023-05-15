# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("blockchain/node" "Stats.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "Actor.cpp"
    "Actor.hpp"
    "Data.cpp"
    "Data.hpp"
    "Imp.cpp"
    "Imp.hpp"
    "Shared.cpp"
    "Shared.hpp"
    "Stats.cpp"
)
libopentxs_link_external(Boost::system)

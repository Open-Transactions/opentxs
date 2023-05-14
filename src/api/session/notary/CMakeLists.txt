# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("api/session" "Notary.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/api/session/notary/Notary.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/session/notary/Types.hpp"
    "Actor.cpp"
    "Actor.hpp"
    "Factory.cpp"
    "Factory.hpp"
    "Notary.cpp"
    "Notary.hpp"
    "Shared.cpp"
    "Shared.hpp"
    "Wallet.cpp"
    "Wallet.hpp"
)
libopentxs_link_external(Boost::system)

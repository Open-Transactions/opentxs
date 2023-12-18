# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(claim)

libopentxs_add_public_header("identity/wot" "Claim.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "Claim.cpp"
    "Claim.factory.cpp"
    "Claim.hash.cpp"
    "Claim.internal.cpp"
    "Claim.internal.hpp"
    "Claim.less.cpp"
    "ClaimPrivate.cpp"
    "ClaimPrivate.data.cpp"
    "ClaimPrivate.hpp"
    "Types.cpp"
)

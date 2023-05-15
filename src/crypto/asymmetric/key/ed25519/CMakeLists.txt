# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("crypto/asymmetric/key" "Ed25519.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/crypto/asymmetric/key/Ed25519.hpp"
    "Ed25519.cpp"
    "Ed25519Private.cpp"
    "Ed25519Private.hpp"
    "Imp.cpp"
    "Imp.hpp"
)

if(ED25519_EXPORT)
  target_sources(opentxs-common PRIVATE "Enabled.cpp")
else()
  target_sources(opentxs-common PRIVATE "Disabled.cpp")
endif()

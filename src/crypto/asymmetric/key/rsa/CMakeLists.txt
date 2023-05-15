# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("crypto/asymmetric/key" "RSA.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/crypto/asymmetric/key/RSA.hpp"
    "Imp.cpp"
    "Imp.hpp"
    "RSA.cpp"
    "RSAPrivate.cpp"
    "RSAPrivate.hpp"
)

if(RSA_EXPORT)
  target_sources(opentxs-common PRIVATE "Enabled.cpp")
else()
  target_sources(opentxs-common PRIVATE "Disabled.cpp")
endif()

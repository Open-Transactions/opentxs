# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("crypto/symmetric" "Algorithm.hpp")
libopentxs_add_public_header("crypto/symmetric" "Key.hpp")
libopentxs_add_public_header("crypto/symmetric" "Source.hpp")
libopentxs_add_public_header("crypto/symmetric" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/crypto/symmetric/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/symmetric/Key.hpp"
    "Common.cpp"
    "Factory.cpp"
    "Key.cpp"
    "KeyPrivate.cpp"
    "KeyPrivate.hpp"
)

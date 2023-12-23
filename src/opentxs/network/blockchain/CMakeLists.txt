# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(bitcoin)

libopentxs_add_public_header("network/blockchain" "Protocol.hpp")
libopentxs_add_public_header("network/blockchain" "Subchain.hpp")
libopentxs_add_public_header("network/blockchain" "Transport.hpp")
libopentxs_add_public_header("network/blockchain" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "Protocol.cpp"
    "Subchain.cpp"
    "Transport.cpp"
    "Types.cpp"
    "Types.internal.cpp"
    "Types.internal.hpp"
)

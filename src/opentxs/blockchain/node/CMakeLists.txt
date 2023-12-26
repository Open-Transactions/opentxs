# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("blockchain/node" "Funding.hpp")
libopentxs_add_public_header("blockchain/node" "SendResult.hpp")
libopentxs_add_public_header("blockchain/node" "TxoState.hpp")
libopentxs_add_public_header("blockchain/node" "TxoTag.hpp")
libopentxs_add_public_header("blockchain/node" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "Funding.cpp"
    "SendResult.cpp"
    "TxoState.cpp"
    "TxoTag.cpp"
    "Types.cpp"
    "Types.internal.cpp"
    "Types.internal.hpp"
)

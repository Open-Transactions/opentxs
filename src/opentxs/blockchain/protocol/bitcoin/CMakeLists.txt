# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(base)
add_subdirectory(bitcoincash)

libopentxs_add_public_header("blockchain/protocol/bitcoin/" "Types.hpp")

target_sources(opentxs-common PRIVATE "Types.cpp")

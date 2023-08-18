# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("blockchain/token" "Descriptor.hpp")
libopentxs_add_public_header("blockchain/token" "TokenType.hpp")
libopentxs_add_public_header("blockchain/token" "Types.hpp")

target_sources(opentxs-common PRIVATE "Common.cpp" "Descriptor.cpp")

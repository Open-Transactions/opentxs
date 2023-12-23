# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(peer)

libopentxs_add_public_header("contract" "ContractType.hpp")
libopentxs_add_public_header("contract" "ProtocolVersion.hpp")
libopentxs_add_public_header("contract" "Types.hpp")
libopentxs_add_public_header("contract" "UnitDefinitionType.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "ContractType.cpp"
    "ProtocolVersion.cpp"
    "Types.cpp"
    "Types.internal.cpp"
    "Types.internal.hpp"
    "UnitDefinitionType.cpp"
)

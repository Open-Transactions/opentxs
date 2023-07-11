# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(verification)

libopentxs_add_public_header("identity/wot/verification" "Group.hpp")
libopentxs_add_public_header("identity/wot/verification" "Item.hpp")
libopentxs_add_public_header("identity/wot/verification" "Nym.hpp")
libopentxs_add_public_header("identity/wot/verification" "Set.hpp")
libopentxs_add_public_header("identity/wot/verification" "Types.hpp")
libopentxs_add_public_header("identity/wot/verification" "VerificationType.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/identity/wot/verification/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/identity/wot/verification/Group.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/identity/wot/verification/Item.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/identity/wot/verification/Nym.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/identity/wot/verification/Set.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/identity/wot/verification/Types.hpp"
    "Common.cpp"
    "Group.cpp"
    "Group.hpp"
    "Item.cpp"
    "Item.hpp"
    "Nym.cpp"
    "Nym.hpp"
    "Set.cpp"
    "Set.hpp"
)

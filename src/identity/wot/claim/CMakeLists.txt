# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("identity/wot/claim" "Attribute.hpp")
libopentxs_add_public_header("identity/wot/claim" "ClaimType.hpp")
libopentxs_add_public_header("identity/wot/claim" "Data.hpp")
libopentxs_add_public_header("identity/wot/claim" "Group.hpp")
libopentxs_add_public_header("identity/wot/claim" "Item.hpp")
libopentxs_add_public_header("identity/wot/claim" "Section.hpp")
libopentxs_add_public_header("identity/wot/claim" "SectionType.hpp")
libopentxs_add_public_header("identity/wot/claim" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/identity/wot/claim/Types.hpp"
    "Data.cpp"
    "Group.cpp"
    "Internal.cpp"
    "Item.cpp"
    "Section.cpp"
)

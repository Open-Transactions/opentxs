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
    "Attribute.cpp"
    "ClaimType.cpp"
    "Item.cpp"
    "Item.factory.cpp"
    "Item.hash.cpp"
    "Item.internal.cpp"
    "Item.internal.hpp"
    "Item.less.cpp"
    "ItemPrivate.cpp"
    "ItemPrivate.hpp"
    "SectionType.cpp"
    "Types.cpp"
    "Types.internal.hpp"
    "internal.factory.hpp"
)

add_library(opentxs-identity-wot-claim OBJECT "Types.internal.cpp")
target_link_libraries(
  opentxs-identity-wot-claim PRIVATE opentxs-common opentxs-protobuf
)
target_include_directories(
  opentxs-identity-wot-claim SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/external/frozen/include"
)
libopentxs_configure_constexpr_target(opentxs-identity-wot-claim)

# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("identifier" "Account.hpp")
libopentxs_add_public_header("identifier" "AccountSubtype.hpp")
libopentxs_add_public_header("identifier" "Algorithm.hpp")
libopentxs_add_public_header("identifier" "Generic.hpp")
libopentxs_add_public_header("identifier" "HDSeed.hpp")
libopentxs_add_public_header("identifier" "Notary.hpp")
libopentxs_add_public_header("identifier" "Nym.hpp")
libopentxs_add_public_header("identifier" "Type.hpp")
libopentxs_add_public_header("identifier" "Types.hpp")
libopentxs_add_public_header("identifier" "UnitDefinition.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/core/identifier/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/identifier/Identifier.hpp"
    "Account.cpp"
    "AccountSubtype.cpp"
    "Algorithm.cpp"
    "Generic.cpp"
    "HDSeed.cpp"
    "IdentifierPrivate.cpp"
    "IdentifierPrivate.hpp"
    "Notary.cpp"
    "Nym.cpp"
    "Type.cpp"
    "Types.cpp"
    "UnitDefinition.cpp"
)

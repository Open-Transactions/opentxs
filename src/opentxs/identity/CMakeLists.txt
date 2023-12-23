# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(wot)

libopentxs_add_public_header("identity" "CredentialRole.hpp")
libopentxs_add_public_header("identity" "CredentialType.hpp")
libopentxs_add_public_header("identity" "IdentityType.hpp")
libopentxs_add_public_header("identity" "NymCapability.hpp")
libopentxs_add_public_header("identity" "SourceProofType.hpp")
libopentxs_add_public_header("identity" "SourceType.hpp")
libopentxs_add_public_header("identity" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "CredentialRole.cpp"
    "CredentialType.cpp"
    "NymCapability.cpp"
    "SourceProofType.cpp"
    "SourceType.cpp"
    "Types.cpp"
    "Types.internal.cpp"
    "Types.internal.hpp"
)

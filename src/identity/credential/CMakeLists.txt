# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("identity/credential" "Base.hpp")
libopentxs_add_public_header("identity/credential" "Contact.hpp")
libopentxs_add_public_header("identity/credential" "Key.hpp")
libopentxs_add_public_header("identity/credential" "Primary.hpp")
libopentxs_add_public_header("identity/credential" "Secondary.hpp")
libopentxs_add_public_header("identity/credential" "Verification.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/identity/credential/Blank.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/identity/credential/Credential.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/identity/credential/Types.hpp"
    "Base.cpp"
    "Base.tpp"
    "Base.tpp.cpp"
    "Contact.cpp"
    "Contact.hpp"
    "Credential.cpp"
    "Key.cpp"
    "Key.hpp"
    "Primary.cpp"
    "Primary.hpp"
    "Secondary.cpp"
    "Secondary.hpp"
    "Verification.cpp"
    "Verification.hpp"
)

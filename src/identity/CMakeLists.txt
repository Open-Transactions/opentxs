# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(credential)
add_subdirectory(wot)

libopentxs_add_public_header("identity" "Authority.hpp")
libopentxs_add_public_header("identity" "Nym.hpp")
libopentxs_add_public_header("identity" "Source.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/identity/Authority.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/identity/Nym.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/identity/Source.hpp"
    "Authority.cpp"
    "Authority.hpp"
    "Nym.cpp"
    "Nym.hpp"
    "Source.cpp"
    "Source.hpp"
)
libopentxs_link_internal(unofficial-sodium::sodium)
libopentxs_link_external(unofficial-sodium::sodium)

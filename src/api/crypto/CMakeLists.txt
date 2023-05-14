# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(base)
add_subdirectory(base58)
add_subdirectory(blockchain)

libopentxs_add_public_header("api/crypto" "Asymmetric.hpp")
libopentxs_add_public_header("api/crypto" "Config.hpp")
libopentxs_add_public_header("api/crypto" "Encode.hpp")
libopentxs_add_public_header("api/crypto" "Hash.hpp")
libopentxs_add_public_header("api/crypto" "Seed.hpp")
libopentxs_add_public_header("api/crypto" "Symmetric.hpp")
libopentxs_add_public_header("api/crypto" "Util.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_BINARY_DIR}/src/api/crypto/Config_generated.cpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/crypto/Asymmetric.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/crypto/Config.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/crypto/Encode.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/crypto/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/crypto/Hash.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/crypto/Null.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/crypto/Seed.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/crypto/Symmetric.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/crypto/Util.hpp"
    "Asymmetric.cpp"
    "Asymmetric.hpp"
    "Config.cpp"
    "Config.hpp"
    "Encode.cpp"
    "Encode.hpp"
    "Hash.cpp"
    "Hash.hpp"
    "Seed.cpp"
    "Seed.hpp"
    "Symmetric.cpp"
    "Symmetric.hpp"
)

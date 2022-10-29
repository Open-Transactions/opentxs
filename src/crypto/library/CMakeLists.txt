# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(openssl)

if(PACKETCRYPT_EXPORT)
  add_subdirectory(packetcrypt)
endif()

add_subdirectory(secp256k1)
add_subdirectory(sodium)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/crypto/library/AsymmetricProvider.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/library/EcdsaProvider.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/library/EncodingProvider.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/library/HashingProvider.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Null.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Pbkdf2.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Ripemd160.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Scrypt.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/library/SymmetricProvider.hpp"
    "AsymmetricProvider.cpp"
    "AsymmetricProvider.hpp"
    "EcdsaProvider.cpp"
    "EcdsaProvider.hpp"
    "HashingProvider.cpp"
)
opentxs_link_internal(unofficial-sodium::sodium)
opentxs_link_external(unofficial-sodium::sodium)

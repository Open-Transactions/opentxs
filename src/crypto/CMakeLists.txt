# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(asymmetric)
add_subdirectory(bip32)
add_subdirectory(hasher)
add_subdirectory(key)
add_subdirectory(library)
add_subdirectory(parameters)
add_subdirectory(symmetric)

libopentxs_add_public_header("crypto" "Bip32.hpp")
libopentxs_add_public_header("crypto" "Bip32Child.hpp")
libopentxs_add_public_header("crypto" "Bip39.hpp")
libopentxs_add_public_header("crypto" "Bip43Purpose.hpp")
libopentxs_add_public_header("crypto" "Bip44Type.hpp")
libopentxs_add_public_header("crypto" "HashType.hpp")
libopentxs_add_public_header("crypto" "Language.hpp")
libopentxs_add_public_header("crypto" "SecretStyle.hpp")
libopentxs_add_public_header("crypto" "Seed.hpp")
libopentxs_add_public_header("crypto" "SeedStrength.hpp")
libopentxs_add_public_header("crypto" "SeedStyle.hpp")
libopentxs_add_public_header("crypto" "SignatureRole.hpp")
libopentxs_add_public_header("crypto" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/crypto/Crypto.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/Envelope.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/Seed.hpp"
    "Bip39.cpp"
    "Bip39.hpp"
    "Crypto.cpp"
    "Envelope.cpp"
    "Envelope.hpp"
    "HDNode.cpp"
    "HDNode.hpp"
    "Seed.cpp"
    "Seed.hpp"
    "bip39_word_list.cpp"
)
libopentxs_link_internal(unofficial-sodium::sodium)
libopentxs_link_external(unofficial-sodium::sodium)

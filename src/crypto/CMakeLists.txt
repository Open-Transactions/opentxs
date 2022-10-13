# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(bip32)
add_subdirectory(key)
add_subdirectory(library)
add_subdirectory(parameters)

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
target_include_directories(
  opentxs-common SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
          "${opentxs_SOURCE_DIR}/deps/cs_libguarded/src"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/Bip32.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/Bip32Child.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/Bip39.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/Bip43Purpose.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/Bip44Type.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/HashType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/Language.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/SecretStyle.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/Seed.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/SeedStrength.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/SeedStyle.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/SignatureRole.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/Types.hpp"
)
target_link_libraries(opentxs-common PRIVATE unofficial-sodium::sodium)
target_link_libraries(opentxs PUBLIC unofficial-sodium::sodium)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/crypto
    COMPONENT dev
  )
endif()

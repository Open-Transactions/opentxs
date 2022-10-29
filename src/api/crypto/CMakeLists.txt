# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(base)
add_subdirectory(base58)
add_subdirectory(blockchain)

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
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Asymmetric.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Config.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Encode.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Hash.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Seed.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Symmetric.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Util.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api/crypto
    COMPONENT dev
  )
endif()

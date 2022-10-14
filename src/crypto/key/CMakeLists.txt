# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(asymmetric)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/crypto/key/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/key/Keypair.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/key/Null.hpp"
    "Key.cpp"
)
target_include_directories(
  opentxs-common SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/Asymmetric.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/EllipticCurve.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/Ed25519.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/HD.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/RSA.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/Secp256k1.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/Types.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/crypto/key
    COMPONENT dev
  )
endif()

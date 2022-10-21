# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/crypto/asymmetric/key/Secp256k1.hpp"
    "Imp.cpp"
    "Imp.hpp"
    "Secp256k1.cpp"
    "Secp256k1Private.cpp"
    "Secp256k1Private.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/asymmetric/key/Secp256k1.hpp"
)

if(SECP256K1_EXPORT)
  target_sources(opentxs-common PRIVATE "Enabled.cpp")
else()
  target_sources(opentxs-common PRIVATE "Disabled.cpp")
endif()

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/crypto/asymmetric/key
    COMPONENT dev
  )
endif()

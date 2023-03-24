# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(opentxs-common PRIVATE "Hasher.cpp" "HasherPrivate.hpp")

if(OPENSSL_EXPORT)
  target_sources(opentxs-common PRIVATE "HasherPrivate.cpp")
else()
  target_sources(opentxs-common PRIVATE "HasherPrivateNoOpenSSL.cpp")
endif()

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/Hasher.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/crypto
    COMPONENT dev
  )
endif()

# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/crypto/symmetric/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/symmetric/Key.hpp"
    "Common.cpp"
    "Factory.cpp"
    "Key.cpp"
    "KeyPrivate.cpp"
    "KeyPrivate.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/symmetric/Algorithm.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/symmetric/Key.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/symmetric/Source.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/symmetric/Types.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/crypto/symmetric
    COMPONENT dev
  )
endif()

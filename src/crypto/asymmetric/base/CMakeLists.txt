# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/crypto/asymmetric/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/asymmetric/Key.hpp"
    "Imp.cpp"
    "Imp.hpp"
    "Key.cpp"
    "KeyPrivate.cpp"
    "KeyPrivate.hpp"
)
target_include_directories(
  opentxs-common SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/cs_libguarded/src"
                                "${opentxs_SOURCE_DIR}/deps/frozen/include"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/asymmetric/Key.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/crypto/asymmetric
    COMPONENT dev
  )
endif()

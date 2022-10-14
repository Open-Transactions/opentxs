# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/identity/credential/Blank.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/identity/credential/Credential.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/identity/credential/Types.hpp"
    "Base.cpp"
    "Base.tpp"
    "Contact.cpp"
    "Contact.hpp"
    "Credential.cpp"
    "Key.cpp"
    "Key.hpp"
    "Primary.cpp"
    "Primary.hpp"
    "Secondary.cpp"
    "Secondary.hpp"
    "Verification.cpp"
    "Verification.hpp"
)
target_include_directories(
  opentxs-common SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/credential/Base.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/credential/Contact.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/credential/Key.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/credential/Primary.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/credential/Secondary.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/credential/Verification.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/identity/credential
    COMPONENT dev
  )
endif()

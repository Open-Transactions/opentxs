# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(credential)
add_subdirectory(wot)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/identity/Authority.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/identity/Nym.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/identity/Source.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/identity/Types.hpp"
    "Authority.cpp"
    "Authority.hpp"
    "Identity.cpp"
    "Nym.cpp"
    "Nym.hpp"
    "Source.cpp"
    "Source.hpp"
)
target_include_directories(
  opentxs-common SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/Authority.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/CredentialRole.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/CredentialType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/IdentityType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/Nym.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/Source.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/SourceProofType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/SourceType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/Types.hpp"
)
target_link_libraries(opentxs-common PRIVATE unofficial-sodium::sodium)
target_link_libraries(opentxs PUBLIC unofficial-sodium::sodium)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/identity
    COMPONENT dev
  )
endif()

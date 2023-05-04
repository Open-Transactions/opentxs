# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/core/identifier/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/identifier/Identifier.hpp"
    "Account.cpp"
    "Generic.cpp"
    "IdentifierPrivate.cpp"
    "IdentifierPrivate.hpp"
    "Notary.cpp"
    "Nym.cpp"
    "UnitDefinition.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/core/identifier/Account.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/identifier/AccountSubtype.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/identifier/Algorithm.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/identifier/Generic.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/identifier/Notary.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/identifier/Nym.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/identifier/Type.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/identifier/Types.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/identifier/UnitDefinition.hpp"
)
opentxs_link_internal(opentxs-core-otprotob)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core/identifier
    COMPONENT dev
  )
endif()

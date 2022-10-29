# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/identity/wot/claim/Types.hpp"
    "Data.cpp"
    "Group.cpp"
    "Internal.cpp"
    "Item.cpp"
    "Section.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/wot/claim/Attribute.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/wot/claim/ClaimType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/wot/claim/Data.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/wot/claim/Group.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/wot/claim/Item.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/wot/claim/Section.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/wot/claim/SectionType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/wot/claim/Types.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/identity/wot/claim
    COMPONENT dev
  )
endif()

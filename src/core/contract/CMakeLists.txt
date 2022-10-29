# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(peer)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/BasketContract.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/Blank.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/Contract.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/CurrencyContract.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/SecurityContract.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/ServerContract.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/Types.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/Unit.hpp"
    "BasketContract.cpp"
    "BasketContract.hpp"
    "Common.cpp"
    "Contract.cpp"
    "CurrencyContract.cpp"
    "CurrencyContract.hpp"
    "SecurityContract.cpp"
    "SecurityContract.hpp"
    "ServerContract.cpp"
    "ServerContract.hpp"
    "Signable.cpp"
    "Signable.hpp"
    "Unit.cpp"
    "Unit.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/ContractType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/ProtocolVersion.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/Signable.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/Types.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/UnitType.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core/contract
    COMPONENT dev
  )
endif()

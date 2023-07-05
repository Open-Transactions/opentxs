# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/request/Bailment.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/request/BailmentNotice.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/request/Base.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/request/Connection.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/request/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/request/Faucet.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/request/Outbailment.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/request/StoreSecret.hpp"
    "Bailment.cpp"
    "Bailment.hpp"
    "BailmentNotice.cpp"
    "BailmentNotice.hpp"
    "Base.cpp"
    "Base.hpp"
    "Connection.cpp"
    "Connection.hpp"
    "Factory.cpp"
    "Faucet.cpp"
    "Faucet.hpp"
    "Outbailment.cpp"
    "Outbailment.hpp"
    "StoreSecret.cpp"
    "StoreSecret.hpp"
)

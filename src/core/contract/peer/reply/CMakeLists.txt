# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/reply/Acknowledgement.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/reply/Bailment.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/reply/Base.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/reply/Connection.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/reply/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/reply/Faucet.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/reply/Outbailment.hpp"
    "Acknowledgement.cpp"
    "Acknowledgement.hpp"
    "Bailment.cpp"
    "Bailment.hpp"
    "Base.cpp"
    "Base.hpp"
    "Connection.cpp"
    "Connection.hpp"
    "Factory.cpp"
    "Faucet.cpp"
    "Faucet.hpp"
    "Outbailment.cpp"
    "Outbailment.hpp"
)

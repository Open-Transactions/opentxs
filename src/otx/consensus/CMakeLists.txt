# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/otx/consensus/Base.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/consensus/Client.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/consensus/Consensus.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/consensus/ManagedNumber.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/consensus/Server.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/consensus/TransactionStatement.hpp"
    "Base.cpp"
    "Base.hpp"
    "Client.cpp"
    "Client.hpp"
    "ClientPrivate.cpp"
    "ClientPrivate.hpp"
    "ConsensusPrivate.cpp"
    "ConsensusPrivate.hpp"
    "ManagedNumber.cpp"
    "ManagedNumber.hpp"
    "Server.cpp"
    "Server.hpp"
    "ServerPrivate.cpp"
    "ServerPrivate.hpp"
    "TransactionStatement.cpp"
)

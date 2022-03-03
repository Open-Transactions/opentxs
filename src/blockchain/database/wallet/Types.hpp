// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"

namespace opentxs::blockchain::database::wallet
{
using Parent = node::internal::WalletDatabase;
using SubchainIndex = Parent::SubchainIndex;
using pSubchainIndex = Parent::pSubchainIndex;
using NodeID = Parent::NodeID;
using pNodeID = Parent::pNodeID;
using Subchain = Parent::Subchain;
using Patterns = Parent::Patterns;
using ElementMap = Parent::ElementMap;
using MatchingIndices = Parent::MatchingIndices;
using PatternID = Identifier;
using pPatternID = OTIdentifier;
}  // namespace opentxs::blockchain::database::wallet

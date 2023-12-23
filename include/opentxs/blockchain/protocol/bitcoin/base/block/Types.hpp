// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/block/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Hash;
class TransactionHash;
class Outpoint;
class Position;
}  // namespace block
}  // namespace blockchain

class ByteArray;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::protocol::bitcoin::base::block
{
using blockchain::block::Hash;
using blockchain::block::Height;
using blockchain::block::Outpoint;
using blockchain::block::Position;
using blockchain::block::TransactionHash;
using WitnessItem = ByteArray;
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block

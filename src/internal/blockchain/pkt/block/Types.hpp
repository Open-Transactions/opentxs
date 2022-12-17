// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <tuple>

#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Header;
}  // namespace block
}  // namespace bitcoin
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::pkt::block
{
using bitcoin::block::CalculatedSize;
using bitcoin::block::Header;
using bitcoin::block::TransactionMap;
using bitcoin::block::TxidIndex;

using ProofType = std::uint8_t;
using Proof = std::pair<ProofType, ByteArray>;
using Proofs = Vector<Proof>;

static constexpr auto terminal_proof_ = ProofType{0x0};
}  // namespace opentxs::blockchain::pkt::block

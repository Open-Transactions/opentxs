// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/bitcoin/block/Opcodes.hpp"

#pragma once

#include "opentxs/blockchain/block/Types.hpp"

#include "internal/blockchain/block/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Hash;
class Outpoint;
class Position;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block
{
using blockchain::block::Element;
using blockchain::block::ElementHash;
using blockchain::block::ElementHashes;
using blockchain::block::Elements;
using blockchain::block::Hash;
using blockchain::block::Height;
using blockchain::block::InputMatches;
using blockchain::block::KeyData;
using blockchain::block::Matches;
using blockchain::block::Outpoint;
using blockchain::block::OutputMatches;
using blockchain::block::ParsedPatterns;
using blockchain::block::Patterns;
using blockchain::block::Position;
using blockchain::block::pTxid;
using blockchain::block::Txid;
}  // namespace opentxs::blockchain::bitcoin::block

namespace opentxs::blockchain::bitcoin::block::internal
{
auto DecodeBip34(const ReadView coinbase) noexcept -> block::Height;
auto EncodeBip34(block::Height height) noexcept -> Space;
auto Opcode(const OP opcode) noexcept(false) -> ScriptElement;
auto PushData(const ReadView data) noexcept(false) -> ScriptElement;
}  // namespace opentxs::blockchain::bitcoin::block::internal

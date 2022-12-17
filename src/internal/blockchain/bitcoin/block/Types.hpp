// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/block/TransactionHash.hpp"
// IWYU pragma: no_include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"

#pragma once

#include <cstddef>
#include <span>
#include <utility>

#include "internal/blockchain/block/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace blockchain
{

namespace block
{
class Hash;
class TransactionHash;
}  // namespace block
}  // namespace blockchain

namespace network
{
namespace blockchain
{
namespace bitcoin
{
class CompactSize;
}  // namespace bitcoin
}  // namespace blockchain
}  // namespace network

class Data;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block
{
using blockchain::block::Element;
using blockchain::block::ElementHash;
using blockchain::block::ElementHashes;
using blockchain::block::Elements;
using blockchain::block::InputMatches;
using blockchain::block::KeyData;
using blockchain::block::Matches;
using blockchain::block::OutputMatches;
using blockchain::block::ParsedPatterns;
using blockchain::block::Patterns;
using blockchain::block::TransactionHash;
using blockchain::block::TransactionMap;
using blockchain::block::TxidIndex;

using CalculatedSize =
    std::pair<std::size_t, network::blockchain::bitcoin::CompactSize>;
using ScriptElements = Vector<script::Element>;

auto CalculateMerkleHash(
    const api::Crypto& crypto,
    const Type chain,
    const Data& lhs,
    const Data& rhs,
    Writer&& out) noexcept(false) -> bool;
auto CalculateMerkleRow(
    const api::Crypto& crypto,
    const Type chain,
    const std::span<const TransactionHash> in,
    Vector<TransactionHash>& out) noexcept(false) -> bool;
auto CalculateMerkleValue(
    const api::Crypto& crypto,
    const Type chain,
    const std::span<const TransactionHash> txids) noexcept(false) -> Hash;
}  // namespace opentxs::blockchain::bitcoin::block

namespace opentxs::blockchain::bitcoin::block::internal
{
auto DecodeBip34(const ReadView coinbase) noexcept -> block::Height;
auto EncodeBip34(block::Height height) noexcept -> Space;
auto Opcode(const script::OP opcode) noexcept(false) -> script::Element;
auto PushData(const ReadView data) noexcept(false) -> script::Element;
}  // namespace opentxs::blockchain::bitcoin::block::internal

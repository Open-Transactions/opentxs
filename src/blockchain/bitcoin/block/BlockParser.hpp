// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

#include "blockchain/bitcoin/block/Block.hpp"
#include "blockchain/bitcoin/block/BlockParser.hpp"
#include "blockchain/block/Block.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/bitcoin/block/Block.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
namespace internal
{
class Header;
}  // namespace internal

class Block;
class Header;
class Transaction;
}  // namespace block
}  // namespace bitcoin
}  // namespace blockchain

template <typename ArrayType>
auto reader(const ArrayType& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(in.data()), in.size()};
}
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
using BlockReturnType = blockchain::bitcoin::block::implementation::Block;
using ByteIterator = const std::byte*;
using ParsedTransactions =
    std::pair<BlockReturnType::TxidIndex, BlockReturnType::TransactionMap>;

auto parse_header(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const ReadView in,
    ByteIterator& it,
    std::size_t& expectedSize)
    -> std::unique_ptr<blockchain::bitcoin::block::Header>;
auto parse_normal_block(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const ReadView in) noexcept(false)
    -> std::shared_ptr<blockchain::bitcoin::block::Block>;
auto parse_pkt_block(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const ReadView in) noexcept(false)
    -> std::shared_ptr<blockchain::bitcoin::block::Block>;
auto parse_transactions(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const ReadView in,
    const blockchain::bitcoin::block::Header& header,
    BlockReturnType::CalculatedSize& sizeData,
    ByteIterator& it,
    std::size_t& expectedSize) -> ParsedTransactions;
}  // namespace opentxs::factory

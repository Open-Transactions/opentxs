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

#include "blockchain/block/Block.hpp"
#include "blockchain/block/bitcoin/Block.hpp"
#include "blockchain/block/bitcoin/BlockParser.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
namespace internal
{
struct Header;
}  // namespace internal

class Block;
class Header;
class Transaction;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

template <typename ArrayType>
auto reader(const ArrayType& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(in.data()), in.size()};
}
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
using BlockReturnType = blockchain::block::bitcoin::implementation::Block;
using ByteIterator = const std::byte*;
using ParsedTransactions =
    std::pair<BlockReturnType::TxidIndex, BlockReturnType::TransactionMap>;

auto parse_header(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView in,
    ByteIterator& it,
    std::size_t& expectedSize)
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>;
auto parse_normal_block(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView in) noexcept(false)
    -> std::shared_ptr<blockchain::block::bitcoin::Block>;
auto parse_pkt_block(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView in) noexcept(false)
    -> std::shared_ptr<blockchain::block::bitcoin::Block>;
auto parse_transactions(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView in,
    const blockchain::block::bitcoin::Header& header,
    BlockReturnType::CalculatedSize& sizeData,
    ByteIterator& it,
    std::size_t& expectedSize) -> ParsedTransactions;
}  // namespace opentxs::factory

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/bitcoin/block/Block.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/SendResult.hpp"

#pragma once

#include <cstdint>
#include <future>
#include <memory>
#include <string_view>
#include <utility>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
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
class Output;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Block;
class Outpoint;
class TransactionHash;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node
{
using TypeEnum = std::uint32_t;

enum class SendResult : TypeEnum;
enum class TxoState : std::uint16_t;
enum class TxoTag : std::uint16_t;

using BlockResult = std::shared_future<block::Block>;
using BlockResults = Vector<BlockResult>;
using SendOutcome = std::pair<SendResult, block::TransactionHash>;
using UTXO = std::pair<block::Outpoint, bitcoin::block::Output>;

OPENTXS_EXPORT auto print(SendResult) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(TxoState) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(TxoTag) noexcept -> std::string_view;
}  // namespace opentxs::blockchain::node

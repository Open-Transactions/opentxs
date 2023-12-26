// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/protocol/bitcoin/base/block/Block.hpp"

#pragma once

#include <cstdint>
#include <future>
#include <string_view>
#include <utility>

#include "opentxs/Export.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Block;
class Outpoint;
class TransactionHash;
}  // namespace block

namespace protocol
{
namespace bitcoin
{
namespace base
{
namespace block
{
class Output;
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node
{
enum class Funding : std::uint32_t;     // IWYU pragma: export
enum class SendResult : std::uint32_t;  // IWYU pragma: export
enum class TxoState : std::uint16_t;    // IWYU pragma: export
enum class TxoTag : std::uint16_t;      // IWYU pragma: export

using BlockResult = std::shared_future<block::Block>;
using BlockResults = Vector<BlockResult>;
using SendOutcome = std::pair<SendResult, block::TransactionHash>;
using PendingOutgoing = std::future<SendOutcome>;
using UTXO = std::pair<block::Outpoint, protocol::bitcoin::base::block::Output>;

OPENTXS_EXPORT auto print(Funding) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(SendResult) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(TxoState) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(TxoTag) noexcept -> std::string_view;
}  // namespace opentxs::blockchain::node

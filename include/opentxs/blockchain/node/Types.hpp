// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/block/bitcoin/Block.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/SendResult.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <future>
#include <memory>
#include <string_view>
#include <utility>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Block;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node
{
using TypeEnum = std::uint32_t;

enum class SendResult : TypeEnum;
enum class TxoState : std::uint16_t;
enum class TxoTag : std::uint16_t;

using BitcoinBlockResult =
    std::shared_future<std::shared_ptr<const block::bitcoin::Block>>;
using BitcoinBlockResults = Vector<BitcoinBlockResult>;
using SendOutcome = std::pair<SendResult, block::pTxid>;

OPENTXS_EXPORT auto print(SendResult) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(TxoState) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(TxoTag) noexcept -> std::string_view;
}  // namespace opentxs::blockchain::node

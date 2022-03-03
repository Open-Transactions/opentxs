// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "internal/blockchain/node/wallet/FeeOracle.hpp"
// IWYU pragma: no_include "internal/blockchain/node/wallet/FeeSource.hpp"
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <string_view>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

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
namespace node
{
namespace wallet
{
class FeeOracle;
class FeeSource;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto FeeOracle(
    const api::Session& api,
    const blockchain::Type chain,
    alloc::Resource* alloc = nullptr) noexcept
    -> blockchain::node::wallet::FeeOracle;
auto FeeSources(
    const api::Session& api,
    const blockchain::Type chain,
    const std::string_view endpoint,
    alloc::Resource* alloc = nullptr) noexcept
    -> ForwardList<blockchain::node::wallet::FeeSource>;
auto BTCFeeSources(
    const api::Session& api,
    const std::string_view endpoint,
    alloc::Resource* alloc = nullptr) noexcept
    -> ForwardList<blockchain::node::wallet::FeeSource>;
}  // namespace opentxs::factory

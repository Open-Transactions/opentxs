// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "internal/blockchain/node/wallet/FeeOracle.hpp"
// IWYU pragma: no_include "internal/blockchain/node/wallet/FeeSource.hpp"

#pragma once

#include <memory>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto FeeSources(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const blockchain::node::Manager> node) noexcept -> void;
auto BTCFeeSources(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const blockchain::node::Manager> node) noexcept -> void;
}  // namespace opentxs::factory

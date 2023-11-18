// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/Amount.hpp"

#pragma once

#include <memory>
#include <optional>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Session;
}  // namespace internal
}  // namespace api

namespace blockchain
{
namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain

class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class FeeOracle
{
public:
    class Actor;
    class Shared;

    auto EstimatedFee() const noexcept -> std::optional<Amount>;

    FeeOracle(
        std::shared_ptr<const api::internal::Session> api,
        std::shared_ptr<const node::Manager> node) noexcept;
    FeeOracle() = delete;
    FeeOracle(const FeeOracle&) = delete;
    FeeOracle(FeeOracle&&) noexcept;
    auto operator=(const FeeOracle&) -> FeeOracle& = delete;
    auto operator=(FeeOracle&&) -> FeeOracle& = delete;

    ~FeeOracle();

private:
    std::shared_ptr<Shared> shared_;
};
}  // namespace opentxs::blockchain::node::wallet

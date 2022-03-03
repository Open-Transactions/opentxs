// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/Amount.hpp"

#pragma once

#include <optional>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace node
{
namespace wallet
{
class FeeOracle;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain

class Amount;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::blockchain::node::wallet::FeeOracle final : public Allocated
{
public:
    class Imp;

    auto EstimatedFee() const noexcept -> std::optional<Amount>;
    auto get_allocator() const noexcept -> allocator_type final;

    auto Shutdown() noexcept -> void;

    FeeOracle(Imp* imp) noexcept;
    FeeOracle() = delete;
    FeeOracle(const FeeOracle&) = delete;
    FeeOracle(FeeOracle&) = delete;
    auto operator=(const FeeOracle&) -> FeeOracle& = delete;
    auto operator=(FeeOracle&) -> FeeOracle& = delete;

    ~FeeOracle() final;

private:
    Imp* imp_;
};

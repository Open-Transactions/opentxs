// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <future>
#include <utility>

#include "opentxs/blockchain/block/TransactionHash.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::node::wallet
{
class Pending
{
public:
    using Promise = std::promise<SendOutcome>;
    using Data = std::pair<identifier::Generic, Promise>;

    auto Exists(const identifier::Generic& id) const noexcept -> bool;
    auto HasData() const noexcept -> bool;

    auto Add(
        const identifier::Generic& id,
        std::promise<SendOutcome>&& promise) noexcept -> bool;
    auto Add(Data&& job) noexcept -> void;
    auto Delete(const identifier::Generic& id) noexcept -> void;
    auto Pop() noexcept -> Data;

    Pending() noexcept;

private:
    struct PrivateData {
        UnallocatedDeque<Data> data_{};
        Set<identifier::Generic> ids_{};
    };

    using Guarded = libguarded::plain_guarded<PrivateData>;

    mutable Guarded data_;
};
}  // namespace opentxs::blockchain::node::wallet

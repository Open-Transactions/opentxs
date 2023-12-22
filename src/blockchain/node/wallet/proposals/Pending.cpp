// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/proposals/Pending.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "opentxs/blockchain/node/SendResult.hpp"  // IWYU pragma: keep
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::node::wallet
{
Pending::Pending() noexcept
    : data_()
{
}

auto Pending::Add(
    const identifier::Generic& id,
    std::promise<SendOutcome>&& promise) noexcept -> bool
{
    auto handle = data_.lock();
    auto& data = *handle;

    if (data.ids_.contains(id)) {
        LogError()()("Proposal already exists").Flush();
        promise.set_value(
            {SendResult::DuplicateProposal, block::TransactionHash{}});

        return false;
    }

    data.ids_.emplace(id);
    data.data_.emplace_back(id, std::move(promise));

    return true;
}

auto Pending::Add(Data&& job) noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;
    const auto& [id, promise] = job;

    assert_true(false == data.ids_.contains(id));

    data.ids_.emplace(id);
    data.data_.emplace_back(std::move(job));
}

auto Pending::Delete(const identifier::Generic& id) noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;
    auto copy = identifier::Generic{id};

    if (data.ids_.contains(copy)) {
        data.ids_.erase(copy);

        for (auto i{data.data_.begin()}; i != data.data_.end();) {
            if (i->first == copy) {
                i = data.data_.erase(i);
            } else {
                ++i;
            }
        }
    }
}

auto Pending::Exists(const identifier::Generic& id) const noexcept -> bool
{
    return data_.lock()->ids_.contains(id);
}

auto Pending::HasData() const noexcept -> bool
{
    return false == data_.lock()->data_.empty();
}

auto Pending::Pop() noexcept -> Data
{
    auto handle = data_.lock();
    auto& data = *handle;
    auto post = ScopeGuard{[&] { data.data_.pop_front(); }};
    auto& job = data.data_.front();
    const auto& [id, promise] = job;
    data.ids_.erase(id);

    return std::move(data.data_.front());
}
}  // namespace opentxs::blockchain::node::wallet

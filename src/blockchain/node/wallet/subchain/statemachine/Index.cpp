// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/Index.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <stdexcept>
#include <type_traits>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Rescan.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Scan.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::blockchain::node::wallet
{
Index::Index(
    SubchainStateData& parent,
    Scan& scan,
    Rescan& rescan,
    Progress& progress) noexcept
    : Job(ThreadPool::General, parent)
    , scan_(scan)
    , rescan_(rescan)
    , progress_(progress)
    , last_indexed_(parent_.db_.SubchainLastIndexed(parent_.db_key_))
    , first_(true)
    , queue_()
{
}

auto Index::done(
    const block::Position& original,
    const node::internal::WalletDatabase::ElementMap& elements) noexcept -> void
{
    const auto& db = parent_.db_;
    const auto& index = parent_.db_key_;
    db.SubchainAddElements(index, elements);
    auto lock = Lock{lock_};
    last_indexed_ = parent_.db_.SubchainLastIndexed(index);

    while (0 < queue_.size()) {
        const auto& [pos, matches] = queue_.front();
        rescan(lock, pos, matches);
        queue_.pop();
    }

    if (first_) {
        ready_for_scan();
    } else {
        rescan(lock, original, 1);
    }

    finish(lock);
}

auto Index::index_element(
    const filter::Type type,
    const blockchain::crypto::Element& input,
    const Bip32Index index,
    node::internal::WalletDatabase::ElementMap& output) const noexcept -> void
{
    parent_.index_element(type, input, index, output);
}

auto Index::Processed(const ProgressBatch& data) noexcept -> void
{
    auto lock = Lock{lock_};

    for (const auto& [position, matches] : data) {
        queue_.emplace(position, matches);
    }
}

auto Index::ready_for_scan() noexcept -> void
{
    if (first_) {
        first_ = false;
        scan_.SetReady();
    }
}

auto Index::Reorg(const block::Position&) noexcept -> void
{
    // NOTE no action necessary
}

auto Index::rescan(
    const Lock& lock,
    const block::Position& pos,
    const std::size_t matches) noexcept -> void
{
    LogVerbose()(OT_PRETTY_CLASS())(parent_.name_)(" processing for block ")(
        pos.second->asHex())(" at height ")(pos.first)(" found ")(
        matches)(" new matches");
    const auto interval = [&]() -> block::Height {
        if (0 < matches) {

            return lookback_;
        } else {

            return -1;
        }
    }();
    const auto height = std::max<block::Height>(0, pos.first - interval);
    const auto rescan =
        block::Position{height, parent_.node_.HeaderOracle().BestHash(height)};
    rescan_.Queue(rescan);
}

auto Index::Run() noexcept -> bool
{
    auto again{false};

    try {
        const auto data = [this] {
            auto lock = Lock{lock_};

            if (is_running(lock)) {
                throw std::runtime_error{"index job is already running"};
            }

            return std::make_pair(last_indexed_, 0 < queue_.size());
        }();
        const auto& [current, queue] = data;
        // NOTE below this line we can assume only one Index thread is running

        if (const auto t = need_index(current); t.has_value() || queue) {
            static constexpr auto job{"index"};

            OT_ASSERT(t.has_value() || current.has_value());

            const auto pos = t.has_value() ? t.value() : current.value();
            again = queue_work([=] { Do(data.first, pos); }, job, false);
        } else {
            ready_for_scan();
            again = false;
        }
    } catch (...) {
        again = true;
    }

    return again;
}
}  // namespace opentxs::blockchain::node::wallet

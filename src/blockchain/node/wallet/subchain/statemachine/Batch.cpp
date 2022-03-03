// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/Batch.hpp"  // IWYU pragma: associated

#include <atomic>
#include <memory>

#include "blockchain/node/wallet/subchain/statemachine/Index.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Progress.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Work.hpp"
#include "internal/blockchain/node/Node.hpp"

namespace opentxs::blockchain::node::wallet
{
Batch::Batch() noexcept
    : id_(NextID())
    , reported_(false)
    , running_(0)
    , jobs_()
{
    jobs_.reserve(max_);
}

auto Batch::AddJob(const block::Position& position) noexcept -> Work*
{
    return jobs_.emplace_back(std::make_unique<Work>(position, *this)).get();
}

auto Batch::CompleteJob() noexcept -> void { --running_; }

auto Batch::Finalize() noexcept -> void { running_ = jobs_.size(); }

auto Batch::IsFinished() const noexcept -> bool
{
    if (reported_) { return false; }

    if (0u == running_) {
        reported_ = true;

        return true;
    }

    return false;
}

auto Batch::IsFull() const noexcept -> bool { return max_ == jobs_.size(); }

auto Batch::NextID() noexcept -> ID
{
    static auto counter = std::atomic<ID>{};

    return ++counter;
}

auto Batch::UpdateProgress(Progress& progress, Index& index) const noexcept
    -> void
{
    const auto data = [this] {
        auto out = ProgressBatch{};

        for (auto& job : jobs_) { job->GetProgress(out); }

        return out;
    }();

    progress.UpdateProcess(data);
    index.Processed(data);
}

auto Batch::Write(const Identifier& key, const internal::WalletDatabase& db)
    const noexcept -> bool
{
    const auto results = [this] {
        auto out = Results{};

        for (auto& job : jobs_) { job->GetResults(out); }

        return out;
    }();

    return db.SubchainMatchBlock(key, results);
}

Batch::~Batch() = default;
}  // namespace opentxs::blockchain::node::wallet

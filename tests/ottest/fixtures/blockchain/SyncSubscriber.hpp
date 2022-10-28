// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <atomic>
#include <future>
#include <memory>

#include "ottest/Basic.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace ottest
{
class MinedBlocks;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
struct OPENTXS_EXPORT SyncSubscriber {
    using BlockHash = ot::blockchain::block::Hash;
    using SyncPromise = std::promise<BlockHash>;
    using SyncFuture = std::shared_future<BlockHash>;

    std::atomic_int expected_;

    auto wait(const bool hard = true) noexcept -> bool;

    SyncSubscriber(
        const ot::api::session::Client& api,
        const MinedBlocks& cache);

    ~SyncSubscriber();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace ottest

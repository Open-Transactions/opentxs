// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <utility>

#include "blockchain/node/wallet/subchain/statemachine/Job.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
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
namespace internal
{
struct WalletDatabase;
}  // namespace internal

namespace wallet
{
class Index;
class Progress;
class Work;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain

class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Batch
{
public:
    using ID = Cookie;

    const ID id_;

    auto IsFinished() const noexcept -> bool;
    auto IsFull() const noexcept -> bool;
    auto UpdateProgress(Progress& progress, Index& index) const noexcept
        -> void;
    auto Write(const Identifier& key, const internal::WalletDatabase& db)
        const noexcept -> bool;

    auto AddJob(const block::Position& position) noexcept -> Work*;
    auto CompleteJob() noexcept -> void;
    auto Finalize() noexcept -> void;

    Batch() noexcept;

    ~Batch();

private:
    static constexpr std::size_t max_{25};

    mutable std::atomic_bool reported_;
    std::atomic<std::size_t> running_;
    UnallocatedVector<std::unique_ptr<Work>> jobs_;

    static auto NextID() noexcept -> ID;
};
}  // namespace opentxs::blockchain::node::wallet

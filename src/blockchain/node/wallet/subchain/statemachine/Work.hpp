// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>

#include "blockchain/node/wallet/subchain/statemachine/Job.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"

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
class Batch;
class SubchainStateData;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Work
{
public:
    const Cookie id_;
    const block::Position position_;

    auto GetProgress(ProgressBatch& out) noexcept -> void;
    auto GetResults(Results& out) noexcept -> void;
    auto IsReady() const noexcept -> bool;

    auto Do(SubchainStateData& parent) noexcept -> bool;
    auto DownloadBlock(const BlockOracle& oracle) noexcept -> void;

    Work(const block::Position& position, Batch& batch) noexcept;

    ~Work();

private:
    Batch& batch_;
    BlockOracle::BitcoinBlockFuture block_;
    Indices matches_;
    bool processed_;
    std::size_t match_count_;

    static auto NextCookie() noexcept -> Cookie;
};
}  // namespace opentxs::blockchain::node::wallet

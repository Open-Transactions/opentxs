// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <future>
#include <memory>

#include "ottest/Basic.hpp"

namespace ottest
{
class OPENTXS_EXPORT BlockchainStartup
{
public:
    using Future = std::shared_future<void>;

    auto BlockOracle() const noexcept -> Future;
    auto BlockOracleDownloader() const noexcept -> Future;
    auto FeeOracle() const noexcept -> Future;
    auto FilterOracle() const noexcept -> Future;
    auto FilterOracleFilterDownloader() const noexcept -> Future;
    auto FilterOracleHeaderDownloader() const noexcept -> Future;
    auto FilterOracleIndexer() const noexcept -> Future;
    auto Node() const noexcept -> Future;
    auto PeerManager() const noexcept -> Future;
    auto SyncServer() const noexcept -> Future;
    auto Wallet() const noexcept -> Future;

    BlockchainStartup(
        const ot::api::Session& api,
        const ot::blockchain::Type chain) noexcept;

    ~BlockchainStartup();

private:
    class Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace ottest

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <atomic>
#include <memory>

#include "util/LMDB.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin
}  // namespace block

namespace node
{
namespace wallet
{
class SubchainStateData;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain

class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Subchain
{
public:
    enum class State {
        normal,
        pre_reorg,
        reorg,
        post_reorg,
        pre_shutdown,
        shutdown,
    };

    virtual auto VerifyState(const State state) const noexcept -> void = 0;

    virtual auto Init(boost::shared_ptr<SubchainStateData> me) noexcept
        -> void = 0;
    virtual auto ProcessReorg(
        const Lock& headerOracleLock,
        storage::lmdb::LMDB::Transaction& tx,
        std::atomic_int& errors,
        const block::Position& ancestor) noexcept -> void = 0;

    virtual ~Subchain() = default;

protected:
    Subchain() = default;

private:
    Subchain(const Subchain&) = delete;
    Subchain(Subchain&&) = delete;
    Subchain& operator=(const Subchain&) = delete;
    Subchain& operator=(Subchain&&) = delete;
};
}  // namespace opentxs::blockchain::node::wallet

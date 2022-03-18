// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/node/wallet/subchain/statemachine/Index.hpp"

#include <boost/smart_ptr/shared_ptr.hpp>
#include <atomic>
#include <optional>

#include "blockchain/node/wallet/subchain/statemachine/Job.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "util/Actor.hpp"

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
class SubchainStateData;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Index::Imp : public statemachine::Job
{
public:
    auto ProcessReorg(const block::Position& parent) noexcept -> void final;

    Imp(const SubchainStateData& parent,
        const network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    Imp& operator=(const Imp&) = delete;
    Imp& operator=(Imp&&) = delete;

    ~Imp() override = default;

protected:
    auto done(
        const node::internal::WalletDatabase::ElementMap& elements) noexcept
        -> void;

private:
    network::zeromq::socket::Raw& to_rescan_;
    std::optional<Bip32Index> last_indexed_;

    virtual auto need_index(const std::optional<Bip32Index>& current)
        const noexcept -> std::optional<Bip32Index> = 0;

    auto do_startup() noexcept -> void final;
    virtual auto process(
        const std::optional<Bip32Index>& current,
        Bip32Index target) noexcept -> void = 0;
    auto process_key(Message&& in) noexcept -> void final;
    auto process_update(Message&& msg) noexcept -> void final;
    auto work() noexcept -> bool final;
};
}  // namespace opentxs::blockchain::node::wallet

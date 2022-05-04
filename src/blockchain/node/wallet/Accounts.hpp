// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string_view>
#include <utility>

#include "internal/blockchain/node/wallet/Account.hpp"
#include "internal/blockchain/node/wallet/Accounts.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/Subchain.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Actor.hpp"
#include "util/LMDB.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace node
{
namespace internal
{
struct Mempool;
struct Network;
struct WalletDatabase;
}  // namespace internal

namespace wallet
{
class Account;
class NotificationStateData;
class Subchain;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

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

class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Accounts::Imp final : public Actor<AccountsJobs>
{
public:
    auto Init(boost::shared_ptr<Imp> me) noexcept -> void
    {
        signal_startup(me);
    }
    auto Shutdown() noexcept -> void;

    Imp(const api::Session& api,
        const node::internal::Network& node,
        node::internal::WalletDatabase& db,
        const node::internal::Mempool& mempool,
        const network::zeromq::BatchID batch,
        const Type chain,
        allocator_type alloc) noexcept;

    ~Imp() final;

protected:
    auto do_startup() noexcept -> void override;
    auto do_shutdown() noexcept -> void override;
    auto pipeline(const Work work, Message&& msg) noexcept -> void override;
    auto work() noexcept -> bool override;

private:
    enum class State {
        normal,
        shutdown,
    };

    using AccountMap = Map<OTNymID, wallet::Account>;

    const api::Session& api_;
    const node::internal::Network& node_;
    node::internal::WalletDatabase& db_;
    const node::internal::Mempool& mempool_;
    const Type chain_;
    const cfilter::Type filter_type_;
    const CString shutdown_endpoint_;
    network::zeromq::socket::Raw& shutdown_socket_;
    State state_;
    StateSequence reorg_counter_;
    AccountMap accounts_;
    std::optional<StateSequence> startup_reorg_;

    auto do_reorg() noexcept -> void;
    template <typename Callback>
    auto for_each(const Callback& cb) noexcept -> void
    {
        std::for_each(accounts_.begin(), accounts_.end(), cb);
    }
    auto process_block_header(Message&& in) noexcept -> void;
    auto process_nym(Message&& in) noexcept -> bool;
    auto process_nym(const identifier::Nym& nym) noexcept -> bool;
    auto process_reorg(Message&& in) noexcept -> void;
    auto process_reorg(
        Message&& in,
        const block::Position& ancestor,
        const block::Position& tip) noexcept -> void;
    auto state_normal(const Work work, Message&& msg) noexcept -> void;
    auto transition_state_shutdown() noexcept -> void;
    auto transition_state_reorg() noexcept -> bool;

    Imp(const api::Session& api,
        const node::internal::Network& node,
        node::internal::WalletDatabase& db,
        const node::internal::Mempool& mempool,
        const network::zeromq::BatchID batch,
        const Type chain,
        CString&& shutdown,
        allocator_type alloc) noexcept;
};
}  // namespace opentxs::blockchain::node::wallet

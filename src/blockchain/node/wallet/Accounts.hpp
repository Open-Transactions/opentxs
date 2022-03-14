// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string_view>
#include <utility>

#include "blockchain/node/wallet/subchain/NotificationStateData.hpp"  // IWYU pragma: keep
#include "internal/blockchain/node/wallet/Account.hpp"
#include "internal/blockchain/node/wallet/Accounts.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/Subchain.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
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
class Accounts::Imp final : public Actor<Imp, AccountsJobs>
{
public:
    auto Init(boost::shared_ptr<Imp> me) noexcept -> void;
    auto Shutdown() noexcept -> void { signal_shutdown(); }

    Imp(const api::Session& api,
        const node::internal::Network& node,
        const node::internal::WalletDatabase& db,
        const node::internal::Mempool& mempool,
        const network::zeromq::BatchID batch,
        const Type chain,
        const std::string_view toParent,
        allocator_type alloc) noexcept;

    ~Imp() final;

private:
    friend Actor<Imp, AccountsJobs>;

    enum class State {
        normal,
        reorg,
        post_reorg,
        pre_shutdown,
        shutdown,
    };

    struct ReorgData {
        const block::Position ancestor_;
        const block::Position tip_;
        const std::size_t target_;
        std::size_t ready_;
        std::size_t done_;

        ReorgData(
            block::Position&& ancestor,
            block::Position&& tip,
            std::size_t target) noexcept
            : ancestor_(std::move(ancestor))
            , tip_(std::move(tip))
            , target_(target)
            , ready_(0)
            , done_(0)
        {
        }
    };
    struct ShutdownData {
        const std::size_t target_;
        std::size_t ready_;

        ShutdownData(std::size_t target) noexcept
            : target_(target)
            , ready_(0)
        {
        }
    };

    using AccountMap =
        Map<OTNymID, std::pair<network::zeromq::socket::Raw, wallet::Account>>;
    using NotificationMap =
        Map<OTIdentifier,
            std::pair<
                network::zeromq::socket::Raw,
                boost::shared_ptr<NotificationStateData>>>;

    const api::Session& api_;
    const node::internal::Network& node_;
    const node::internal::WalletDatabase& db_;
    const node::internal::Mempool& mempool_;
    const Type chain_;
    const cfilter::Type filter_type_;
    const CString from_children_endpoint_;
    network::zeromq::socket::Raw& to_parent_;
    State state_;
    std::optional<ReorgData> reorg_;
    std::optional<ShutdownData> shutdown_;
    AccountMap accounts_;
    NotificationMap codes_;
    Timer reorg_timer_;

    auto reorg_children() const noexcept -> std::size_t;
    auto verify_child_state(
        const Subchain::State subchain,
        const Account::State account) const noexcept -> void;

    auto do_reorg() noexcept -> void;
    auto do_shutdown() noexcept -> void;
    auto finish_reorg() noexcept -> void;
    auto finish_shutdown() noexcept -> void;
    auto index_nym(const identifier::Nym& id) noexcept -> void;
    auto pipeline(const Work work, Message&& msg) noexcept -> void;
    auto process_block_header(Message&& in) noexcept -> void;
    auto process_nym(Message&& in) noexcept -> bool;
    auto process_nym(const identifier::Nym& nym) noexcept -> bool;
    auto process_reorg_ready(Message&& in) noexcept -> void;
    auto send_to_children(const Message& msg) noexcept -> void;
    auto startup() noexcept -> void;
    auto state_normal(const Work work, Message&& msg) noexcept -> void;
    auto state_post_reorg(const Work work, Message&& msg) noexcept -> void;
    auto state_pre_shutdown(const Work work, Message&& msg) noexcept -> void;
    auto state_reorg(const Work work, Message&& msg) noexcept -> void;
    auto transition_state_normal(Message&& in) noexcept -> void;
    auto transition_state_post_reorg(Message&& in) noexcept -> void;
    auto transition_state_pre_shutdown(Message&& in) noexcept -> void;
    auto transition_state_reorg(Message&& in) noexcept -> void;
    auto transition_state_shutdown(Message&& in) noexcept -> void;
    [[noreturn]] auto work() noexcept -> bool;

    Imp(const api::Session& api,
        const node::internal::Network& node,
        const node::internal::WalletDatabase& db,
        const node::internal::Mempool& mempool,
        const network::zeromq::BatchID batch,
        const Type chain,
        const std::string_view toParent,
        CString&& fromChildren,
        allocator_type alloc) noexcept;
};
}  // namespace opentxs::blockchain::node::wallet

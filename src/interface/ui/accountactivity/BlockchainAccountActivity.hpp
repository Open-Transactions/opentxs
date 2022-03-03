// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <atomic>
#include <mutex>
#include <optional>
#include <tuple>
#include <utility>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "interface/qt/SendMonitor.hpp"
#include "interface/ui/accountactivity/AccountActivity.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/core/Core.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/Unit.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/interface/ui/AccountActivity.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "serialization/protobuf/PaymentWorkflowEnums.pb.h"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

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
class Publish;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class PaymentEvent;
class PaymentWorkflow;
}  // namespace proto

class Data;
class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
class BlockchainAccountActivity final : public AccountActivity
{
public:
    auto ContractID() const noexcept -> UnallocatedCString final
    {
        return opentxs::blockchain::UnitID(Widget::api_, chain_).str();
    }
    using AccountActivity::DepositAddress;
    auto DepositAddress() const noexcept -> UnallocatedCString final
    {
        return DepositAddress(chain_);
    }
    auto DepositAddress(const blockchain::Type) const noexcept
        -> UnallocatedCString final;
    auto DepositChains() const noexcept
        -> UnallocatedVector<blockchain::Type> final
    {
        return {chain_};
    }
    auto DisplayUnit() const noexcept -> UnallocatedCString final
    {
        return blockchain::internal::Ticker(chain_);
    }
    auto Name() const noexcept -> UnallocatedCString final
    {
        return opentxs::blockchain::AccountName(chain_);
    }
    auto NotaryID() const noexcept -> UnallocatedCString final
    {
        return opentxs::blockchain::NotaryID(Widget::api_, chain_).str();
    }
    auto NotaryName() const noexcept -> UnallocatedCString final
    {
        return blockchain::DisplayString(chain_);
    }
    using AccountActivity::Send;
    auto Send(
        const UnallocatedCString& address,
        const Amount& amount,
        const UnallocatedCString& memo) const noexcept -> bool final;
    auto Send(
        const UnallocatedCString& address,
        const UnallocatedCString& amount,
        const UnallocatedCString& memo,
        Scale scale) const noexcept -> bool final;
    auto Send(
        const UnallocatedCString& address,
        const UnallocatedCString& amount,
        const UnallocatedCString& memo,
        Scale scale,
        SendMonitor::Callback cb) const noexcept -> int final;
    auto SyncPercentage() const noexcept -> double final
    {
        return progress_.get_percentage();
    }
    auto SyncProgress() const noexcept -> std::pair<int, int> final
    {
        return progress_.get_progress();
    }
    auto Unit() const noexcept -> UnitType final
    {
        return BlockchainToUnit(chain_);
    }
    auto ValidateAddress(const UnallocatedCString& text) const noexcept
        -> bool final;
    auto ValidateAmount(const UnallocatedCString& text) const noexcept
        -> UnallocatedCString final;

    BlockchainAccountActivity(
        const api::session::Client& api,
        const blockchain::Type chain,
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const SimpleCallback& cb) noexcept;

    ~BlockchainAccountActivity() final;

private:
    struct Progress {
        auto get_percentage() const noexcept -> double
        {
            auto lock = Lock{lock_};

            return percentage_;
        }
        auto get_progress() const noexcept -> std::pair<int, int>
        {
            auto lock = Lock{lock_};

            return ratio_;
        }

        auto set(
            blockchain::block::Height height,
            blockchain::block::Height target) noexcept -> double
        {
            auto lock = Lock{lock_};
            auto& [current, max] = ratio_;
            current = static_cast<int>(height);
            max = static_cast<int>(target);
            percentage_ = internal::make_progress(height, target);

            return percentage_;
        }

    private:
        mutable std::mutex lock_{};
        double percentage_{};
        std::pair<int, int> ratio_{};
    };

    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        contact = value(WorkType::ContactUpdated),
        balance = value(WorkType::BlockchainBalance),
        new_block = value(WorkType::BlockchainNewHeader),
        txid = value(WorkType::BlockchainNewTransaction),
        reorg = value(WorkType::BlockchainReorg),
        statechange = value(WorkType::BlockchainStateChange),
        sync = value(WorkType::BlockchainSyncProgress),
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    const blockchain::Type chain_;
    mutable Amount confirmed_;
    OTZMQListenCallback balance_cb_;
    OTZMQDealerSocket balance_socket_;
    Progress progress_;
    blockchain::block::Height height_;

    static auto print(Work type) noexcept -> const char*;

    auto display_balance(opentxs::Amount value) const noexcept
        -> UnallocatedCString final;

    auto load_thread() noexcept -> void;
    auto pipeline(const Message& in) noexcept -> void final;
    auto process_balance(const Message& in) noexcept -> void;
    auto process_block(const Message& in) noexcept -> void;
    auto process_contact(const Message& in) noexcept -> void;
    auto process_height(const blockchain::block::Height height) noexcept
        -> void;
    auto process_reorg(const Message& in) noexcept -> void;
    auto process_state(const Message& in) noexcept -> void;
    auto process_sync(const Message& in) noexcept -> void;
    auto process_txid(const Message& in) noexcept -> void;
    auto process_txid(const Data& txid) noexcept
        -> std::optional<AccountActivityRowID>;
    auto startup() noexcept -> void final;

    BlockchainAccountActivity() = delete;
    BlockchainAccountActivity(const BlockchainAccountActivity&) = delete;
    BlockchainAccountActivity(BlockchainAccountActivity&&) = delete;
    auto operator=(const BlockchainAccountActivity&)
        -> BlockchainAccountActivity& = delete;
    auto operator=(BlockchainAccountActivity&&)
        -> BlockchainAccountActivity& = delete;
};
}  // namespace opentxs::ui::implementation

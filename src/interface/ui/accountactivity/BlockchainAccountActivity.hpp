// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>
#include <optional>
#include <span>
#include <string_view>
#include <utility>

#include "interface/qt/SendMonitor.hpp"
#include "interface/ui/accountactivity/AccountActivity.hpp"
#include "internal/core/Core.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Dealer.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/WorkType.hpp"  // IWYU pragma: keep
#include "opentxs/WorkType.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class TransactionHash;
}  // namespace block
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
class BlockchainAccountActivity final : public AccountActivity
{
public:
    auto API() const noexcept -> const api::Session& final { return api_; }
    auto ContractID() const noexcept -> UnallocatedCString final
    {
        return opentxs::blockchain::UnitID(api_, chain_)
            .asBase58(api_.Crypto());
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
        return ticker_symbol(chain_);
    }
    auto Name() const noexcept -> UnallocatedCString final
    {
        return opentxs::blockchain::AccountName(chain_);
    }
    auto NotaryID() const noexcept -> UnallocatedCString final
    {
        return opentxs::blockchain::NotaryID(api_, chain_)
            .asBase58(api_.Crypto());
    }
    auto NotaryName() const noexcept -> UnallocatedCString final
    {
        return UnallocatedCString{blockchain::print(chain_)};
    }
    auto Notify(std::span<const PaymentCode> contacts, SendMonitor::Callback cb)
        const noexcept -> int final;
    auto Notify(std::span<const PaymentCode> contacts) const noexcept
        -> bool final;
    using AccountActivity::Send;
    auto Send(
        const UnallocatedCString& address,
        const Amount& amount,
        const std::string_view memo,
        std::span<const PaymentCode> notify) const noexcept -> bool final;
    auto Send(
        const UnallocatedCString& address,
        const UnallocatedCString& amount,
        const std::string_view memo,
        Scale scale,
        std::span<const PaymentCode> notify) const noexcept -> bool final;
    auto Send(
        const UnallocatedCString& address,
        const UnallocatedCString& amount,
        const std::string_view memo,
        Scale scale,
        SendMonitor::Callback cb,
        std::span<const PaymentCode> notify) const noexcept -> int final;
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
        return blockchain_to_unit(chain_);
    }
    auto ValidateAddress(const UnallocatedCString& text) const noexcept
        -> bool final;
    auto ValidateAmount(const UnallocatedCString& text) const noexcept
        -> UnallocatedCString final;

    BlockchainAccountActivity(
        const api::session::Client& api,
        const blockchain::Type chain,
        const identifier::Nym& nymID,
        const identifier::Account& accountID,
        const SimpleCallback& cb) noexcept;
    BlockchainAccountActivity() = delete;
    BlockchainAccountActivity(const BlockchainAccountActivity&) = delete;
    BlockchainAccountActivity(BlockchainAccountActivity&&) = delete;
    auto operator=(const BlockchainAccountActivity&)
        -> BlockchainAccountActivity& = delete;
    auto operator=(BlockchainAccountActivity&&)
        -> BlockchainAccountActivity& = delete;

    ~BlockchainAccountActivity() final;

private:
    struct Progress {
        auto get_percentage() const noexcept -> double
        {
            const auto lock = Lock{lock_};

            return percentage_;
        }
        auto get_progress() const noexcept -> std::pair<int, int>
        {
            const auto lock = Lock{lock_};

            return ratio_;
        }

        auto set(
            blockchain::block::Height height,
            blockchain::block::Height target) noexcept -> double
        {
            const auto lock = Lock{lock_};
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
    auto process_txid(const blockchain::block::TransactionHash& txid) noexcept
        -> std::optional<AccountActivityRowID>;
    auto process_txid(
        const blockchain::block::TransactionHash& txid,
        blockchain::block::Transaction tx) noexcept
        -> std::optional<AccountActivityRowID>;
    auto startup() noexcept -> void final;
};
}  // namespace opentxs::ui::implementation

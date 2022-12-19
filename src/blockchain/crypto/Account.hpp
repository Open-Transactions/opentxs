// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>

#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/network/zeromq/socket/Push.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/Imported.hpp"
#include "opentxs/blockchain/crypto/Notification.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Contacts;
}  // namespace session

class Session;
}  // namespace api

namespace blockchain
{
namespace crypto
{
class Account;
class AccountIndex;
class Element;
}  // namespace crypto
}  // namespace blockchain

namespace proto
{
class HDPath;
}  // namespace proto

class Amount;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::implementation
{
class Account final : public internal::Account
{
public:
    using Accounts = UnallocatedSet<identifier::Generic>;

    auto AccountID() const noexcept -> const identifier::Generic& final
    {
        return account_id_;
    }
    auto AssociateTransaction(
        const UnallocatedVector<Activity>& unspent,
        const UnallocatedVector<Activity>& spent,
        UnallocatedSet<identifier::Generic>& contacts,
        const PasswordPrompt& reason) const noexcept -> bool final;
    auto Chain() const noexcept -> opentxs::blockchain::Type final
    {
        return chain_;
    }
    auto ClaimAccountID(const identifier::Generic& id, crypto::Subaccount* node)
        const noexcept -> void final;
    auto FindNym(const identifier::Nym& id) const noexcept -> void final;
    auto GetDepositAddress(
        const blockchain::crypto::AddressStyle style,
        const PasswordPrompt& reason,
        const UnallocatedCString& memo) const noexcept
        -> UnallocatedCString final;
    auto GetDepositAddress(
        const blockchain::crypto::AddressStyle style,
        const identifier::Generic& contact,
        const PasswordPrompt& reason,
        const UnallocatedCString& memo) const noexcept
        -> UnallocatedCString final;
    auto GetHD() const noexcept -> const HDAccounts& final { return hd_; }
    auto GetImported() const noexcept -> const ImportedAccounts& final
    {
        return imported_;
    }
    auto GetNotification() const noexcept -> const NotificationAccounts& final
    {
        return notification_;
    }
    auto GetNextChangeKey(const PasswordPrompt& reason) const noexcept(false)
        -> const Element& final;
    auto GetNextDepositKey(const PasswordPrompt& reason) const noexcept(false)
        -> const Element& final;
    auto GetPaymentCode() const noexcept -> const PaymentCodeAccounts& final
    {
        return payment_code_;
    }
    auto Internal() const noexcept -> Account& final
    {
        return const_cast<Account&>(*this);
    }
    auto LookupUTXO(const Coin& coin) const noexcept
        -> std::optional<std::pair<Key, Amount>> final;
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return nym_id_;
    }
    auto Parent() const noexcept -> const crypto::Wallet& final
    {
        return parent_;
    }
    auto Subaccount(const identifier::Generic& id) const noexcept(false)
        -> const crypto::Subaccount& final;

    auto AddHDNode(
        const proto::HDPath& path,
        const crypto::HDProtocol standard,
        const PasswordPrompt& reason,
        identifier::Generic& id) noexcept -> bool final;
    auto AddUpdatePaymentCode(
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const PasswordPrompt& reason,
        identifier::Generic& out) noexcept -> bool final
    {
        return payment_code_.Construct(
            out, contacts_, local, remote, path, reason);
    }
    auto AddUpdatePaymentCode(
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const opentxs::blockchain::block::TransactionHash& txid,
        const PasswordPrompt& reason,
        identifier::Generic& out) noexcept -> bool final
    {
        return payment_code_.Construct(
            out, contacts_, local, remote, path, txid, reason);
    }
    auto Startup() noexcept -> void final { init_notification(); }

    Account(
        const api::Session& api,
        const api::session::Contacts& contacts,
        const crypto::Wallet& parent,
        const AccountIndex& index,
        const identifier::Nym& nym,
        const Accounts& hd,
        const Accounts& imported,
        const Accounts& paymentCode) noexcept;
    Account() = delete;
    Account(const Account&) = delete;
    Account(Account&&) = delete;
    auto operator=(const Account&) -> Account& = delete;
    auto operator=(Account&&) -> Account& = delete;

    ~Account() final = default;

private:
    template <typename InterfaceType, typename PayloadType>
    class NodeGroup final : virtual public InterfaceType
    {
    public:
        using const_iterator = typename InterfaceType::const_iterator;
        using value_type = typename InterfaceType::value_type;

        auto all() const noexcept -> UnallocatedSet<identifier::Generic> final
        {
            auto out = UnallocatedSet<identifier::Generic>{};
            auto lock = Lock{lock_};

            for (const auto& [id, count] : index_) { out.emplace(id); }

            return out;
        }
        auto at(const std::size_t position) const -> const value_type& final
        {
            auto lock = Lock{lock_};

            return *nodes_.at(position);
        }
        auto at(const identifier::Generic& id) const -> const PayloadType& final
        {
            auto lock = Lock{lock_};

            return *nodes_.at(index_.at(id));
        }
        auto begin() const noexcept -> const_iterator final
        {
            return const_iterator(this, 0);
        }
        auto cbegin() const noexcept -> const_iterator final
        {
            return const_iterator(this, 0);
        }
        auto cend() const noexcept -> const_iterator final
        {
            return const_iterator(this, nodes_.size());
        }
        auto end() const noexcept -> const_iterator final
        {
            return const_iterator(this, nodes_.size());
        }
        auto size() const noexcept -> std::size_t final
        {
            return nodes_.size();
        }
        auto Type() const noexcept -> SubaccountType final { return type_; }

        auto at(const std::size_t position) -> value_type&
        {
            auto lock = Lock{lock_};

            return *nodes_.at(position);
        }
        auto at(const identifier::Generic& id) -> PayloadType&
        {
            auto lock = Lock{lock_};

            return *nodes_.at(index_.at(id));
        }
        template <typename... Args>
        auto Construct(identifier::Generic& out, const Args&... args) noexcept
            -> bool
        {
            auto lock = Lock{lock_};

            return construct(lock, out, args...);
        }

        NodeGroup(
            const api::Session& api,
            const SubaccountType type,
            Account& parent) noexcept
            : api_(api)
            , type_(type)
            , parent_(parent)
            , lock_()
            , nodes_()
            , index_()
        {
        }

    private:
        const api::Session& api_;
        const SubaccountType type_;
        Account& parent_;
        mutable std::mutex lock_;
        UnallocatedVector<std::unique_ptr<PayloadType>> nodes_;
        UnallocatedMap<identifier::Generic, std::size_t> index_;

        auto add(
            const Lock& lock,
            const identifier::Generic& id,
            std::unique_ptr<PayloadType> node) noexcept -> bool;
        template <typename... Args>
        auto construct(
            const Lock& lock,
            identifier::Generic& id,
            const Args&... args) noexcept -> bool
        {
            auto node{
                Factory<PayloadType, Args...>::get(api_, parent_, id, args...)};

            if (false == bool(node)) { return false; }

            if (0 < index_.count(id)) {
                LogTrace()(OT_PRETTY_CLASS())("subaccount ")(
                    id)(" already exists")
                    .Flush();

                return false;
            } else {
                LogTrace()(OT_PRETTY_CLASS())("subaccount ")(id)(" created")
                    .Flush();
            }

            return add(lock, id, std::move(node));
        }
    };

    template <typename ReturnType, typename... Args>
    struct Factory {
        static auto get(
            const api::Session& api,
            const crypto::Account& parent,
            identifier::Generic& id,
            const Args&... args) noexcept -> std::unique_ptr<ReturnType>;
    };

    struct NodeIndex {
        auto Find(const identifier::Generic& id) const noexcept
            -> crypto::Subaccount*;

        void Add(
            const identifier::Generic& id,
            crypto::Subaccount* node) noexcept;

        NodeIndex() noexcept
            : lock_()
            , index_()
        {
        }

    private:
        mutable std::mutex lock_;
        UnallocatedMap<identifier::Generic, crypto::Subaccount*> index_;
    };

    using HDNodes = NodeGroup<HDAccounts, crypto::HD>;
    using ImportedNodes = NodeGroup<ImportedAccounts, crypto::Imported>;
    using NotificationNodes =
        NodeGroup<NotificationAccounts, crypto::Notification>;
    using PaymentCodeNodes =
        NodeGroup<PaymentCodeAccounts, crypto::PaymentCode>;

    const api::Session& api_;
    const api::session::Contacts& contacts_;
    const crypto::Wallet& parent_;
    const AccountIndex& account_index_;
    const opentxs::blockchain::Type chain_;
    const identifier::Nym nym_id_;
    const identifier::Generic account_id_;
    HDNodes hd_;
    ImportedNodes imported_;
    NotificationNodes notification_;
    PaymentCodeNodes payment_code_;
    mutable NodeIndex node_index_;
    mutable std::mutex lock_;
    mutable internal::ActivityMap unspent_;
    mutable internal::ActivityMap spent_;
    OTZMQPushSocket find_nym_;

    auto init_hd(const Accounts& HDAccounts) noexcept -> void;
    auto init_notification() noexcept -> void;
    auto init_payment_code(const Accounts& HDAccounts) noexcept -> void;

    auto find_next_element(
        Subchain subchain,
        const identifier::Generic& contact,
        const UnallocatedCString& label,
        const PasswordPrompt& reason) const noexcept(false) -> const Element&;
};
}  // namespace opentxs::blockchain::crypto::implementation

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/crypto/HDProtocol.hpp"

#pragma once

#include <algorithm>
#include <cstddef>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>
#include <tuple>
#include <utility>

#include "internal/blockchain/crypto/Crypto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/Imported.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
class AccountIndex;
class Element;
}  // namespace crypto
}  // namespace blockchain

namespace proto
{
class HDPath;
}  // namespace proto

class PasswordPrompt;
class PaymentCode;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::implementation
{
class Account final : public internal::Account
{
public:
    using Accounts = UnallocatedSet<OTIdentifier>;

    auto AccountID() const noexcept -> const Identifier& final
    {
        return account_id_;
    }
    auto AssociateTransaction(
        const UnallocatedVector<Activity>& unspent,
        const UnallocatedVector<Activity>& spent,
        UnallocatedSet<OTIdentifier>& contacts,
        const PasswordPrompt& reason) const noexcept -> bool final;
    auto Chain() const noexcept -> opentxs::blockchain::Type final
    {
        return chain_;
    }
    auto ClaimAccountID(const UnallocatedCString& id, crypto::Subaccount* node)
        const noexcept -> void final;
    auto FindNym(const identifier::Nym& id) const noexcept -> void final;
    auto GetDepositAddress(
        const blockchain::crypto::AddressStyle style,
        const PasswordPrompt& reason,
        const UnallocatedCString& memo) const noexcept
        -> UnallocatedCString final;
    auto GetDepositAddress(
        const blockchain::crypto::AddressStyle style,
        const Identifier& contact,
        const PasswordPrompt& reason,
        const UnallocatedCString& memo) const noexcept
        -> UnallocatedCString final;
    auto GetHD() const noexcept -> const HDAccounts& final { return hd_; }
    auto GetImported() const noexcept -> const ImportedAccounts& final
    {
        return imported_;
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
    auto Subaccount(const Identifier& id) const noexcept(false)
        -> const crypto::Subaccount& final;

    auto AddHDNode(
        const proto::HDPath& path,
        const crypto::HDProtocol standard,
        const PasswordPrompt& reason,
        Identifier& id) noexcept -> bool final
    {
        return hd_.Construct(id, path, standard, reason);
    }
    auto AddUpdatePaymentCode(
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const PasswordPrompt& reason,
        Identifier& out) noexcept -> bool final
    {
        return payment_code_.Construct(
            out, contacts_, local, remote, path, reason);
    }
    auto AddUpdatePaymentCode(
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const opentxs::blockchain::block::Txid& txid,
        const PasswordPrompt& reason,
        Identifier& out) noexcept -> bool final
    {
        return payment_code_.Construct(
            out, contacts_, local, remote, path, txid, reason);
    }

    Account(
        const api::Session& api,
        const api::session::Contacts& contacts,
        const crypto::Wallet& parent,
        const AccountIndex& index,
        const identifier::Nym& nym,
        const Accounts& hd,
        const Accounts& imported,
        const Accounts& paymentCode) noexcept;

    ~Account() final = default;

private:
    template <typename InterfaceType, typename PayloadType>
    class NodeGroup final : virtual public InterfaceType
    {
    public:
        using const_iterator = typename InterfaceType::const_iterator;
        using value_type = typename InterfaceType::value_type;

        auto all() const noexcept -> UnallocatedSet<OTIdentifier> final
        {
            auto out = UnallocatedSet<OTIdentifier>{};
            auto lock = Lock{lock_};

            for (const auto& [id, count] : index_) { out.emplace(id); }

            return out;
        }
        auto at(const std::size_t position) const -> const value_type& final
        {
            auto lock = Lock{lock_};

            return *nodes_.at(position);
        }
        auto at(const Identifier& id) const -> const PayloadType& final
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
        auto at(const Identifier& id) -> PayloadType&
        {
            auto lock = Lock{lock_};

            return *nodes_.at(index_.at(id));
        }
        template <typename... Args>
        auto Construct(Identifier& out, const Args&... args) noexcept -> bool
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
        UnallocatedMap<OTIdentifier, std::size_t> index_;

        auto add(
            const Lock& lock,
            const Identifier& id,
            std::unique_ptr<PayloadType> node) noexcept -> bool;
        template <typename... Args>
        auto construct(
            const Lock& lock,
            Identifier& id,
            const Args&... args) noexcept -> bool
        {
            auto node{
                Factory<PayloadType, Args...>::get(api_, parent_, id, args...)};

            if (false == bool(node)) { return false; }

            if (0 < index_.count(id)) {
                LogError()("Blockchain account ")(id)(" already exists")
                    .Flush();

                return false;
            }

            return add(lock, id, std::move(node));
        }
    };

    template <typename ReturnType, typename... Args>
    struct Factory {
        static auto get(
            const api::Session& api,
            const crypto::Account& parent,
            Identifier& id,
            const Args&... args) noexcept -> std::unique_ptr<ReturnType>;
    };

    struct NodeIndex {
        auto Find(const UnallocatedCString& id) const noexcept
            -> crypto::Subaccount*;

        void Add(
            const UnallocatedCString& id,
            crypto::Subaccount* node) noexcept;

        NodeIndex() noexcept
            : lock_()
            , index_()
        {
        }

    private:
        mutable std::mutex lock_;
        UnallocatedMap<UnallocatedCString, crypto::Subaccount*> index_;
    };

    using HDNodes = NodeGroup<HDAccounts, crypto::HD>;
    using ImportedNodes = NodeGroup<ImportedAccounts, crypto::Imported>;
    using PaymentCodeNodes =
        NodeGroup<PaymentCodeAccounts, crypto::PaymentCode>;

    const api::Session& api_;
    const api::session::Contacts& contacts_;
    const crypto::Wallet& parent_;
    const AccountIndex& account_index_;
    const opentxs::blockchain::Type chain_;
    const OTNymID nym_id_;
    const OTIdentifier account_id_;
    HDNodes hd_;
    ImportedNodes imported_;
    PaymentCodeNodes payment_code_;
    mutable NodeIndex node_index_;
    mutable std::mutex lock_;
    mutable internal::ActivityMap unspent_;
    mutable internal::ActivityMap spent_;
    OTZMQPushSocket find_nym_;

    void init_hd(const Accounts& HDAccounts) noexcept;
    void init_payment_code(const Accounts& HDAccounts) noexcept;

    auto find_next_element(
        Subchain subchain,
        const Identifier& contact,
        const UnallocatedCString& label,
        const PasswordPrompt& reason) const noexcept(false) -> const Element&;

    Account() = delete;
    Account(const Account&) = delete;
    Account(Account&&) = delete;
    auto operator=(const Account&) -> Account& = delete;
    auto operator=(Account&&) -> Account& = delete;
};
}  // namespace opentxs::blockchain::crypto::implementation

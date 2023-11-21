// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_shared_guarded.h>
#include <cstddef>
#include <memory>
#include <shared_mutex>

#include "internal/blockchain/crypto/Wallet.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"

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

namespace proto
{
class HDPath;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::implementation
{
class Wallet final : virtual public internal::Wallet
{
public:
    auto Account(const identifier::Nym& id) const noexcept
        -> crypto::Account& final;
    auto at(const std::size_t position) const noexcept(false)
        -> const_iterator::value_type& final;
    auto begin() const noexcept -> const_iterator final { return {this, 0}; }
    auto cbegin() const noexcept -> const_iterator final { return {this, 0}; }
    auto cend() const noexcept -> const_iterator final;
    auto Chain() const noexcept -> opentxs::blockchain::Type final
    {
        return chain_;
    }
    auto end() const noexcept -> const_iterator final;
    auto Internal() const noexcept -> internal::Wallet& final
    {
        return const_cast<Wallet&>(*this);
    }
    auto Parent() const noexcept -> const api::crypto::Blockchain& final
    {
        return parent_;
    }
    auto size() const noexcept -> std::size_t final;

    auto AddEthereum(
        const identifier::Nym& nym,
        const proto::HDPath& path,
        const crypto::HDProtocol standard,
        const PasswordPrompt& reason) noexcept -> crypto::Subaccount& final;
    auto AddHD(
        const identifier::Nym& nym,
        const proto::HDPath& path,
        const crypto::HDProtocol standard,
        const PasswordPrompt& reason) noexcept -> crypto::Subaccount& final;
    auto AddPaymentCode(
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const PasswordPrompt& reason) noexcept -> crypto::Subaccount& final;

    Wallet(
        const api::Session& api,
        const api::session::Contacts& contacts,
        const api::crypto::Blockchain& parent,
        const opentxs::blockchain::Type chain) noexcept;
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;

    ~Wallet() final = default;

private:
    using Accounts = UnallocatedSet<identifier::Account>;

    struct Data {
        Vector<std::unique_ptr<crypto::Account>> trees_{};
        Map<identifier::Nym, std::size_t> index_{};
    };
    using Guarded = libguarded::shared_guarded<Data, std::shared_mutex>;

    const api::crypto::Blockchain& parent_;
    const api::Session& api_;
    const api::session::Contacts& contacts_;
    const opentxs::blockchain::Type chain_;
    mutable Guarded data_;

    auto add(
        Data& data,
        const identifier::Nym& id,
        std::unique_ptr<crypto::Account> tree) const noexcept -> bool;
    using crypto::Wallet::at;
    auto at(const Data& data, const std::size_t index) const noexcept(false)
        -> const crypto::Account&;
    auto at(Data& data, const std::size_t index) const noexcept(false)
        -> crypto::Account&;
    auto factory(
        const identifier::Nym& nym,
        const Accounts& hd,
        const Accounts& ethereum,
        const Accounts& paymentCode) const noexcept
        -> std::unique_ptr<crypto::Account>;
    auto get_or_create(Data& data, const identifier::Nym& id) const noexcept
        -> crypto::Account&;
    using crypto::Wallet::size;
    auto size(const Data& data) const noexcept -> std::size_t;

    auto init() noexcept -> void;
};
}  // namespace opentxs::blockchain::crypto::implementation

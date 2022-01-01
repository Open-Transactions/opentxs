// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/crypto/HDProtocol.hpp"

#pragma once

#include <cstddef>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <vector>

#include "internal/blockchain/crypto/Crypto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"

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
}  // namespace crypto
}  // namespace blockchain

namespace proto
{
class HDPath;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::blockchain::crypto::implementation
{
class Wallet final : virtual public internal::Wallet
{
public:
    auto Account(const identifier::Nym& id) const noexcept
        -> crypto::Account& final;
    auto at(const std::size_t position) const noexcept(false)
        -> const_iterator::value_type& final;
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
        return const_iterator(this, trees_.size());
    }
    auto Chain() const noexcept -> opentxs::blockchain::Type final
    {
        return chain_;
    }
    auto end() const noexcept -> const_iterator final
    {
        return const_iterator(this, trees_.size());
    }
    auto Internal() const noexcept -> internal::Wallet& final
    {
        return const_cast<Wallet&>(*this);
    }
    auto Parent() const noexcept -> const api::crypto::Blockchain& final
    {
        return parent_;
    }
    auto size() const noexcept -> std::size_t final { return trees_.size(); }

    auto AddHDNode(
        const identifier::Nym& nym,
        const proto::HDPath& path,
        const crypto::HDProtocol standard,
        const PasswordPrompt& reason,
        Identifier& id) noexcept -> bool final;

    Wallet(
        const api::Session& api,
        const api::session::Contacts& contacts,
        const api::crypto::Blockchain& parent,
        const AccountIndex& index,
        const opentxs::blockchain::Type chain) noexcept;

    ~Wallet() final = default;

private:
    using Accounts = std::pmr::set<OTIdentifier>;

    const api::crypto::Blockchain& parent_;
    const AccountIndex& account_index_;
    const api::Session& api_;
    const api::session::Contacts& contacts_;
    const opentxs::blockchain::Type chain_;
    mutable std::mutex lock_;
    std::pmr::vector<std::unique_ptr<crypto::Account>> trees_;
    std::pmr::map<OTNymID, std::size_t> index_;

    using crypto::Wallet::at;
    auto at(const Lock& lock, const std::size_t index) const noexcept(false)
        -> const crypto::Account&;
    auto factory(
        const identifier::Nym& nym,
        const Accounts& hd,
        const Accounts& paymentCode) const noexcept
        -> std::unique_ptr<crypto::Account>;
    using crypto::Wallet::size;
    auto size(const Lock& lock) const noexcept -> std::size_t;

    auto add(
        const Lock& lock,
        const identifier::Nym& id,
        std::unique_ptr<crypto::Account> tree) noexcept -> bool;
    auto at(const Lock& lock, const std::size_t index) noexcept(false)
        -> crypto::Account&;
    auto get_or_create(const Lock& lock, const identifier::Nym& id) noexcept
        -> crypto::Account&;
    void init() noexcept;

    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;
};
}  // namespace opentxs::blockchain::crypto::implementation

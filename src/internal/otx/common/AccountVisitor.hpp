// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Wallet;
}  // namespace session
}  // namespace api

namespace identifier
{
class Notary;
}  // namespace identifier

class Account;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class AccountVisitor
{
public:
    using mapOfAccounts = UnallocatedMap<UnallocatedCString, const Account*>;

    auto GetNotaryID() const -> const identifier::Notary& { return notary_id_; }

    virtual auto Trigger(const Account& account, const PasswordPrompt& reason)
        -> bool = 0;

    auto Wallet() const -> const api::session::Wallet& { return wallet_; }

    AccountVisitor() = delete;
    AccountVisitor(const AccountVisitor&) = delete;
    AccountVisitor(AccountVisitor&&) = delete;
    auto operator=(const AccountVisitor&) -> AccountVisitor& = delete;
    auto operator=(AccountVisitor&&) -> AccountVisitor& = delete;

    virtual ~AccountVisitor() = default;

protected:
    const api::session::Wallet& wallet_;
    const identifier::Notary notary_id_;
    mapOfAccounts* loaded_accounts_;

    AccountVisitor(
        const api::session::Wallet& wallet,
        const identifier::Notary& notaryID);
};
}  // namespace opentxs

// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <map>
#include <string>

#include "opentxs/core/identifier/Server.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
class Wallet;
}  // namespace session
}  // namespace api

class Account;
class PasswordPrompt;

class AccountVisitor
{
public:
    using mapOfAccounts = std::pmr::map<std::string, const Account*>;

    auto GetNotaryID() const -> const identifier::Server& { return notaryID_; }

    virtual auto Trigger(const Account& account, const PasswordPrompt& reason)
        -> bool = 0;

    auto Wallet() const -> const api::session::Wallet& { return wallet_; }

    virtual ~AccountVisitor() = default;

protected:
    const api::session::Wallet& wallet_;
    const OTServerID notaryID_;
    mapOfAccounts* loadedAccounts_;

    AccountVisitor(
        const api::session::Wallet& wallet,
        const identifier::Server& notaryID);

private:
    AccountVisitor() = delete;
    AccountVisitor(const AccountVisitor&) = delete;
    AccountVisitor(AccountVisitor&&) = delete;
    auto operator=(const AccountVisitor&) -> AccountVisitor& = delete;
    auto operator=(AccountVisitor&&) -> AccountVisitor& = delete;
};
}  // namespace opentxs

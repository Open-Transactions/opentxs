// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "internal/api/session/Wallet.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Nym;
class Server;
class UnitDefinition;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

class PasswordPrompt;
class Tag;
}  // namespace opentxs

namespace opentxs::otx::internal
{
/** The server needs to store a list of accounts, by instrument definition ID,
 * to store the backing funds for vouchers. The below class is useful for that.
 * It's also useful for the same purpose for stashes, in smart contracts.
 * Eventually will add expiration dates, possibly, to this class. (To have
 * series, just like cash already does now.) */
class AccountList
{
public:
    auto GetCountAccountIDs() const -> std::int32_t
    {
        return static_cast<std::int32_t>(mapAcctIDs_.size());
    }
    void Release();
    void Release_AcctList();
    void Serialize(Tag& parent) const;
    auto ReadFromXMLNode(
        irr::io::IrrXMLReader*& xml,
        const String& acctType,
        const String& acctCount) -> std::int32_t;
    void SetType(Account::AccountType acctType) { acctType_ = acctType; }
    auto GetOrRegisterAccount(
        const identity::Nym& serverNym,
        const identifier::Nym& ACCOUNT_OWNER_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::Server& NOTARY_ID,
        bool& wasAcctCreated,  // this will be set to true if the acct is
                               // created here. Otherwise set to false;
        const PasswordPrompt& reason,
        std::int64_t stashTransNum = 0) -> ExclusiveAccount;

    explicit AccountList(const api::Session& api);
    explicit AccountList(
        const api::Session& api,
        Account::AccountType acctType);

    ~AccountList();

private:
    using MapOfWeakAccounts =
        std::pmr::map<std::string, std::weak_ptr<Account>>;

    const api::Session& api_;
    Account::AccountType acctType_;

    /** AcctIDs as second mapped by ASSET TYPE ID as first. */
    String::Map mapAcctIDs_;

    AccountList() = delete;
};
}  // namespace opentxs::otx::internal

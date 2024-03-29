// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/AccountList.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>

#include "internal/core/String.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Helpers.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::otx::internal
{
AccountList::AccountList(const api::Session& api)
    : api_(api)
    , acct_type_(Account::voucher)
    , map_acct_ids_{}
{
}

AccountList::AccountList(const api::Session& api, Account::AccountType acctType)
    : api_(api)
    , acct_type_(acctType)
    , map_acct_ids_{}
{
}

void AccountList::Serialize(Tag& parent) const
{
    auto acctType = String::Factory();
    TranslateAccountTypeToString(acct_type_, acctType);
    const auto sizeMapAcctIDs = map_acct_ids_.size();

    TagPtr pTag(new Tag("accountList"));

    pTag->add_attribute("type", acctType->Get());
    pTag->add_attribute("count", std::to_string(sizeMapAcctIDs));

    for (const auto& it : map_acct_ids_) {
        const UnallocatedCString instrumentDefinitionID = it.first;
        const UnallocatedCString accountId = it.second;
        assert_true(
            (instrumentDefinitionID.size() > 0) && (accountId.size() > 0));

        TagPtr pTagEntry(new Tag("accountEntry"));

        pTagEntry->add_attribute(
            "instrumentDefinitionID", instrumentDefinitionID);
        pTagEntry->add_attribute("accountID", accountId);

        pTag->add_tag(pTagEntry);
    }

    parent.add_tag(pTag);
}

auto AccountList::ReadFromXMLNode(
    irr::io::IrrXMLReader*& xml,
    const String& acctType,
    const String& acctCount) -> std::int32_t
{
    if (!acctType.Exists()) {
        LogError()()("Failed: Empty accountList "
                     "'type' attribute.")
            .Flush();
        return -1;
    }

    acct_type_ = TranslateAccountTypeStringToEnum(acctType);

    if (Account::err_acct == acct_type_) {
        LogError()()("Failed: accountList 'type' "
                     "attribute contains unknown value.")
            .Flush();
        return -1;
    }

    // Load up the account IDs.
    // NOLINTNEXTLINE(cert-err34-c)
    std::int32_t count = acctCount.Exists() ? atoi(acctCount.Get()) : 0;
    if (count > 0) {
        while (count-- > 0) {
            if (!SkipToElement(xml)) {
                LogConsole()()("Failure: Unable to find "
                               "expected element.")
                    .Flush();
                return -1;
            }

            if ((xml->getNodeType() == irr::io::EXN_ELEMENT) &&
                (!strcmp("accountEntry", xml->getNodeName()))) {
                auto instrumentDefinitionID =
                    String::Factory(xml->getAttributeValue(
                        "instrumentDefinitionID"));  // Instrument Definition ID
                                                     // of this account.
                auto accountID = String::Factory(xml->getAttributeValue(
                    "accountID"));  // Account ID for this account.

                if (!instrumentDefinitionID->Exists() || !accountID->Exists()) {
                    LogError()()("Error loading accountEntry: Either the "
                                 "instrumentDefinitionID (")(
                        instrumentDefinitionID.get())("), or the accountID (")(
                        accountID.get())(") was EMPTY.")
                        .Flush();
                    return -1;
                }

                map_acct_ids_.insert(std::make_pair(
                    instrumentDefinitionID->Get(), accountID->Get()));
            } else {
                LogError()()("Expected accountEntry element in accountList.")
                    .Flush();
                return -1;
            }
        }
    }

    if (!SkipAfterLoadingField(xml))  // </accountList>
    {
        LogConsole()()(
            "Bad data? Expected EXN_ELEMENT_END here, but didn't get it. "
            "Returning false.")
            .Flush();
        return -1;
    }

    return 1;
}

void AccountList::Release_AcctList() { map_acct_ids_.clear(); }

void AccountList::Release() { Release_AcctList(); }

auto AccountList::GetOrRegisterAccount(
    const identity::Nym& serverNym,
    const identifier::Nym& accountOwnerId,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const identifier::Notary& notaryID,
    bool& wasAcctCreated,
    const PasswordPrompt& reason,
    std::int64_t stashTransNum) -> ExclusiveAccount
{
    ExclusiveAccount account{};
    wasAcctCreated = false;

    if (Account::stash == acct_type_) {
        if (1 > stashTransNum) {
            LogError()()("Failed attempt to "
                         "create stash account without cron item #.")
                .Flush();

            return {};
        }
    }

    // First, we'll see if there's already an account ID available for the
    // requested instrument definition ID.
    auto acctTypeString = String::Factory();
    TranslateAccountTypeToString(acct_type_, acctTypeString);
    auto acctIDsIt =
        map_acct_ids_.find(instrumentDefinitionID.asBase58(api_.Crypto()));

    // Account ID *IS* already there for this instrument definition
    if (map_acct_ids_.end() != acctIDsIt) {
        const auto& accountID = acctIDsIt->second;
        account = api_.Wallet().Internal().mutable_Account(
            api_.Factory().AccountIDFromBase58(accountID), reason);

        if (account) {

            LogDebug()()("Successfully loaded ")(acctTypeString.get())(
                " account ID: ")(accountID)("Unit Type ID:: ")(
                instrumentDefinitionID, api_.Crypto())
                .Flush();

            return account;
        }
    }

    // Not found. There's no account ID yet for that instrument definition ID.
    // That means we can create it.
    account = api_.Wallet().Internal().CreateAccount(
        accountOwnerId,
        notaryID,
        instrumentDefinitionID,
        serverNym,
        acct_type_,
        stashTransNum,
        reason);

    if (false == bool(account)) {
        LogError()()("Failed trying to generate ")(acctTypeString.get())(
            " account with instrument definition ID: ")(
            instrumentDefinitionID, api_.Crypto())(".")
            .Flush();
    } else {
        auto acctIDString = String::Factory();
        account.get().GetIdentifier(acctIDString);

        LogConsole()()("Successfully created ")(acctTypeString.get())(
            " account ID: ")(acctIDString.get())(" Instrument Definition ID: ")(
            instrumentDefinitionID, api_.Crypto())
            .Flush();
        map_acct_ids_[instrumentDefinitionID.asBase58(api_.Crypto())] =
            acctIDString->Get();

        wasAcctCreated = true;
    }

    return account;
}

AccountList::~AccountList() { Release_AcctList(); }
}  // namespace opentxs::otx::internal

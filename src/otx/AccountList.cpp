// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "internal/otx/AccountList.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>

#include "internal/api/session/Wallet.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Helpers.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::otx::internal
{
AccountList::AccountList(const api::Session& api)
    : api_(api)
    , acctType_(Account::voucher)
    , mapAcctIDs_{}
{
}

AccountList::AccountList(const api::Session& api, Account::AccountType acctType)
    : api_(api)
    , acctType_(acctType)
    , mapAcctIDs_{}
{
}

void AccountList::Serialize(Tag& parent) const
{
    auto acctType = String::Factory();
    TranslateAccountTypeToString(acctType_, acctType);
    const auto sizeMapAcctIDs = mapAcctIDs_.size();

    TagPtr pTag(new Tag("accountList"));

    pTag->add_attribute("type", acctType->Get());
    pTag->add_attribute("count", std::to_string(sizeMapAcctIDs));

    for (auto& it : mapAcctIDs_) {
        UnallocatedCString instrumentDefinitionID = it.first;
        UnallocatedCString accountId = it.second;
        OT_ASSERT(
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
        LogError()(OT_PRETTY_CLASS())("Failed: Empty accountList "
                                      "'type' attribute.")
            .Flush();
        return -1;
    }

    acctType_ = TranslateAccountTypeStringToEnum(acctType);

    if (Account::err_acct == acctType_) {
        LogError()(OT_PRETTY_CLASS())("Failed: accountList 'type' "
                                      "attribute contains unknown value.")
            .Flush();
        return -1;
    }

    // Load up the account IDs.
    std::int32_t count = acctCount.Exists() ? atoi(acctCount.Get()) : 0;
    if (count > 0) {
        while (count-- > 0) {
            if (!SkipToElement(xml)) {
                LogConsole()(OT_PRETTY_CLASS())("Failure: Unable to find "
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
                    LogError()(OT_PRETTY_CLASS())(
                        "Error loading accountEntry: Either the "
                        "instrumentDefinitionID (")(
                        instrumentDefinitionID)("), or the accountID (")(
                        accountID)(") was EMPTY.")
                        .Flush();
                    return -1;
                }

                mapAcctIDs_.insert(std::make_pair(
                    instrumentDefinitionID->Get(), accountID->Get()));
            } else {
                LogError()(OT_PRETTY_CLASS())(
                    "Expected accountEntry element in accountList.")
                    .Flush();
                return -1;
            }
        }
    }

    if (!SkipAfterLoadingField(xml))  // </accountList>
    {
        LogConsole()(OT_PRETTY_CLASS())("Bad data? Expected "
                                        "EXN_ELEMENT_END here, but "
                                        "didn't get it. Returning false.")
            .Flush();
        return -1;
    }

    return 1;
}

void AccountList::Release_AcctList() { mapAcctIDs_.clear(); }

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

    if (Account::stash == acctType_) {
        if (1 > stashTransNum) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed attempt to "
                "create stash account without cron item #.")
                .Flush();

            return {};
        }
    }

    // First, we'll see if there's already an account ID available for the
    // requested instrument definition ID.
    auto acctTypeString = String::Factory();
    TranslateAccountTypeToString(acctType_, acctTypeString);
    auto acctIDsIt = mapAcctIDs_.find(instrumentDefinitionID.str());

    // Account ID *IS* already there for this instrument definition
    if (mapAcctIDs_.end() != acctIDsIt) {
        const auto& accountID = acctIDsIt->second;
        account = api_.Wallet().Internal().mutable_Account(
            Identifier::Factory(accountID), reason);

        if (account) {

            LogDebug()(OT_PRETTY_CLASS())("Successfully loaded ")(
                acctTypeString)(" account ID: ")(accountID)("Unit Type ID:: ")(
                instrumentDefinitionID.str())
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
        acctType_,
        stashTransNum,
        reason);

    if (false == bool(account)) {
        LogError()(OT_PRETTY_CLASS())("Failed trying to generate ")(
            acctTypeString)(" account with instrument definition ID: ")(
            instrumentDefinitionID)(".")
            .Flush();
    } else {
        auto acctIDString = String::Factory();
        account.get().GetIdentifier(acctIDString);

        LogConsole()(OT_PRETTY_CLASS())("Successfully created ")(
            acctTypeString)(" account ID: ")(
            acctIDString)(" Instrument Definition ID: ")(
            instrumentDefinitionID.str())
            .Flush();
        mapAcctIDs_[instrumentDefinitionID.str()] = acctIDString->Get();

        wasAcctCreated = true;
    }

    return account;
}

AccountList::~AccountList() { Release_AcctList(); }
}  // namespace opentxs::otx::internal

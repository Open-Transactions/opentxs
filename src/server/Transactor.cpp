// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Transactor.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/consensus/ClientContext.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/AccountList.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/OTFolders.hpp"

#include "MainFile.hpp"
#include "Server.hpp"

#include <cinttypes>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>

#define OT_METHOD "opentxs::Transactor::"

namespace opentxs::server
{

Transactor::Transactor(Server& server)
    : server_(server)
    , transactionNumber_(0)
    , idToBasketMap_()
    , contractIdToBasketAccountId_()
    , voucherAccounts_(server.API())
{
}

/// Just as every request must be accompanied by a request number, so
/// every transaction request must be accompanied by a transaction number.
/// The request numbers can simply be incremented on both sides (per user.)
/// But the transaction numbers must be issued by the server and they do
/// not repeat from user to user. They are unique to transaction.
///
/// Users must ask the server to send them transaction numbers so that they
/// can be used in transaction requests.
bool Transactor::issueNextTransactionNumber(
    TransactionNumber& lTransactionNumber)
{
    // transactionNumber_ stores the last VALID AND ISSUED transaction number.
    // So first, we increment that, since we don't want to issue the same number
    // twice.
    transactionNumber_++;

    // Next, we save it to file.
    if (!server_.GetMainFile().SaveMainFile()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error saving main server file.")
            .Flush();
        transactionNumber_--;
        return false;
    }

    // SUCCESS?
    // Now the server main file has saved the latest transaction number,
    // NOW we set it onto the parameter and return true.
    lTransactionNumber = transactionNumber_;
    return true;
}

bool Transactor::issueNextTransactionNumberToNym(
    ClientContext& context,
    TransactionNumber& lTransactionNumber)
{
    if (!issueNextTransactionNumber(lTransactionNumber)) { return false; }

    // Each Nym stores the transaction numbers that have been issued to it.
    // (On client AND server side.)
    //
    // So whenever the server issues a new number, it's to a specific Nym, then
    // it is recorded in his Nym file before being sent to the client (where it
    // is also recorded in his Nym file.)  That way the server always knows
    // which numbers are valid for each Nym.
    if (!context.IssueNumber(transactionNumber_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error adding transaction number to Nym file.")
            .Flush();
        transactionNumber_--;
        // Save it back how it was, since we're not issuing this number after
        // all.
        server_.GetMainFile().SaveMainFile();

        return false;
    }

    // SUCCESS?
    // Now the server main file has saved the latest transaction number,
    // NOW we set it onto the parameter and return true.
    lTransactionNumber = transactionNumber_;

    return true;
}

// Server stores a map of BASKET_ID to BASKET_ACCOUNT_ID.
bool Transactor::addBasketAccountID(
    const Identifier& BASKET_ID,
    const Identifier& BASKET_ACCOUNT_ID,
    const Identifier& BASKET_CONTRACT_ID)
{
    auto theBasketAcctID = Identifier::Factory();

    if (lookupBasketAccountID(BASKET_ID, theBasketAcctID)) {
        {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": User attempted to add Basket that already exists.")
                .Flush();
        }

        return false;
    }

    auto strBasketID = String::Factory(BASKET_ID),
         strBasketAcctID = String::Factory(BASKET_ACCOUNT_ID),
         strBasketContractID = String::Factory(BASKET_CONTRACT_ID);

    idToBasketMap_[strBasketID->Get()] = strBasketAcctID->Get();
    contractIdToBasketAccountId_[strBasketContractID->Get()] =
        strBasketAcctID->Get();

    return true;
}

/// Use this to find the basket account ID for this server (which is unique to
/// this server)
/// using the contract ID to look it up. (The basket contract ID is unique to
/// this server.)
bool Transactor::lookupBasketAccountIDByContractID(
    const Identifier& BASKET_CONTRACT_ID,
    OTIdentifier& BASKET_ACCOUNT_ID)
{
    // Server stores a map of BASKET_ID to BASKET_ACCOUNT_ID. Let's iterate
    // through that map...
    for (auto& it : contractIdToBasketAccountId_) {
        auto strBasketContractID = String::Factory(it.first.c_str());
        auto strBasketAcctID = String::Factory(it.second.c_str());

        auto id_BASKET_CONTRACT = Identifier::Factory(strBasketContractID),
             id_BASKET_ACCT = Identifier::Factory(strBasketAcctID);

        if (BASKET_CONTRACT_ID == id_BASKET_CONTRACT)  // if the basket contract
                                                       // ID passed in matches
                                                       // this one...
        {
            BASKET_ACCOUNT_ID = Identifier::Factory(id_BASKET_ACCT);
            return true;
        }
    }
    return false;
}

/// Use this to find the basket account ID for this server (which is unique to
/// this server)
/// using the contract ID to look it up. (The basket contract ID is unique to
/// this server.)
bool Transactor::lookupBasketContractIDByAccountID(
    const Identifier& BASKET_ACCOUNT_ID,
    OTIdentifier& BASKET_CONTRACT_ID)
{
    // Server stores a map of BASKET_ID to BASKET_ACCOUNT_ID. Let's iterate
    // through that map...
    for (auto& it : contractIdToBasketAccountId_) {
        auto strBasketContractID = String::Factory(it.first.c_str());
        auto strBasketAcctID = String::Factory(it.second.c_str());

        auto id_BASKET_CONTRACT = Identifier::Factory(strBasketContractID),
             id_BASKET_ACCT = Identifier::Factory(strBasketAcctID);

        if (BASKET_ACCOUNT_ID == id_BASKET_ACCT)  // if the basket contract ID
                                                  // passed in matches this
                                                  // one...
        {
            BASKET_CONTRACT_ID = Identifier::Factory(id_BASKET_CONTRACT);
            return true;
        }
    }
    return false;
}

/// Use this to find the basket account for this server (which is unique to this
/// server)
/// using the basket ID to look it up (the Basket ID is the same for all
/// servers)
bool Transactor::lookupBasketAccountID(
    const Identifier& BASKET_ID,
    OTIdentifier& BASKET_ACCOUNT_ID)
{
    // Server stores a map of BASKET_ID to BASKET_ACCOUNT_ID. Let's iterate
    // through that map...
    for (auto& it : idToBasketMap_) {
        auto strBasketID = String::Factory(it.first.c_str());
        auto strBasketAcctID = String::Factory(it.second.c_str());

        auto id_BASKET = Identifier::Factory(strBasketID),
             id_BASKET_ACCT = Identifier::Factory(strBasketAcctID);

        if (BASKET_ID == id_BASKET)  // if the basket ID passed in matches this
                                     // one...
        {
            BASKET_ACCOUNT_ID = Identifier::Factory(id_BASKET_ACCT);
            return true;
        }
    }
    return false;
}

/// Looked up the voucher account (where cashier's cheques are issued for any
/// given instrument definition) return a pointer to the account.  Since it's
/// SUPPOSED to
/// exist, and since it's being requested, also will GENERATE it if it cannot
/// be found, add it to the list, and return the pointer. Should always succeed.
ExclusiveAccount Transactor::getVoucherAccount(
    const Identifier& INSTRUMENT_DEFINITION_ID)
{
    const auto NOTARY_NYM_ID = Identifier::Factory(server_.GetServerNym()),
               NOTARY_ID = Identifier::Factory(server_.GetServerID());
    bool bWasAcctCreated = false;
    auto pAccount = voucherAccounts_.GetOrRegisterAccount(
        server_.GetServerNym(),
        NOTARY_NYM_ID,
        INSTRUMENT_DEFINITION_ID,
        NOTARY_ID,
        bWasAcctCreated);
    if (bWasAcctCreated) {
        auto strAcctID = String::Factory();
        pAccount.get().GetIdentifier(strAcctID);
        const auto strInstrumentDefinitionID =
            String::Factory(INSTRUMENT_DEFINITION_ID);
        {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Successfully created "
                "voucher account ID: ")(strAcctID)(
                " Instrument Definition ID: ")(strInstrumentDefinitionID)(".")
                .Flush();
        }
        if (!server_.GetMainFile().SaveMainFile()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error saving main "
                "server file containing new account ID!!")
                .Flush();
        }
    }

    return pAccount;
}
}  // namespace opentxs::server

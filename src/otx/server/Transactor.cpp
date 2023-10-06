// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/server/Transactor.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/core/String.hpp"
#include "internal/otx/AccountList.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/consensus/Client.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "otx/server/MainFile.hpp"
#include "otx/server/Server.hpp"

namespace opentxs::server
{
Transactor::Transactor(Server& server, const PasswordPrompt& reason)
    : server_(server)
    , reason_(reason)
    , transaction_number_(0)
    , id_to_basket_map_()
    , contract_id_to_basket_account_id_()
    , voucher_accounts_(server.API())
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
auto Transactor::issueNextTransactionNumber(
    TransactionNumber& lTransactionNumber) -> bool
{
    // transaction_number_ stores the last VALID AND ISSUED transaction number.
    // So first, we increment that, since we don't want to issue the same number
    // twice.
    transaction_number_++;

    // Next, we save it to file.
    if (!server_.GetMainFile().SaveMainFile()) {
        LogError()()("Error saving main server file.").Flush();
        transaction_number_--;
        return false;
    }

    // SUCCESS?
    // Now the server main file has saved the latest transaction number,
    // NOW we set it onto the parameter and return true.
    lTransactionNumber = transaction_number_;
    return true;
}

auto Transactor::issueNextTransactionNumberToNym(
    otx::context::Client& context,
    TransactionNumber& lTransactionNumber) -> bool
{
    if (!issueNextTransactionNumber(lTransactionNumber)) { return false; }

    // Each Nym stores the transaction numbers that have been issued to it.
    // (On client AND server side.)
    //
    // So whenever the server issues a new number, it's to a specific Nym, then
    // it is recorded in his Nym file before being sent to the client (where it
    // is also recorded in his Nym file.)  That way the server always knows
    // which numbers are valid for each Nym.
    if (!context.IssueNumber(transaction_number_)) {
        LogError()()(": Error adding transaction number to Nym file.").Flush();
        transaction_number_--;
        // Save it back how it was, since we're not issuing this number after
        // all.
        server_.GetMainFile().SaveMainFile();

        return false;
    }

    // SUCCESS?
    // Now the server main file has saved the latest transaction number,
    // NOW we set it onto the parameter and return true.
    lTransactionNumber = transaction_number_;

    return true;
}

// Server stores a map of BASKET_ID to BASKET_ACCOUNT_ID.
auto Transactor::addBasketAccountID(
    const identifier::Generic& BASKET_ID,
    const identifier::Generic& BASKET_ACCOUNT_ID,
    const identifier::Generic& BASKET_CONTRACT_ID) -> bool
{
    auto theBasketAcctID = identifier::Generic{};

    if (lookupBasketAccountID(BASKET_ID, theBasketAcctID)) {
        {
            LogConsole()()(
                ": User attempted to add Basket that already exists.")
                .Flush();
        }

        return false;
    }

    auto strBasketID = String::Factory(BASKET_ID, server_.API().Crypto()),
         strBasketAcctID =
             String::Factory(BASKET_ACCOUNT_ID, server_.API().Crypto()),
         strBasketContractID =
             String::Factory(BASKET_CONTRACT_ID, server_.API().Crypto());

    id_to_basket_map_[strBasketID->Get()] = strBasketAcctID->Get();
    contract_id_to_basket_account_id_[strBasketContractID->Get()] =
        strBasketAcctID->Get();

    return true;
}

/// Use this to find the basket account ID for this server (which is unique to
/// this server)
/// using the contract ID to look it up. (The basket contract ID is unique to
/// this server.)
auto Transactor::lookupBasketAccountIDByContractID(
    const identifier::Generic& BASKET_CONTRACT_ID,
    identifier::Generic& BASKET_ACCOUNT_ID) -> bool
{
    // Server stores a map of BASKET_ID to BASKET_ACCOUNT_ID. Let's iterate
    // through that map...
    for (auto& it : contract_id_to_basket_account_id_) {
        auto id_BASKET_CONTRACT =
            server_.API().Factory().IdentifierFromBase58(it.first);
        auto id_BASKET_ACCT =
            server_.API().Factory().IdentifierFromBase58(it.second);

        if (BASKET_CONTRACT_ID == id_BASKET_CONTRACT)  // if the basket contract
                                                       // ID passed in matches
                                                       // this one...
        {
            BASKET_ACCOUNT_ID.Assign(id_BASKET_ACCT);
            return true;
        }
    }
    return false;
}

/// Use this to find the basket account ID for this server (which is unique to
/// this server)
/// using the contract ID to look it up. (The basket contract ID is unique to
/// this server.)
auto Transactor::lookupBasketContractIDByAccountID(
    const identifier::Generic& BASKET_ACCOUNT_ID,
    identifier::Generic& BASKET_CONTRACT_ID) -> bool
{
    // Server stores a map of BASKET_ID to BASKET_ACCOUNT_ID. Let's iterate
    // through that map...
    for (auto& it : contract_id_to_basket_account_id_) {
        auto id_BASKET_CONTRACT =
            server_.API().Factory().IdentifierFromBase58(it.first);
        auto id_BASKET_ACCT =
            server_.API().Factory().IdentifierFromBase58(it.second);

        if (BASKET_ACCOUNT_ID == id_BASKET_ACCT)  // if the basket contract ID
                                                  // passed in matches this
                                                  // one...
        {
            BASKET_CONTRACT_ID.Assign(id_BASKET_CONTRACT);
            return true;
        }
    }
    return false;
}

/// Use this to find the basket account for this server (which is unique to this
/// server)
/// using the basket ID to look it up (the Basket ID is the same for all
/// servers)
auto Transactor::lookupBasketAccountID(
    const identifier::Generic& BASKET_ID,
    identifier::Generic& BASKET_ACCOUNT_ID) -> bool
{
    // Server stores a map of BASKET_ID to BASKET_ACCOUNT_ID. Let's iterate
    // through that map...
    for (auto& it : id_to_basket_map_) {
        auto id_BASKET = server_.API().Factory().IdentifierFromBase58(it.first);
        auto id_BASKET_ACCT =
            server_.API().Factory().IdentifierFromBase58(it.second);

        if (BASKET_ID == id_BASKET)  // if the basket ID passed in matches this
                                     // one...
        {
            BASKET_ACCOUNT_ID.Assign(id_BASKET_ACCT);
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
auto Transactor::getVoucherAccount(
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID)
    -> ExclusiveAccount
{
    const auto& NOTARY_NYM_ID = server_.GetServerNym().ID();
    const auto& NOTARY_ID = server_.GetServerID();
    bool bWasAcctCreated = false;
    auto pAccount = voucher_accounts_.GetOrRegisterAccount(
        server_.GetServerNym(),
        NOTARY_NYM_ID,
        INSTRUMENT_DEFINITION_ID,
        NOTARY_ID,
        bWasAcctCreated,
        reason_);
    if (bWasAcctCreated) {
        auto strAcctID = String::Factory();
        pAccount.get().GetIdentifier(strAcctID);
        const auto strInstrumentDefinitionID =
            String::Factory(INSTRUMENT_DEFINITION_ID, server_.API().Crypto());
        {
            LogConsole()()("Successfully created voucher account ID: ")(
                strAcctID.get())(" Instrument Definition ID: ")(
                strInstrumentDefinitionID.get())(".")
                .Flush();
        }
        if (!server_.GetMainFile().SaveMainFile()) {
            LogError()()(
                ": Error saving main server file containing new account ID!!")
                .Flush();
        }
    }

    return pAccount;
}
}  // namespace opentxs::server

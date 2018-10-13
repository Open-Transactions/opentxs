// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/client/Cash.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/ServerAction.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/cash/Purse.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/ServerAction.hpp"
#include "opentxs/client/Utility.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"

#include "Cash.hpp"

#define OT_METHOD "opentxs::Cash"

//#define OT_METHOD "opentxs::api::client::implementation::Cash::"

namespace opentxs
{
api::client::Cash* Factory::Cash(
    const api::client::Manager& api,
    const api::client::ServerAction& serverAction)
{
    return new api::client::implementation::Cash(api, serverAction);
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
Cash::Cash(
    const api::client::Manager& api,
    const client::ServerAction& serverAction)
    : api_(api)
    , server_action_(serverAction)
{
    // WARNING: do not access api_.Wallet() during construction
}

#if OT_CASH
bool Cash::deposit_cash(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCT_ID,
    const std::string& STR_PURSE) const
{
    return 1 == deposit_purse(notaryID, ACCT_ID, nymID, STR_PURSE, "");
}

bool Cash::deposit_local_purse(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCT_ID,
    const std::string& STR_INDICES) const
{
    return 1 == deposit_purse(notaryID, ACCT_ID, nymID, "", STR_INDICES);
}

bool Cash::easy_withdraw_cash(const std::string& ACCT_ID, std::int64_t AMOUNT)
    const
{
    // There are other high-level functions that call this low-level version.
    // (Not just the function we're currently in.)
    return 1 == easy_withdraw_cash_low_level(ACCT_ID, AMOUNT);
}

std::string Cash::export_cash(
    const std::string& notaryID,
    const std::string& FROM_nymID,
    const std::string& unitTypeID,
    const std::string& TO_nymID,
    const std::string& indices,
    bool bPasswordProtected,
    std::string& retainedCopy) const
{
    std::string strContract = api_.Exec().GetAssetType_Contract(unitTypeID);

    if (strContract.empty()) {
        otOut << "Error: cannot load asset contract.\n";
        return {};
    }
    std::string to_nym_id = TO_nymID;
    // -------------------------------------------------------------------
    std::string instrument =
        api_.Exec().LoadPurse(notaryID, unitTypeID, FROM_nymID);
    if (instrument.empty()) {
        otOut << "Error: cannot load purse.\n";
        return {};
    }
    // -------------------------------------------------------------------
    if (!bPasswordProtected && TO_nymID.empty()) {
        // If no recipient, then recipient == Nym.
        //
        otOut << "Cash::export_cash: recipientNym empty--using "
                 "'FROM_nymID' also as 'TO_nymID': "
              << FROM_nymID << "\n";
        to_nym_id = FROM_nymID;
    }
    // -------------------------------------------------------------------
    std::vector<std::string> tokens;
    if (false ==
        get_tokens(
            tokens, notaryID, FROM_nymID, unitTypeID, instrument, indices)) {
        return {};
    }
    // -------------------------------------------------------------------
    return export_cash_low_level(
        notaryID,
        unitTypeID,
        FROM_nymID,  // nymID
        instrument,  // oldPurse
        tokens,      // selectedTokens
        to_nym_id,   // recipientNymID
        bPasswordProtected,
        retainedCopy);
}

bool Cash::withdraw_and_export_cash(
    const std::string& ACCT_ID,
    const std::string& RECIPIENT_NYM_ID,
    std::int64_t AMOUNT,
    std::shared_ptr<const Purse>& recipientCopy,
    std::shared_ptr<const Purse>& senderCopy,
    bool bPasswordProtected /*=false*/) const
{
    const std::string server = api_.Exec().GetAccountWallet_NotaryID(ACCT_ID);
    const std::string mynym = api_.Exec().GetAccountWallet_NymID(ACCT_ID);
    const std::string asset_type =
        api_.Exec().GetAccountWallet_InstrumentDefinitionID(ACCT_ID);
    std::string hisnym = RECIPIENT_NYM_ID;
    std::string indices;

    return 1 == withdraw_and_export_cash_low_level(
                    server,
                    mynym,
                    asset_type,
                    ACCT_ID,
                    hisnym,
                    std::to_string(AMOUNT),
                    indices,
                    bPasswordProtected,
                    recipientCopy,
                    senderCopy);
}

bool Cash::withdraw_and_send_cash(
    const std::string& ACCT_ID,
    const std::string& recipientNymID,
    std::int64_t AMOUNT) const
{
    std::string response, indices;
    std::string hisnym = recipientNymID;
    return 1 == send_cash(
                    response,
                    "",       // server
                    "",       // mynym
                    ACCT_ID,  // myacct
                    "",       // mypurse
                    hisnym,
                    std::to_string(AMOUNT),
                    indices,
                    "");  // password.
}

// Input: notaryID, instrumentDefinitionID, Nym of current owner, existing
// purse, list of
// selected tokens, Nym of Recipient, and bool bPasswordProtected.
// Returns: "new Purse"
//
std::string Cash::export_cash_low_level(
    const std::string& notaryID,
    const std::string& instrumentDefinitionID,
    const std::string& nymID,
    const std::string& oldPurse,
    const std::vector<std::string>& selectedTokens,
    std::string& recipientNymID,
    bool bPasswordProtected,
    std::string& strRetainedCopy) const
{
    std::string token, exportedToken, exportedPurse;
    std::string newPurse;  // for recipient;
    std::string newPurseForSender;
    std::string copyOfOldPurse = oldPurse;
    // Next I create another "newPurse" by calling this function.
    const bool bSuccessProcess = process_cash_purse(
        newPurse,
        newPurseForSender,
        notaryID,
        instrumentDefinitionID,
        nymID,
        copyOfOldPurse,
        selectedTokens,
        recipientNymID,
        false,
        bPasswordProtected);

    if (bSuccessProcess) { strRetainedCopy = newPurseForSender; }

    // Whatever is returned from that function, I return here also.
    // Presumably a purse...
    //
    return newPurse;
}

std::int32_t Cash::withdraw_and_export_cash_low_level(
    const std::string& server,
    const std::string& mynym,
    const std::string& assetType,
    const std::string& myacct,
    std::string& hisnym,
    const std::string& amount,
    std::string& indices,
    bool hasPassword,
    std::shared_ptr<const Purse>& recipientCopy,
    std::shared_ptr<const Purse>& senderCopy) const
{
    std::int64_t startAmount = "" == amount ? 0 : stoll(amount);

    // What we want to do from here is, see if we can send the cash purely using
    // cash we already have in the local purse. If so, we just package it up and
    // send it off using send_user_payment.
    //
    // But if we do NOT have the proper cash tokens in the local purse to send,
    // then we need to withdraw enough tokens until we do, and then try sending
    // again.

    std::int64_t remain = startAmount;
    if (!get_purse_indices_or_amount(
            server, mynym, assetType, remain, indices)) {
        if (!indices.empty()) {
            otOut << "Error: invalid purse indices.\n";
            return -1;
        }

        // Not enough cash found in existing purse to match the amount
        if (1 != easy_withdraw_cash_low_level(myacct, remain)) {
            otOut << "Error: cannot withdraw cash.\n";
            return -1;
        }

        remain = startAmount;
        if (!get_purse_indices_or_amount(
                server, mynym, assetType, remain, indices)) {
            otOut
                << "Error: cannot retrieve purse indices. "
                   "(It's possible that you have enough cash, yet lack the "
                   "correct "
                   "denominations of token for the exact amount requested).\n";
            return -1;
        }
    }

    std::string retainedCopy = "";
    std::string exportedCash = export_cash(
        server, mynym, assetType, hisnym, indices, hasPassword, retainedCopy);
    if (exportedCash.empty()) {
        otOut << "Error: cannot export cash.\n";
        return -1;
    }
    // By this point, exportedCash and retainedCopy should both be valid.

    auto pRecipientCopy{api_.Factory().Purse(String::Factory(exportedCash))};
    auto pSenderCopy{api_.Factory().Purse(String::Factory(retainedCopy))};

    OT_ASSERT(pRecipientCopy);
    OT_ASSERT(pSenderCopy);

    recipientCopy.reset(pRecipientCopy.release());
    senderCopy.reset(pSenderCopy.release());

    return 1;
}

std::int32_t Cash::send_cash(
    std::string& response,
    const std::string& server,
    const std::string& mynym,
    const std::string& assetType,
    const std::string& myacct,
    std::string& hisnym,
    const std::string& amount,
    std::string& indices,
    bool hasPassword) const
{
    std::int64_t startAmount = (amount.empty() ? 0 : stoll(amount));

    // What we want to do from here is, see if we can send the cash purely using
    // cash we already have in the local purse. If so, we just package it up and
    // send it off using send_user_payment.
    //
    // But if we do NOT have the proper cash tokens in the local purse to send,
    // then we need to withdraw enough tokens until we do, and then try sending
    // again.

    std::int64_t remain = startAmount;
    if (false == get_purse_indices_or_amount(
                     server, mynym, assetType, remain, indices)) {
        if (!indices.empty()) {
            otOut << "Error: invalid purse indices.\n";
            return -1;
        }

        // Not enough cash found in existing purse to match the amount
        if (1 != easy_withdraw_cash_low_level(myacct, remain)) {
            otOut << "Error: cannot withdraw cash.\n";
            return -1;
        }

        remain = startAmount;
        if (false == get_purse_indices_or_amount(
                         server, mynym, assetType, remain, indices)) {
            otOut
                << "Error: cannot retrieve purse indices. "
                   "(It's possible that you have enough cash, yet lack the "
                   "correct "
                   "denominations of token for the exact amount requested).\n";
            return -1;
        }
    }

    std::string retainedCopy;
    std::string exportedCash = export_cash(
        server, mynym, assetType, hisnym, indices, hasPassword, retainedCopy);
    if (exportedCash.empty()) {
        otOut << "Error: cannot export cash.\n";
        return -1;
    }
    // By this point, exportedCash and retainedCopy should both be valid.

    auto recipientCopy{api_.Factory().Purse(String::Factory(exportedCash))};
    auto senderCopy{api_.Factory().Purse(String::Factory(retainedCopy))};

    OT_ASSERT(recipientCopy);
    OT_ASSERT(senderCopy);

    std::shared_ptr<const Purse> precipientCopy{recipientCopy.release()};
    std::shared_ptr<const Purse> psenderCopy{senderCopy.release()};

    response = server_action_
                   .SendCash(
                       Identifier::Factory(mynym),
                       Identifier::Factory(server),
                       Identifier::Factory(hisnym),
                       precipientCopy,
                       psenderCopy)
                   ->Run();
    if (1 != VerifyMessageSuccess(api_, response)) {
        // cannot send cash so try to re-import into sender's purse
        if (!api_.Exec().Wallet_ImportPurse(
                server, assetType, mynym, retainedCopy)) {
            otOut << "Error: cannot send cash AND failed re-importing purse."
                  << "\nServer: " << server << "\nAsset Type: " << assetType
                  << "\nNym: " << mynym
                  << "\n\nPurse (SAVE THIS SOMEWHERE!):\n\n"
                  << retainedCopy << "\n";
            return -1;
        }

        // at least re-importing succeeeded
        otOut << "Error: cannot send cash.\n";
        return -1;
    }

    return 1;
}

// If you pass the indices, this function returns true if those exact indices
// exist. In that case, this function will also set remain to the total.
//
// If, instead, you pass remain and a blank indices, this function will try to
// determine the indices that would create remain, if they were selected.
//
bool Cash::get_purse_indices_or_amount(
    const std::string& server,
    const std::string& mynym,
    const std::string& assetType,
    std::int64_t& remain,
    std::string& indices) const
{
    bool findAmountFromIndices = "" != indices && 0 == remain;
    bool findIndicesFromAmount = "" == indices && 0 != remain;
    if (!findAmountFromIndices && !findIndicesFromAmount) {
        otOut << "Error: invalid parameter combination.\n";
        return false;
    }

    std::string purse = api_.Exec().LoadPurse(server, assetType, mynym);
    if (purse.empty()) {
        otOut << "Error: cannot load purse.\n";
        return false;
    }

    std::int32_t items = api_.Exec().Purse_Count(server, assetType, purse);
    if (0 > items) {
        otOut << "Error: cannot load purse item count.\n\n";
        return false;
    }

    if (0 == items) {
        otOut << "Error: the purse is empty.\n\n";
        return false;
    }

    for (std::int32_t i = 0; i < items; i++) {
        std::string token =
            api_.Exec().Purse_Peek(server, assetType, mynym, purse);
        if (token.empty()) {
            otOut << "Error:cannot load token from purse.\n";
            return false;
        }

        purse = api_.Exec().Purse_Pop(server, assetType, mynym, purse);
        if (purse.empty()) {
            otOut << "Error: cannot load updated purse.\n";
            return false;
        }

        std::int64_t denomination =
            api_.Exec().Token_GetDenomination(server, assetType, token);
        if (0 >= denomination) {
            otOut << "Error: cannot get token denomination.\n";
            return false;
        }

        time64_t validTo =
            api_.Exec().Token_GetValidTo(server, assetType, token);
        if (OT_TIME_ZERO > validTo) {
            otOut << "Error: cannot get token validTo.\n";
            return false;
        }

        time64_t time = api_.Exec().GetTime();
        if (OT_TIME_ZERO > time) {
            otOut << "Error: cannot get token time.\n";
            return false;
        }

        if (time > validTo) {
            otOut << "Skipping: token is expired.\n";
            continue;
        }

        if (findAmountFromIndices) {
            if ("all" == indices ||
                api_.Exec().NumList_VerifyQuery(indices, std::to_string(i))) {
                remain += denomination;
            }
            continue;
        }

        // TODO: There could be a denomination order that will cause this
        // function to fail, even though there is a denomination combination
        // that would make it succeeed. Example: try to find 6 when the
        // denominations are: 5, 2, 2, and 2. This will not succeed since it
        // will use the 5 first and then cannot satisfy the remaining 1 even
        // though the three 2's would satisfy the 6...

        if (denomination <= remain) {
            indices = api_.Exec().NumList_Add(indices, std::to_string(i));
            remain -= denomination;
            if (0 == remain) { return true; }
        }
    }

    return findAmountFromIndices ? true : false;
}

std::int32_t Cash::deposit_purse(
    const std::string& server,
    const std::string& myacct,
    const std::string& mynym,
    std::string instrument,
    const std::string& indices,
    std::string* pOptionalOutput /*=nullptr*/) const  // contains server reply
{
    std::string assetType =
        api_.Exec().GetAccountWallet_InstrumentDefinitionID(myacct);
    if (assetType.empty()) {
        otOut << "Error: cannot get unit type from acct purse.\n";
        return -1;
    }

    if (!instrument.empty()) {
        std::vector<std::string> tokens;
        return deposit_purse_low_level(
            server,
            assetType,
            mynym,
            instrument,
            tokens,
            myacct,
            false,
            pOptionalOutput);
    }

    // we have to load the purse ourselves
    instrument = api_.Exec().LoadPurse(server, assetType, mynym);
    if (instrument.empty()) {
        otOut << "Error: cannot load purse.\n";
        return -1;
    }

    std::vector<std::string> tokens;
    if (false ==
        get_tokens(tokens, server, mynym, assetType, instrument, indices)) {
        return -1;
    }

    return deposit_purse_low_level(
        server,
        assetType,
        mynym,
        instrument,
        tokens,
        myacct,
        true,
        pOptionalOutput);
}

std::int32_t Cash::deposit_purse_low_level(
    const std::string& notaryID,
    const std::string& unitTypeID,
    const std::string& nymID,
    const std::string& oldPurse,
    const std::vector<std::string>& selectedTokens,
    const std::string& accountID,
    const bool bReimportIfFailure,  // So we don't re-import a purse that wasn't
                                    // internal to begin with.
    std::string* pOptionalOutput /*=nullptr*/) const  // copy of server
                                                      // response.
{
    std::string recipientNymID = api_.Exec().GetAccountWallet_NymID(accountID);
    if (!VerifyStringVal(recipientNymID)) {
        otOut << "deposit_purse_low_level: Unable to find recipient "
                 "Nym based on myacct. \n";
        return -1;
    }

    bool bPasswordProtected = api_.Exec().Purse_HasPassword(notaryID, oldPurse);

    std::string newPurse;                // being deposited.;
    std::string newPurseForSender = "";  // Probably unused in this case.;
    std::string copyOfOldPurse = oldPurse;
    bool bSuccessProcess = process_cash_purse(
        newPurse,
        newPurseForSender,
        notaryID,
        unitTypeID,
        nymID,
        copyOfOldPurse,
        selectedTokens,
        recipientNymID,
        bPasswordProtected,
        false);

    if (!bSuccessProcess || !VerifyStringVal(newPurse)) {
        otOut << "deposit_purse_low_level: new Purse is empty, after "
                 "processing "
                 "it for deposit. \n";
        return -1;
    }

    auto purse{api_.Factory().Purse(String::Factory(newPurse))};

    OT_ASSERT(purse);

    auto action = server_action_.DepositCashPurse(
        Identifier::Factory(nymID),
        Identifier::Factory(notaryID),
        Identifier::Factory(accountID),
        purse);
    std::string strResponse = action->Run();
    std::string strAttempt = "deposit_cash";

    // HERE, WE INTERPRET THE SERVER REPLY, WHETHER SUCCESS, FAIL, OR ERROR...

    std::int32_t nInterpretReply = InterpretTransactionMsgReply(
        api_, notaryID, recipientNymID, accountID, strAttempt, strResponse);

    if (1 == nInterpretReply) {

        if (nullptr != pOptionalOutput) { *pOptionalOutput = strResponse; }

        // Download all the intermediary files (account balance, inbox, outbox,
        // etc)
        // since they have probably changed from this operation.
        //
        bool bRetrieved = server_action_.DownloadAccount(
            Identifier::Factory(recipientNymID),
            Identifier::Factory(notaryID),
            Identifier::Factory(accountID),
            true);  // bForceDownload defaults to false.;

        otOut << "\nServer response (" << strAttempt
              << "): SUCCESS depositing cash!\n";
        otOut << std::string(bRetrieved ? "Success" : "Failed")
              << " retrieving intermediary files for account.\n";
    } else  // failure. (so we re-import the cash, so as not to lose it...)
    {

        if (!bPasswordProtected && bReimportIfFailure) {
            bool importStatus = api_.Exec().Wallet_ImportPurse(
                notaryID, unitTypeID, recipientNymID, newPurse);
            otOut << "Since failure in deposit_purse_low_level, "
                     "OT_API_Wallet_ImportPurse called. Status of "
                     "import: "
                  << importStatus << "\n";

            if (!importStatus) {
                // Raise the alarm here that we failed depositing the purse, and
                // then we failed
                // importing it back into our wallet again.
                otOut << "Error: Failed depositing the cash purse, and then "
                         "failed re-importing it back to wallet. Therefore YOU "
                         "must copy the purse NOW and save it to a safe place! "
                         "\n";

                otOut << newPurse << "\n";

                otOut << "AGAIN: Be sure to copy the above purse "
                         "to a safe place, since it FAILED to "
                         "deposit and FAILED to re-import back "
                         "into the wallet. \n";
            }
        } else {
            otOut << "Error: Failed depositing the cash purse. "
                     "Therefore YOU must copy the purse NOW and "
                     "save it to a safe place! \n";

            otOut << newPurse << "\n";

            otOut << "AGAIN: Be sure to copy the above purse to a "
                     "safe place, since it FAILED to deposit. \n";
        }

        return -1;
    }

    //
    // Return status to caller.
    //
    return nInterpretReply;
}

std::int32_t Cash::easy_withdraw_cash_low_level(
    const std::string& myacct,
    std::int64_t amount) const
{
    std::string server = api_.Exec().GetAccountWallet_NotaryID(myacct);
    if (server.empty()) {
        otOut << "Error: cannot determine server from myacct.\n";
        return -1;
    }

    std::string mynym = api_.Exec().GetAccountWallet_NymID(myacct);
    if (mynym.empty()) {
        otOut << "Error: cannot determine mynym from myacct.\n";
        return -1;
    }

    std::string assetType =
        api_.Exec().GetAccountWallet_InstrumentDefinitionID(myacct);
    if (assetType.empty()) { return -1; }

    const opentxs::OTIdentifier theNotaryID = Identifier::Factory(server),
                                theNymID = Identifier::Factory(mynym),
                                theAssetType = Identifier::Factory(assetType),
                                theAcctID = Identifier::Factory(myacct);

    std::string assetContract = api_.Exec().GetAssetType_Contract(assetType);
    if (assetContract.empty()) {
        std::string response =
            server_action_
                .DownloadContract(theNymID, theNotaryID, theAssetType)
                ->Run();
        if (1 != VerifyMessageSuccess(api_, response)) {
            otOut << "Error: cannot retrieve asset contract.\n";
            return -1;
        }

        assetContract = api_.Exec().GetAssetType_Contract(assetType);
        if (assetContract.empty()) {
            otOut << "Error: cannot load asset contract.\n";
            return -1;
        }
    }

    std::string mint = load_or_retrieve_mint(server, mynym, assetType);
    if (mint.empty()) {
        otOut << "Error: cannot load asset mint.\n";
        return -1;
    }

    std::string response =
        server_action_.WithdrawCash(theNymID, theNotaryID, theAcctID, amount)
            ->Run();
    std::int32_t reply = InterpretTransactionMsgReply(
        api_, server, mynym, myacct, "withdraw_cash", response);
    if (1 != reply) { return reply; }

    if (!server_action_.DownloadAccount(
            theNymID, theNotaryID, theAcctID, true)) {
        otOut << "Error retrieving intermediary files for account.\n";
        return -1;
    }

    return 1;
}

// CHECK USER (download a public key)
//
std::string Cash::check_nym(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& targetNymID) const
{
    auto action = server_action_.DownloadNym(
        opentxs::Identifier::Factory(nymID),
        opentxs::Identifier::Factory(notaryID),
        opentxs::Identifier::Factory(targetNymID));

    return action->Run();
}

// LOAD MINT (from local storage)
//
// To load a mint withOUT retrieving it from server, call:
//
// var strMint = OT_API_LoadMint(notaryID, instrumentDefinitionID);
// It returns the mint, or null.
// LOAD MINT (from local storage).
// Also, if necessary, RETRIEVE it from the server first.
//
// Returns the mint, or null.
//
std::string Cash::load_or_retrieve_mint(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& unitTypeID) const
{
    std::string response = check_nym(notaryID, nymID, nymID);

    if (1 != VerifyMessageSuccess(api_, response)) {
        otOut << "load_or_retrieve_mint: Cannot verify nym for "
                 "IDs: \n";
        otOut << "   Notary ID: " << notaryID << "\n";
        otOut << "      Nym ID: " << nymID << "\n";
        otOut << "Unit Type Id: " << unitTypeID << "\n";
        return "";
    }

    // HERE, WE MAKE SURE WE HAVE THE PROPER MINT...
    //
    // Download the public mintfile if it's not there, or if it's expired.
    // Also load it up into memory as a std::string (just to make sure it
    // works.)

    // expired or missing.
    if (!api_.Exec().Mint_IsStillGood(notaryID, unitTypeID)) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Mint file is ")(
            "missing or expired. Downloading from ")("server...")
            .Flush();

        response = server_action_
                       .DownloadMint(
                           Identifier::Factory(nymID),
                           Identifier::Factory(notaryID),
                           Identifier::Factory(unitTypeID))
                       ->Run();

        if (1 != VerifyMessageSuccess(api_, response)) {
            otOut << "load_or_retrieve_mint: Unable to "
                     "retrieve mint for IDs: \n";
            otOut << "   Notary ID: " << notaryID << "\n";
            otOut << "      Nym ID: " << nymID << "\n";
            otOut << "Unit Type Id: " << unitTypeID << "\n";
            return "";
        }

        if (!api_.Exec().Mint_IsStillGood(notaryID, unitTypeID)) {
            otOut << "load_or_retrieve_mint: Retrieved "
                     "mint, but still 'not good' for IDs: \n";
            otOut << "   Notary ID: " << notaryID << "\n";
            otOut << "      Nym ID: " << nymID << "\n";
            otOut << "Unit Type Id: " << unitTypeID << "\n";
            return "";
        }
    }
    // else // current mint IS available already on local storage (and not
    // expired.)

    // By this point, the mint is definitely good, whether we had to download it
    // or not.
    // It's here, and it's NOT expired. (Or we would have returned already.)

    std::string strMint = api_.Exec().LoadMint(notaryID, unitTypeID);
    if (!VerifyStringVal(strMint)) {
        otOut << "load_or_retrieve_mint: Unable to load mint "
                 "for IDs: \n";
        otOut << "   Notary ID: " << notaryID << "\n";
        otOut << "      Nym ID: " << nymID << "\n";
        otOut << "Unit Type Id: " << unitTypeID << "\n";
    }

    return strMint;
}

// process_cash_purse pops the selected tokens off of oldPurse, changes
// their owner to recipientNymID, and pushes them onto newPurse, owned
// by recipientNymID. Meanwhile any unselected tokens are pushed onto
// newPurseUnselectedTokens, owned by nymID, which is saved to local
// storage (overwriting...) newPurse is returned from this function.
//
// Thus, for example, if there's a problem
// depositing/exporting/exchanging newPurse, then it needs to be
// merged back into the old purse again, which is otherwise still
// missing those tokens. (We should keep a copy of them in the
// payments outbox or something like that, in the meantime.)
//
// What is newPurseForSender for? Sometimes when process_cash_purse is
// called, a Nym is just removing tokens from his purse so he can
// deposit them into his own account. Meaning the "sender" Nym (who
// owns the purse) is the same as the "recipient" Nym (who owns the
// account.) In which case, newPurse (for recipient) and
// newPurseForSender will contain the exact same thing. But in the
// case where they are different Nyms (like if I am exporting these
// tokens from my purse in order to give them to someone else) then I
// will want a copy of the exported purse for the recipient, and a
// copy of the same exported purse for the sender. process_cash_purse
// returns both of those for me. The reason is because otherwise, if I
// just encrypted some of my cash tokens to some external Nym, then
// only THAT Nym's private key will ever open them up again. Even if I
// have them in my outpayments box, I can never recover them because I
// can never again decrypt them. Therefore I make a special copy of
// the outgoing purse which is encrypted to my own key, and put that
// into my outpayments box instead of the one I sent to the recipient
// (or including both of them.) That way the cash can still be
// recovered if necessary, from my outpayments box, and re-imported
// back into my cash purse again.
//
bool Cash::process_cash_purse(
    std::string& newPurse,
    std::string& newPurseForSender,
    const std::string& notaryID,
    const std::string& instrumentDefinitionID,
    const std::string& nymID,
    std::string& oldPurse,
    const std::vector<std::string>& selectedTokens,
    const std::string& recipientNymID,
    bool bPWProtectOldPurse,
    bool bPWProtectNewPurse) const
{
    // By this point, we know that "selected tokens" has a size of 0, or MORE
    // THAN ONE. (But NOT 1 exactly.)
    // (At least, if this function was called by exportCashPurse.)
    std::string strLocation = "process_cash_purse";

    // This block handles cases where NO TOKENS ARE SELECTED.
    //
    // (Meaning: "PROCESS" THEM ALL.)
    //
    if (selectedTokens.size() < 1) {
        // newPurse is created, OWNED BY RECIPIENT.
        newPurse =
            (bPWProtectNewPurse ? api_.Exec().CreatePurse_Passphrase(
                                      notaryID, instrumentDefinitionID, nymID)
                                : api_.Exec().CreatePurse(
                                      notaryID,
                                      instrumentDefinitionID,
                                      recipientNymID,
                                      nymID));  // recipientNymID is owner,
                                                // nymID is signer;

        if (!VerifyStringVal(newPurse)) {
            otOut << strLocation << ": "
                  << (bPWProtectNewPurse ? "OT_API_CreatePurse_Passphrase"
                                         : "OT_API_CreatePurse")
                  << " returned null\n";
            return false;
        }

        // We'll create an extra copy of the newPurse, which is encrypted to
        // the sender (instead of the recipient or to some passphrase.) We'll
        // call it newPurseForSender. This way the sender can later have the
        // option to recover the cash from his outbox.
        //
        newPurseForSender = api_.Exec().CreatePurse(
            notaryID,
            instrumentDefinitionID,
            nymID,
            nymID);  // nymID is owner, nymID is signer;

        if (!VerifyStringVal(newPurseForSender)) {
            otOut << strLocation
                  << ": Failure: OT_API_CreatePurse returned null\n";
            return false;
        }

        // Iterate through the OLD PURSE. (as tempOldPurse.)
        //
        std::int32_t count =
            api_.Exec().Purse_Count(notaryID, instrumentDefinitionID, oldPurse);
        std::string tempOldPurse = oldPurse;

        for (std::int32_t i = 0; i < count; ++i) {
            // Peek into TOKEN, from the top token on the stack. (And it's STILL
            // on top after this call.)
            //
            std::string token = api_.Exec().Purse_Peek(
                notaryID, instrumentDefinitionID, nymID, tempOldPurse);

            // Now pop the token off of tempOldPurse (our iterator for the old
            // purse).
            // Store updated copy of purse (sans token) into "str1".
            //
            std::string str1 = api_.Exec().Purse_Pop(
                notaryID, instrumentDefinitionID, nymID, tempOldPurse);

            if (!VerifyStringVal(token) || !VerifyStringVal(str1)) {
                otOut << strLocation
                      << ": OT_API_Purse_Peek or OT_API_Purse_Pop "
                         "returned null... SHOULD NEVER HAPPEN. "
                         "Returning null.\n";
                return false;
            }

            // Since pop succeeded, copy the output to tempOldPurse
            // (for next iteration, in case any continues happen below.)
            // Now tempOldPurse contains what it did before, MINUS ONE TOKEN.
            // (The exported one.)
            //
            tempOldPurse = str1;

            std::string strSender = bPWProtectOldPurse ? oldPurse : nymID;
            std::string strRecipient =
                bPWProtectNewPurse ? newPurse : recipientNymID;

            std::string strSenderAsRecipient =
                nymID;  // Used as the "owner" of newPurseForSender. (So the
                        // sender can recover his sent coins that got encrypted
                        // to someone else's key.);

            // Change the OWNER on token, from NymID to RECIPIENT.
            // (In this block, we change ALL the tokens in the purse.)
            //
            std::string exportedToken = api_.Exec().Token_ChangeOwner(
                notaryID,
                instrumentDefinitionID,
                token,
                nymID,          // signer ID
                strSender,      // old owner
                strRecipient);  // new owner
            // If change failed, then continue.
            //
            if (!VerifyStringVal(exportedToken)) {
                otOut << strLocation
                      << ": 1, OT_API_Token_ChangeOwner "
                         "returned null...(should never "
                         "happen) Returning null.\n";
                return false;
            }

            // SAVE A COPY FOR THE SENDER...
            //
            std::string retainedToken = api_.Exec().Token_ChangeOwner(
                notaryID,
                instrumentDefinitionID,
                token,
                nymID,                  // signer ID
                strSender,              // old owner
                strSenderAsRecipient);  // new owner
            // If change failed, then continue.
            //
            if (!VerifyStringVal(retainedToken)) {
                otOut << strLocation
                      << ":  2, OT_API_Token_ChangeOwner "
                         "returned null...(should never "
                         "happen) Returning null.\n";
                return false;
            }

            //          strSender    = bPWProtectOldPurse ? "" : nymID //
            //          unused, not needed
            strRecipient = bPWProtectNewPurse ? "" : recipientNymID;

            // PUSH the EXPORTED TOKEN (new owner) into the new purse (again,
            // recipient/newPurse is new owner) and save results in
            // "strPushedForRecipient".
            // Results are, FYI, newPurse+exportedToken.
            //
            std::string strPushedForRecipient = api_.Exec().Purse_Push(
                notaryID,
                instrumentDefinitionID,
                nymID,         // server, asset, signer
                strRecipient,  // owner is either nullptr (for
                               // password-protected
                               // purse) or recipientNymID
                newPurse,
                exportedToken);  // purse, token

            // If push failed, then continue.
            if (!VerifyStringVal(strPushedForRecipient)) {
                otOut << strLocation
                      << ":  OT_API_Purse_Push 1 returned null... "
                         "(should never happen) Returning null.\n";
                return false;
            }

            // PUSH the RETAINED TOKEN (copy for original owner) into the
            // newPurseForSender and save results in "strPushedForRetention".
            // Results are, FYI, newPurseForSender+retainedToken.
            //
            std::string strPushedForRetention = api_.Exec().Purse_Push(
                notaryID,
                instrumentDefinitionID,
                nymID,                 // server, asset, signer
                strSenderAsRecipient,  // This version of the purse is the
                // outgoing copy (for the SENDER's notes).
                // Thus strSenderAsRecipient.
                newPurseForSender,
                retainedToken);  // purse, token

            // If push failed, then continue.
            if (!VerifyStringVal(strPushedForRetention)) {
                otOut << strLocation
                      << ":  OT_API_Purse_Push 2 returned null... "
                         "(should never happen) Returning null.\n";
                return false;
            }

            // Since push succeeded, copy "strPushedForRecipient" (containing
            // newPurse + exportedToken) into newPurse.
            // Since push succeeded, copy "strPushedForRetention" (containing
            // newPurseForSender + retainedToken) into newPurseForSender.
            //
            newPurse = strPushedForRecipient;
            newPurseForSender = strPushedForRetention;
        }  // for

        // Save tempOldPurse to local storage. (For OLD Owner.)
        // By now, all of the tokens have been popped off of this purse, so it
        // is EMPTY.
        // We're now saving the empty purse, since the user exported all of the
        // tokens.
        //
        // THERE MAYBE SHOULD BE AN EXTRA MODAL HERE, that says,
        // "Moneychanger will now save your purse, EMPTY, back to local storage.
        // Are you sure you want to do this?"
        //
        if (!bPWProtectOldPurse)  // If old purse is NOT password-protected
                                  // (that is, it's encrypted to a Nym.)
        {
            if (!api_.Exec().SavePurse(
                    notaryID,
                    instrumentDefinitionID,
                    nymID,
                    tempOldPurse))  // if FAILURE.
            {
                // No error message if saving fails??
                // No modal?
                //
                // FT: adding log.
                otOut << strLocation
                      << ": OT_API_SavePurse "
                         "FAILED. SHOULD NEVER HAPPEN!!!!!!\n";
                return false;
            }
        } else  // old purse IS password protected. (So return its updated
                // version.)
        {
            oldPurse =
                tempOldPurse;  // We never cared about this with Nym-owned
                               // old purse, since it saves to storage
                               // anyway, in the above block. But now in
                               // the case of password-protected purses,
                               // we set the oldPurse to contain the new
                               // version of itself (containing the tokens
                               // that had been left unselected) so the
                               // caller can do what he wills with it.;
        }
    }

    // Else, SPECIFIC TOKENS were selected, so process those ONLY...
    //
    else {
        //      otOut << "Tokens in Cash Purse being processed");

        // newPurseSelectedTokens is created (CORRECTLY) with recipientNymID as
        // owner. (Or with a symmetric key / passphrase.)
        // newPurseUnSelectedTokens is created (CORRECTLY) with NymID as owner.
        // (Unselected tokens aren't being exported...)
        //
        std::string newPurseUnSelectedTokens = api_.Exec().Purse_Empty(
            notaryID,
            instrumentDefinitionID,
            nymID,
            oldPurse);  // Creates an empty copy of oldPurse.;
        std::string newPurseSelectedTokens =
            (bPWProtectNewPurse ? api_.Exec().CreatePurse_Passphrase(
                                      notaryID, instrumentDefinitionID, nymID)
                                : api_.Exec().CreatePurse(
                                      notaryID,
                                      instrumentDefinitionID,
                                      recipientNymID,
                                      nymID));  // recipientNymID = owner,
                                                // nymID = signer;
        std::string newPurseSelectedForSender = api_.Exec().CreatePurse(
            notaryID,
            instrumentDefinitionID,
            nymID,
            nymID);  // nymID = owner, nymID = signer. This is a copy of
                     // newPurseSelectedTokens that's encrypted to the SENDER
                     // (for putting in his outpayments box, so he can still
                     // decrypt if necessary.);

        if (!VerifyStringVal(newPurseSelectedForSender)) {
            otOut << strLocation << ":  OT_API_CreatePurse returned null\n";
            return false;
        }
        if (!VerifyStringVal(newPurseSelectedTokens)) {
            otOut << strLocation
                  << ":  OT_API_CreatePurse or "
                     "OT_API_CreatePurse_Passphrase returned null\n";
            return false;
        }
        if (!VerifyStringVal((newPurseUnSelectedTokens))) {
            otOut << strLocation << ":  OT_API_Purse_Empty returned null\n";
            return false;
        }

        // Iterate through oldPurse, using tempOldPurse as iterator.
        //
        std::int32_t count =
            api_.Exec().Purse_Count(notaryID, instrumentDefinitionID, oldPurse);
        std::string tempOldPurse = oldPurse;

        for (std::int32_t i = 0; i < count; ++i) {
            // Peek at the token on top of the stack.
            // (Without removing it.)
            //
            std::string token = api_.Exec().Purse_Peek(
                notaryID, instrumentDefinitionID, nymID, tempOldPurse);

            // Remove the top token from the stack, and return the updated stack
            // in "str1".
            //
            std::string str1 = api_.Exec().Purse_Pop(
                notaryID, instrumentDefinitionID, nymID, tempOldPurse);

            if (!VerifyStringVal(str1) || !VerifyStringVal(token)) {
                otOut << strLocation
                      << ":  OT_API_Purse_Peek or "
                         "OT_API_Purse_Pop returned null... returning Null. "
                         "(SHOULD NEVER HAPPEN.)\n";
                return false;
            }

            // Putting updated purse into iterator, so any subsequent continues
            // will work properly.
            //
            tempOldPurse = str1;

            // Grab the TokenID for that token. (Token still has OLD OWNER.)
            //
            std::string tokenID = api_.Exec().Token_GetID(
                notaryID, instrumentDefinitionID, token);

            if (!VerifyStringVal(tokenID)) {
                otOut << strLocation
                      << ":  OT_API_Token_GetID returned null... "
                         "SHOULD NEVER HAPPEN. Returning now.\n";
                return false;
            }

            // At this point, we check TokenID (identifying the current token)
            // to see if it's on the SELECTED LIST.
            //
            if (find(
                    selectedTokens.begin(),
                    selectedTokens.end(),
                    tokenID) != selectedTokens.end())  // We ARE exporting this
                                                       // token. (Its ID was on
                                                       // the list.)
            {
                // CHANGE OWNER from NYM to RECIPIENT
                // "token" will now contain the EXPORTED TOKEN, with the
                // NEW OWNER.
                //
                std::string strSender = bPWProtectOldPurse ? oldPurse : nymID;
                std::string strRecipient = bPWProtectNewPurse
                                               ? newPurseSelectedTokens
                                               : recipientNymID;

                std::string strSenderAsRecipient =
                    nymID;  // Used as the "owner" of newPurseSelectedForSender.
                // (So the sender can recover his sent coins that got
                // encrypted to someone else's key.);

                std::string exportedToken = api_.Exec().Token_ChangeOwner(
                    notaryID,
                    instrumentDefinitionID,
                    token,          // server, asset, token,;
                    nymID,          // signer nym
                    strSender,      // old owner
                    strRecipient);  // new owner
                if (!VerifyStringVal(exportedToken)) {
                    otOut << strLocation
                          << ": 1  OT_API_Token_ChangeOwner "
                             "returned null... SHOULD NEVER "
                             "HAPPEN. Returning now.\n";
                    return false;
                }

                std::string retainedToken = api_.Exec().Token_ChangeOwner(
                    notaryID,
                    instrumentDefinitionID,
                    token,                  // server, asset, token,;
                    nymID,                  // signer nym
                    strSender,              // old owner
                    strSenderAsRecipient);  // new owner
                if (!VerifyStringVal(retainedToken)) {
                    otOut << strLocation
                          << ": 2  OT_API_Token_ChangeOwner "
                             "returned null... SHOULD NEVER "
                             "HAPPEN. Returning now.\n";
                    return false;
                }

                // Push exported version of token into new purse for recipient
                // (for selected tokens.)
                //
                //              strSender    = bPWProtectOldPurse ? "" : nymID
                //              // Not needed.
                strRecipient = bPWProtectNewPurse ? "" : recipientNymID;

                std::string strPushedForRecipient = api_.Exec().Purse_Push(
                    notaryID,
                    instrumentDefinitionID,
                    nymID,         // server, asset, signer;
                    strRecipient,  // owner is either nullptr (for
                    // password-protected purse) or recipientNymID
                    newPurseSelectedTokens,
                    exportedToken);  // purse, token
                if (!VerifyStringVal(strPushedForRecipient)) {
                    otOut << strLocation
                          << ":  OT_API_Purse_Push "
                             "newPurseSelectedTokens returned null... "
                             "SHOULD NEVER HAPPEN (returning.)\n";
                    return false;
                }

                // Done: push a copy of these into a purse for the original
                // owner as well, so he has his OWN copy to save in his
                // payments outbox (that HE can decrypt...) so if the cash
                // is lost, for example, he can still recover it. If the
                // recipient receives it and deposits it correctly, the cash
                // in your payment outbox is now worthless and can be discarded,
                // although its existence may be valuable to you as a receipt.
                //
                std::string strPushedForRetention = api_.Exec().Purse_Push(
                    notaryID,
                    instrumentDefinitionID,
                    nymID,  // server, asset, signer;
                    strSenderAsRecipient,
                    newPurseSelectedForSender,
                    retainedToken);  // purse, token
                if (!VerifyStringVal(strPushedForRetention)) {
                    otOut << strLocation
                          << ":  OT_API_Purse_Push "
                             "newPurseSelectedForSender returned null... "
                             "SHOULD NEVER HAPPEN (returning.)\n";
                    return false;
                }

                newPurseSelectedTokens = strPushedForRecipient;
                newPurseSelectedForSender = strPushedForRetention;

            } else  // The token, this iteration, is NOT being exported, but is
                    // remaining with the original owner.
            {
                std::string strSender = bPWProtectOldPurse ? "" : nymID;

                std::string str = api_.Exec().Purse_Push(
                    notaryID,
                    instrumentDefinitionID,
                    nymID,      // server, asset, signer;
                    strSender,  // owner is either nullptr (for
                                // password-protected
                                // purse) or nymID
                    newPurseUnSelectedTokens,
                    token);  // purse, token
                if (!VerifyStringVal(str)) {
                    otOut << strLocation
                          << ": OT_API_Purse_Push "
                             "newPurseUnSelectedTokens returned null... "
                             "SHOULD NEVER HAPPEN. Returning false.\n";
                    return false;
                }

                newPurseUnSelectedTokens = str;
            }
        }  // for

        if (!bPWProtectOldPurse)  // If old purse is NOT password-protected
                                  // (that is, it's encrypted to a Nym.)
        {
            if (!api_.Exec().SavePurse(
                    notaryID,
                    instrumentDefinitionID,
                    nymID,
                    newPurseUnSelectedTokens))  // if FAILURE.
            {
                // No error message if saving fails??
                // No modal?
                //
                // FT: adding log.
                otOut << strLocation
                      << ":  OT_API_SavePurse "
                         "FAILED. SHOULD NEVER HAPPEN!!!!!!\n";
                return false;
            }
        } else  // old purse IS password protected. (So return its updated
                // version.)
        {
            oldPurse = newPurseUnSelectedTokens;
            // We never cared about this with Nym-owned old purse, since it
            // saves to storage anyway, in the above block. But now in the case
            // of password-protected purses, we set the oldPurse to contain the
            // new version of itself (containing the tokens that had been left
            // unselected) so the caller can do what he wills with it.;
        }

        // The SELECTED tokens (with Recipient as owner of purse AND tokens
        // within) are returned as the "newPurse".
        // The SELECTED tokens (with Sender as owner of purse AND tokens within)
        // are returned as "newPurseForSender".
        //
        newPurse = newPurseSelectedTokens;
        newPurseForSender = newPurseSelectedForSender;
    }

    return true;
}

bool Cash::get_tokens(
    std::vector<std::string>& tokens,
    const std::string& server,
    const std::string& mynym,
    const std::string& assetType,
    std::string purse,
    const std::string& indices) const
{
    if (indices.empty()) { return true; }

    std::int32_t items = api_.Exec().Purse_Count(server, assetType, purse);
    if (0 > items) {
        otOut << "Error: cannot load purse item count.\n";
        return false;
    }

    if (1 > items) {
        otOut << "Error: The purse is empty.\n";
        return false;
    }

    const bool all = (0 == indices.compare("all"));
    for (std::int32_t i = 0; i < items; i++) {
        std::string token =
            api_.Exec().Purse_Peek(server, assetType, mynym, purse);
        if (token.empty()) {
            otOut << "Error: cannot load token from purse.\n";
            return false;
        }

        purse = api_.Exec().Purse_Pop(server, assetType, mynym, purse);
        if (purse.empty()) {
            otOut << "Error: cannot load updated purse.\n";
            return false;
        }

        std::string tokenID = api_.Exec().Token_GetID(server, assetType, token);
        if (tokenID.empty()) {
            otOut << "Error: cannot get token ID.\n";
            return false;
        }

        if (!all &&
            api_.Exec().NumList_VerifyQuery(indices, std::to_string(i))) {
            tokens.push_back(tokenID);
        }
    }

    return true;
}

#endif
}  // namespace opentxs::api::client::implementation

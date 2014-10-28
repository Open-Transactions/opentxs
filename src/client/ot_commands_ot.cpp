// NEW RETURN CODE CONVENTION!

// OT API already returns: -1 for error, 0 for "didn't need to do anything" and
// 1 for success (or larger.)
// All of the below functions follow that convention (they just return the same
// values.)
//
// But for the OT command line tool, we need to return 0 for success, and
// non-zero codes for errors.
// This is because it's a UNIX convention and will be expected by anyone writing
// bash scripts.
//
// However, I don't want to physically change all the below functions, so I'm
// going to change the
// opentxs script itself, so that it translates the return value. That way I
// only have to change it
// in one spot, and we have access to use these scripts both ways.
//
// THE POINT? If you notice these scripts returning 1, which is then interpreted
// as 0 by bash scripts,
// it's because the script that CALLS these scripts, is doing that translation
// (for the above described
// reasons.)
//

#include <opentxs/client/ot_commands_ot.hpp>
#include "ot_utility_ot.hpp"
#include <opentxs/client/ot_otapi_ot.hpp>
#include "ot_made_easy_ot.hpp"
#include <opentxs/client/OTAPI.hpp>
#include <opentxs/client/OT_ME.hpp>
#include <opentxs/core/OTLog.hpp>

namespace opentxs
{
using namespace std;

OT_COMMANDS_OT int32_t OT_Command::mainRevokeCredential()
{
    otOut << "\nThis command not coded yet. We need to use the "
             "OT_API_RevokeSubcredential API call here.\n\n";

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainNewCredential()
{
    otOut << "\nThis command not coded yet. A Nym is created with credentials "
             "already,\nbut to add MORE credentials to an existing Nym, I need "
             "to use the OT_API_AddSubcredential API call.\n\n";

    return -1;
}

// Also TODO:
// Update the "cancel" command, for outgoing cash, to give you a choice to
// deposit the cash instead
// of discarding it.
// details_deposit_purse(strServerID, strMyAcctID, strToNymID, strInstrument,
// "") // strIndices is left blank in this case

// BASKETS

OT_COMMANDS_OT int32_t OT_Command::mainShowBasket()
{
    otOut << "Usage:   showbasket\nOPTIONAL:   --args \"index "
             "BASKET_INDEX\"\n\nNOTE: If you leave off the index, then it "
             "lists all the basket currencies.\nBut if an index is provided, "
             "this command will zoom in and show the details\nfor that "
             "specific basket currency.\n\n";

    return OT_Command::details_show_basket();
}

OT_COMMANDS_OT int32_t OT_Command::details_show_basket()
{
    int32_t nAssetCount = OTAPI_Wrap::GetAssetTypeCount();

    if (nAssetCount > 0) {
        string strIndex = "";

        if (VerifyExists("Args", false)) {
            string strTempIndex = OT_CLI_GetValueByKey(Args, "index");
            if (VerifyStringVal(strTempIndex)) {
                strIndex = strTempIndex;
            }
        }

        // Zoom in on a specific basket currency.
        if (VerifyStringVal(strIndex)) {
            int32_t nIndex = std::stol(strIndex);

            if (nIndex < 0 || nIndex >= nAssetCount) {
                otOut << "Error: index out of bounds.\n";
                return -1;
            }
            else {
                string strAssetID = OTAPI_Wrap::GetAssetType_ID(nIndex);
                if (!VerifyStringVal(strAssetID)) {
                    otOut << "ERROR: Failed getting asset type ID at index: "
                          << nIndex << "\n";
                }
                else {
                    string strAssetName =
                        OTAPI_Wrap::GetAssetType_Name(strAssetID);
                    if (!VerifyStringVal(strAssetName)) {
                        strAssetName = "";
                    }

                    if (!OTAPI_Wrap::IsBasketCurrency(strAssetID)) {
                        otOut
                            << "Failure: not a basket currency: " << strAssetID
                            << " : " << strAssetName << "\n";
                    }
                    else {
                        // display all the details about this basket currency
                        cout << "Name: " << strAssetName << "\n";
                        cout << "ID:   " << strAssetID << "\n";

                        int32_t nMemberCount =
                            OTAPI_Wrap::Basket_GetMemberCount(strAssetID);

                        if (0 > nMemberCount) {
                            otOut << "ERROR: expected int32_t return value "
                                     "from "
                                     "OT_API_Basket_GetMemberCount(strAssetID)"
                                     "\n";
                        }
                        else if (nMemberCount <= 0) {
                            otOut << "Strange: this basket has " << nMemberCount
                                  << " sub-currencies. (Expected 1 or more.)\n";
                        }
                        else {
                            int64_t nBasketMinTransAmt =
                                OTAPI_Wrap::Basket_GetMinimumTransferAmount(
                                    strAssetID);

                            if (0 > nBasketMinTransAmt) {
                                otOut << "Strange: expected minimum transfer "
                                         "amount for basket, but got bad value "
                                         "instead.\n";
                            }
                            else {
                                cout << "Minimum transfer amount for basket:   "
                                        "  " << nBasketMinTransAmt << "\n";
                                cout << "Number of sub-currencies in the "
                                        "basket: " << nMemberCount << "\n";

                                bool bFirstMember = true;

                                for (int32_t nMemberIndex = 0;
                                     nMemberIndex < nMemberCount;
                                     ++nMemberIndex) {
                                    string strMemberType =
                                        OTAPI_Wrap::Basket_GetMemberType(
                                            strAssetID, nMemberIndex);
                                    string strMemberName =
                                        VerifyStringVal(strMemberType)
                                            ? OTAPI_Wrap::GetAssetType_Name(
                                                  strMemberType)
                                            : "";

                                    int64_t nMinTransAmt = OTAPI_Wrap::
                                        Basket_GetMemberMinimumTransferAmount(
                                            strAssetID, nMemberIndex);

                                    if (bFirstMember) {
                                        bFirstMember = false;

                                        otOut << "    Index  :  Min Transfer "
                                                 "Amount  :  Member currency  "
                                                 "\n---------------------------"
                                                 "----"
                                                 "--------\n";
                                    }
                                    cout << "    " << nMemberIndex << "      : "
                                         << nMinTransAmt << " : "
                                         << strMemberType << " : "
                                         << strMemberName << "\n";
                                    otOut << "---------------------------------"
                                             "----"
                                             "--\n";
                                }
                            }
                        }
                    }
                }
            }
            return 1;
        }

        bool bOnFirstIteration = true;
        // List ALL the basket currencies.
        for (int32_t nAssetIndex = 0; nAssetIndex < nAssetCount;
             ++nAssetIndex) {
            string strAssetID = OTAPI_Wrap::GetAssetType_ID(nAssetIndex);
            if (VerifyStringVal(strAssetID) &&
                OTAPI_Wrap::IsBasketCurrency(strAssetID)) {
                if (bOnFirstIteration) {
                    bOnFirstIteration = false;
                    otOut << "Index |  Basket "
                             "currencies:\n------------------------------------"
                             "\n";
                }

                string strAssetName = OTAPI_Wrap::GetAssetType_Name(strAssetID);

                if (VerifyStringVal(strAssetName)) {
                    cout << nAssetIndex << ": " << strAssetID
                         << " : " + strAssetName << "\n";
                }
                else {
                    cout << nAssetIndex << ": " << strAssetID << "\n";
                }
            }
        }
        if (nAssetCount > 0) {
            otOut << "\n";
        }
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainNewBasket()
{
    if (VerifyExists("Server") && VerifyExists("MyNym")) {
        return OT_Command::details_new_basket(Server, MyNym);
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::details_new_basket(const string& strServer,
                                                      const string& strNym)
{
    int32_t nBasketCount = 2;
    otOut << "How many different asset types will compose this new basket "
             "currency? [2]: ";

    string strBasketCount = OT_CLI_ReadLine();

    if (VerifyStringVal(strBasketCount)) {
        nBasketCount = std::stol(strBasketCount);

        if (nBasketCount < 2) {
            otOut << "Sorry, but a basket currency must be composed of at "
                     "least 2 sub-currencies.\n";
            return -1;
        }
    }

    int64_t lMinimumTransAmount = int64_t(100);

    otOut << "\nIf your basket has a minimum transfer amount of 100, you might "
             "have 2 or 3 sub-currencies,\nwith the first being a minimum of 2 "
             "gold, the second being a minimum of 50 dollars, and the\nthird "
             "being a minimum of 30 silver.\nIn this example, 100 units of the "
             "basket currency is transferrable in or out of the\nbasket "
             "currency, in return for 2 gold, 50 dollars, and 30 silver.\n\n";
    otOut << "What is the minimum transfer amount for the basket currency "
             "itself? [100]: ";

    string strMinAmount = OT_CLI_ReadLine();

    if (VerifyStringVal(strMinAmount)) {
        lMinimumTransAmount = std::stoll(strMinAmount);

        if (lMinimumTransAmount < 1) {
            otOut << "Sorry, but this needs to be a non-zero value. Minimum is "
                     "1.\n";
            return -1;
        }
    }

    string strBasket =
        OTAPI_Wrap::GenerateBasketCreation(strNym, lMinimumTransAmount);

    if (!VerifyStringVal(strBasket)) {
        otOut << "Error while generating initial basket object.\n";
        return -1;
    }

    for (int32_t ibasket = 0; ibasket < nBasketCount; ++ibasket) {
        OT_Command::mainShowAssets();

        otOut << "\nThis basket currency has " << nBasketCount
              << " subcurrencies.\n";
        otOut << "So far you have defined " << ibasket << " of them.\n";
        otOut << "Please PASTE the asset type ID for a subcurrency of this "
                 "basket: ";

        string strSubcurrencyID = OT_CLI_ReadLine();

        if (!VerifyStringVal(strSubcurrencyID)) {
            return -1;
        }

        string strSubcurrencyContract =
            OTAPI_Wrap::GetAssetType_Contract(strSubcurrencyID);

        if (!VerifyStringVal(strSubcurrencyContract)) {
            otOut << "Sorry, but " << strSubcurrencyID
                  << " is apparently not a currency contract in your wallet.\n";
            ibasket -= 1;
            continue;
        }

        otOut << "Enter minimum transfer amount for that asset type [100]: ";

        lMinimumTransAmount = 100;
        strMinAmount = OT_CLI_ReadLine();

        if (VerifyStringVal(strMinAmount)) {
            lMinimumTransAmount =
                OTAPI_Wrap::StringToAmount(strSubcurrencyID, strMinAmount);

            if (lMinimumTransAmount < 1) {
                otOut << "Sorry, but this needs to be a non-zero value. "
                         "Minimum is 1.\n";
                ibasket -= 1;
                continue;
            }
        }

        string strTempBasket = OTAPI_Wrap::AddBasketCreationItem(
            strNym, strBasket, strSubcurrencyID, lMinimumTransAmount);

        if (!VerifyStringVal(strTempBasket)) {
            otOut << "Error: OT_API_AddBasketCreationItem returned nullptr. "
                     "(Failure.)\n";
            return -1;
        }
        else {
            strBasket = strTempBasket;
        }
    }

    otOut << "Here's the basket we're issuing:\n\n" << strBasket << "\n";

    string strResponse =
        MadeEasy::issue_basket_currency(strServer, strNym, strBasket);
    int32_t nStatus = VerifyMessageSuccess(strResponse);
    switch (nStatus) {
    case 1: {
        otOut << "\n\n SUCCESS in issue_basket_currency! Server response:\n\n";
        cout << strResponse << "\n";

        string strNewID = OTAPI_Wrap::Message_GetNewAssetTypeID(strResponse);
        bool bGotNewID = VerifyStringVal(strNewID);
        bool bRetrieved = false;
        string strEnding = ".";

        if (bGotNewID) {
            string strRetrieved =
                MadeEasy::retrieve_contract(strServer, strNym, strNewID);
            strEnding = ": " + strNewID;

            if (1 == VerifyMessageSuccess(strRetrieved)) {
                bRetrieved = true;
            }
        }
        otOut << "Server response: SUCCESS in issue_basket_currency!\n";
        otOut << (bRetrieved ? "Success" : "Failed")
              << " retrieving new basket contract" << strEnding << "\n";
        break;
    }
    case 0:
        otOut << "\n\n FAILURE in issue_basket_currency! Server response:\n\n";
        cout << strResponse << "\n";
        otOut << " FAILURE in issue_basket_currency!\n";
        break;
    default:
        otOut << "\n\nError in issue_basket_currency! nStatus is: " << nStatus
              << "\n";

        if (VerifyStringVal(strResponse)) {
            otOut << "Server response:\n\n";
            cout << strResponse << "\n";
            otOut << "\nError in issue_basket_currency! nStatus is: " << nStatus
                  << "\n";
        }
        break;
    }
    otOut << "\n";

    return (0 == nStatus) ? -1 : nStatus;
}

OT_COMMANDS_OT int32_t
    OT_Command::details_exchange_basket(const string& strServer,
                                        const string& strNym,
                                        const string& strAcct,
                                        const string& strBasketType)
{
    // NOTE: details_exchange_basket ASSUMES that strAcct has a server of
    // strServer and
    // a NymID of strNym and an asset type of strBasketType. (These are already
    // verified
    // in OT_Command::mainExchangeBasket.)
    int32_t nMemberCount = OTAPI_Wrap::Basket_GetMemberCount(strBasketType);

    if (nMemberCount < 2) {
        otOut << "Strange, the chosen basket asset type apparently has no "
                 "sub-currencies. (Failure.)\n";
        return -1;
    }

    int64_t lMinimumTransAmount =
        OTAPI_Wrap::Basket_GetMinimumTransferAmount(strBasketType);

    if (0 > lMinimumTransAmount) {
        otOut << "Strange, the chosen basket asset type apparently has no "
                 "minimum transfer amount. (Failure.)\n";
        return -1;
    }

    bool bExchangingIn = true;

    otOut << "Are you exchanging IN or OUT of the basket? [IN]: ";
    string strInOrOut = OT_CLI_ReadLine();

    if (!VerifyStringVal(strInOrOut)) {
        strInOrOut = "in";
    }

    int64_t lBalance = OTAPI_Wrap::GetAccountWallet_Balance(strAcct);
    if (0 > lBalance) {
        otOut << "Strange: unable to retrieve balance for basket account: "
              << strAcct << "\n";
        return -1;
    }

    if ((strInOrOut == "out") || (strInOrOut == "OUT")) {
        otOut << "Exchanging OUT of the basket currency...\n\n";
        bExchangingIn = false;

        if (lBalance < lMinimumTransAmount) {
            otOut << "Sorry, but the minimum transfer amount for this basket "
                     "currency is " << lMinimumTransAmount
                  << "\nand the chosen account's balance is only " << lBalance
                  << ". (Failure.)\n";
            return -1;
        }
    }
    else {
        otOut << "Exchanging IN to the basket currency...\n\n";
    }

    int32_t nTransferMultiple = 1;

    otOut << "This basket currency has a minimum transfer amount of "
          << lMinimumTransAmount << ".\n";
    otOut << "Now you must choose the Transfer Multiple for the exchange. For "
             "example:\n";

    for (int32_t i = 1; i < 5; ++i) {
        int64_t lUnits = i * lMinimumTransAmount;

        otOut << "A transfer multiple of " << i << " will exchange " << lUnits
              << " units of the basket currency.\n";
    }
    otOut << "Etc.\n";

    if (!bExchangingIn) {
        otOut << "Keep in mind, the balance in the basket account is: "
              << lBalance << "\n\n";
    }

    otOut << "Please choose a transfer multiple [1]: ";
    string strTransferMultiple = OT_CLI_ReadLine();

    if (VerifyStringVal(strTransferMultiple)) {
        int32_t nTempMultiple = std::stol(strTransferMultiple);

        if (nTempMultiple > 1) {
            nTransferMultiple = nTempMultiple;
        }
    }

    int64_t lAttemptedTransfer = (nTransferMultiple * lMinimumTransAmount);

    if (!bExchangingIn && (lAttemptedTransfer > lBalance)) {
        otOut << "A transfer multiple of " << nTransferMultiple
              << " will exchange " << lAttemptedTransfer
              << " units out of the basket currency.\nUnfortunately, you only "
                 "have a " << lBalance
              << " balance in your basket account. (Failed.)\n";
        return -1;
    }

    if (!MadeEasy::insure_enough_nums(20, strServer, strNym)) {
        return -1;
    }

    string strBasket = OTAPI_Wrap::GenerateBasketExchange(
        strServer, strNym, strBasketType, strAcct, nTransferMultiple);

    if (!VerifyStringVal(strBasket)) {
        otOut << "Failed generating basket exchange request.\n";
        return -1;
    }

    // Below this point, after any failure, call this before returning:
    //
    // OTAPI_Wrap::Msg_HarvestTransactionNumbers(strBasket, strNym, false,
    // false, false, false, false)
    //
    // NOTE: Only do this up until the message is sent. Once it's sent, the
    // harvesting is already
    // handled internally. (Only need to harvest in the event that some failure
    // occurs in the middle of
    // constructing the basket exchange request, for any time BEFORE actually
    // trying to send it.)

    // SUB-CURRENCIES!

    for (int32_t nMember = 0; nMember < nMemberCount; ++nMember) {
        string strMemberType =
            OTAPI_Wrap::Basket_GetMemberType(strBasketType, nMember);

        if (!VerifyStringVal(strMemberType)) {
            otOut << "Error retrieving member type from index " << nMember
                  << " of basket currency: " << strBasketType << "\n";
            OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                strBasket, strNym, false, false, false, false, false);
            return -1;
        }

        string strMemberTypeName = OTAPI_Wrap::GetAssetType_Name(strMemberType);

        if (!VerifyStringVal(strMemberTypeName)) {
            strMemberTypeName = "";
        }

        int64_t lMemberAmount =
            OTAPI_Wrap::Basket_GetMemberMinimumTransferAmount(strBasketType,
                                                              nMember);

        if (0 > lMemberAmount) {
            otOut << "Error retrieving minimum transfer amount from index "
                  << nMember << " (" << strMemberType
                  << ") on basket currency: " << strBasketType << "\n";
            OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                strBasket, strNym, false, false, false, false, false);
            return -1;
        }

        lAttemptedTransfer = (nTransferMultiple * lMemberAmount);

        // This will only display accounts registered on strServer of asset type
        // strMemberType.
        OT_Command::stat_basket_accounts(strServer, strNym, true,
                                         strMemberType);

        otOut << "There are " << (nMemberCount - nMember)
              << " accounts remaining to be selected.\n\n";
        otOut << "Currently we need to select an account with the asset type:\n"
              << strMemberType << " (" << strMemberTypeName << ")\n";
        otOut << "Above are all the accounts in the wallet, for the relevant "
                 "server and nym, of that asset type.\n";

        if (bExchangingIn) {
            otOut << "\nKeep in mind, with a transfer multiple of "
                  << nTransferMultiple << " and a minimum transfer amount of "
                  << lMemberAmount
                  << "\n(for this sub-currency), you must therefore select an "
                     "account with a minimum\nbalance of: "
                  << lAttemptedTransfer << "\n";
        }

        otOut << "\nPlease PASTE an account ID from the above list: ";
        string strSubAccount = OT_CLI_ReadLine();

        if (!VerifyStringVal(strSubAccount)) {
            OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                strBasket, strNym, false, false, false, false, false);
            return -1;
        }

        string strSubAssetID =
            OTAPI_Wrap::GetAccountWallet_AssetTypeID(strSubAccount);
        if (!VerifyStringVal(strSubAssetID)) {
            otOut << "Error retrieving asset type ID from pasted account: "
                  << strSubAccount << "\n";
            OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                strBasket, strNym, false, false, false, false, false);
            return -1;
        }

        if (strMemberType != strSubAssetID) {
            otOut << "Failure: The selected account has the wrong asset type: "
                  << strSubAssetID << "\n";
            OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                strBasket, strNym, false, false, false, false, false);
            return -1;
        }

        lBalance = OTAPI_Wrap::GetAccountWallet_Balance(strSubAccount);
        if (0 > lBalance) {
            otOut << "Strange, error while retrieving balance for sub-account: "
                  << strSubAccount << "\n";
            OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                strBasket, strNym, false, false, false, false, false);
            return -1;
        }

        if (bExchangingIn && (lAttemptedTransfer > lBalance)) {
            otOut << "\nWith a minimum transfer amount of " << lMemberAmount
                  << " for this sub-currency, a transfer multiple of "
                  << nTransferMultiple << " will exchange "
                  << lAttemptedTransfer
                  << " units out of the basket currency.\n\nUnfortunately, you "
                     "only have a " << lBalance
                  << " balance in your selected account. (Failure.)\n";
            OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                strBasket, strNym, false, false, false, false, false);
            return -1;
        }

        string strTempBasket = OTAPI_Wrap::AddBasketExchangeItem(
            strServer, strNym, strBasket, strSubAssetID, strSubAccount);

        if (!VerifyStringVal(strTempBasket)) {
            otOut << "Failed while adding this sub-account to the exchange "
                     "request!\n";
            OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                strBasket, strNym, false, false, false, false, false);
            return -1;
        }

        strBasket = strTempBasket;

        otOut << "\n\n";
    }

    string strResponse = MadeEasy::exchange_basket_currency(
        strServer, strNym, strBasketType, strBasket, strAcct, bExchangingIn);
    string strAttempt = "exchange_basket";

    int32_t nInterpretReply = InterpretTransactionMsgReply(
        strServer, strNym, strAcct, strAttempt, strResponse);

    if (1 == nInterpretReply) {
        bool bRetrieved =
            MadeEasy::retrieve_account(strServer, strNym, strAcct, true);

        otOut << "Server response (" << strAttempt
              << "): SUCCESS exchanging basket!\n";
        otOut << (bRetrieved ? "Success" : "Failed")
              << " retrieving intermediary files for account.\n";
    }

    return nInterpretReply;
}

OT_COMMANDS_OT int32_t OT_Command::mainExchangeBasket()
{
    otOut << "Usage:   exchange --myacct BASKET_ACCT_ID\n\nThis command "
             "exchanges in or out of a basket currency.\nYou must already have "
             "an asset account which has a basket currency as its asset "
             "type.\nYou must also have accounts for all the subcurrencies in "
             "that basket.\n\n";

    if (VerifyExists("MyAcct")) {
        string strAcctType = OTAPI_Wrap::GetAccountWallet_AssetTypeID(MyAcct);
        string strServer = OTAPI_Wrap::GetAccountWallet_ServerID(MyAcct);
        string strNym = OTAPI_Wrap::GetAccountWallet_NymID(MyAcct);

        if (!VerifyStringVal(strAcctType)) {
            otOut << "Error while trying to retrieve asset type for account: "
                  << MyAcct << "\n";
        }
        else if (!VerifyStringVal(strServer)) {
            otOut << "Error while trying to retrieve server ID for account: "
                  << MyAcct << "\n";
        }
        else if (!VerifyStringVal(strNym)) {
            otOut << "Error while trying to retrieve Nym ID for account: "
                  << MyAcct << "\n";
        }
        else if (VerifyExists("MyPurse", false) && (MyPurse != strAcctType)) {
            otOut << "MyAcct has a different asset type than the one specified "
                     "at the command line.\nPlease re-try, and add: --mypurse "
                  << strAcctType << "\n";
        }
        else if (VerifyExists("Server", false) && (Server != strServer)) {
            otOut << "MyAcct has a different server ID than the one specified "
                     "at the command line.\nPlease re-try, and add: --server "
                  << strServer << "\n";
        }
        else if (VerifyExists("MyNym", false) && (MyNym != strNym)) {
            otOut << "MyAcct has a different owner Nym ID than the one "
                     "specified at the command line.\nPlease re-try, and add: "
                     "--mynym " << strNym << "\n";
        }
        else if (!OTAPI_Wrap::IsBasketCurrency(strAcctType)) {
            otOut << "\nMyAcct is NOT a basket currency! Its asset type is: "
                  << strAcctType << "\nHere is a list of the basket currencies "
                                    "in your wallet:\n";

            details_show_basket();

            otOut << "\nHere is a list of the accounts whose asset type IS a "
                     "basket currency:\n";

            stat_basket_accounts("", "", false, "");

            otOut << "\nMyAcct is not a basket currency!\nPlease use --myacct "
                     "to specify an account whose asset type IS a basket "
                     "currency.\n";
        }
        else {
            return details_exchange_basket(strServer, strNym, MyAcct,
                                           strAcctType);
        }
    }
    else {
        otOut << "You must provide an account ID, and that account must have a "
                 "basket\ncurrency for its asset type. Use --myacct and choose "
                 "from these basket accounts:\n";

        stat_basket_accounts("", "", false, "");
    }

    return -1;
}

// Used by exchange_basket for displaying certain types of accounts.
//
// if strBasketType doesn't exist, it will ONLY show accounts that are basket
// currencies.
// if strBasketType exists, and bFilter is TRUE, it will ONLY show accounts of
// that type.
// if strBasketType exists, and bFilter is FALSE, it will only show accounts
// that are NOT of that type.
//
// Also: if strServer exists, the accounts are filtered by that server.
// Also: if strNym exists, the accounts are filtered by that Nym.
//
OT_COMMANDS_OT int32_t
    OT_Command::stat_basket_accounts(const string& strServer,
                                     const string& strNym, bool bFilter,
                                     const string& strBasketType)
{
    cout << "------------------------------------------------------------------"
            "\n";
    cout << " ** ACCOUNTS :\n\n";

    int32_t nAccountCount = OTAPI_Wrap::GetAccountCount();

    for (int32_t i = 0; i < nAccountCount; ++i) {
        string strID = OTAPI_Wrap::GetAccountWallet_ID(i);
        string strAssetID = OTAPI_Wrap::GetAccountWallet_AssetTypeID(strID);
        string strAcctServerID = OTAPI_Wrap::GetAccountWallet_ServerID(strID);
        string strAcctNymID = OTAPI_Wrap::GetAccountWallet_NymID(strID);

        if (!VerifyStringVal(strServer) ||
            (VerifyStringVal(strServer) && (strServer == strAcctServerID))) {
            if (!VerifyStringVal(strNym) ||
                (VerifyStringVal(strNym) && (strNym == strAcctNymID))) {
                if ((!VerifyStringVal(strBasketType) &&
                     OTAPI_Wrap::IsBasketCurrency(strAssetID)) ||
                    (VerifyStringVal(strBasketType) && bFilter &&
                     (strBasketType == strAssetID)) ||
                    (VerifyStringVal(strBasketType) && !bFilter &&
                     (strBasketType != strAssetID))) {
                    if ((i > 0) && (i != (nAccountCount))) {
                        cout << "-------------------------------------\n";
                    }

                    string strStatAcct = MadeEasy::stat_asset_account(strID);
                    bool bSuccess = VerifyStringVal(strStatAcct);
                    if (bSuccess) {
                        cout << strStatAcct << "\n";
                    }
                    else {
                        cout << "Error trying to stat an account: " << strID
                             << "\n";
                    }

                    cout << "\n";
                }
            }
        }
    }
    cout << "------------------------------------------------------------------"
            "\n";

    return 1;
}

// ALSO TODO:  Modified cancelCronItem so you can pass an instrument.
// So even for non-cron items, you should still be able to cancel them.
// (aka cancel the transaction numbers on it.) This is for when you change
// your mind on an outgoing instrument, and you want it cancelled before the
// recipient can process it.
// Outgoing cheque, outgoing payment plan, outgoing smart contract, etc.
//

// AND WHAT ABOUT incoming cheques? I can already discard it, but that leaves
// the sender with it still sitting in his outpayment box, until it expires.
// I'd rather notify him. He should get a failure/rejection notice so that his
// client can also harvest whatever numbers it needs to harvest.

// TODO:  details_cancel_outgoing and details_discard_incoming both need to be
// updated
// so that they involve a server message. This way the other parties can be
// notified of
// the cancellation / discarding.
//

OT_COMMANDS_OT int32_t
    OT_Command::details_discard_incoming(const string& strServer,
                                         const string& strMyNym,
                                         const string& strIndices)
{
    if (!VerifyStringVal(strIndices)) {
        return -1;
    }

    int32_t nSuccess = 1;

    string strInbox = OTAPI_Wrap::LoadPaymentInbox(strServer, strMyNym);

    if (!VerifyStringVal(strInbox)) {
        otOut << "\n\n details_discard_incoming:  OT_API_LoadPaymentInbox "
                 "Failed.\n\n";
        return -1;
    }

    int32_t nInboxCount =
        OTAPI_Wrap::Ledger_GetCount(strServer, strMyNym, strMyNym, strInbox);

    if (0 > nInboxCount) {
        otOut << "details_discard_incoming: Unable to retrieve size of "
                 "payments inbox ledger. (Failure.)\n";
        return -1;
    }

    if (nInboxCount <= 0) return 0;

    int32_t nIndicesCount =
        VerifyStringVal(strIndices) ? OTAPI_Wrap::NumList_Count(strIndices) : 0;

    // Either we loop through all the instruments and accept them all, or
    // we loop through all the instruments and accept the specified indices.
    //
    // (But either way, we loop through all the instruments.)

    // Loop from back to front, so if any are removed,
    // the indices remain accurate subsequently.
    for (int32_t nInboxIndex = (nInboxCount - 1); nInboxIndex >= 0;
         --nInboxIndex) {
        // - If "all" was passed, we process the item.
        // - If indices are specified, but the current index is not on
        //   that list, then continue...
        if (!("all" == strIndices) &&
            ((nIndicesCount > 0) &&
             !OTAPI_Wrap::NumList_VerifyQuery(strIndices,
                                              std::to_string(nInboxIndex)))) {
            continue;
        }

        // If it IS "all" OR, if there are indices and the current index was
        // found in them.
        // removes payment instrument (from payments in or out box)
        bool bRecorded = OTAPI_Wrap::RecordPayment(strServer, strMyNym, true,
                                                   nInboxIndex, false);
        otOut << "\n" << (bRecorded ? "Success" : "Failure")
              << " discarding instrument from payments inbox at index: "
              << nInboxIndex
              << ".\n\n NOTE: Now the sender has to leave it in his "
                 "outbox until it expires or he cancels it on his "
                 "end.\n";
        if (!bRecorded) {
            nSuccess = -1;
        }
    }

    return nSuccess;
}

OT_COMMANDS_OT int32_t OT_Command::mainDiscard()
{
    otOut << "Usage:   discard --mynym NYM_ID --server SERVER_ID --args "
             "\"index|indices INDICES_GO_HERE\"\n\nThis command discards an "
             "incoming instrument from the payments inbox.\n(Usually used for "
             "discarding an invoice, for when you don't want to pay it.)\nYou "
             "can also use 'all' for the index, for it to process ALL "
             "instruments.\n\n";

    if (!VerifyExists("MyNym") || !VerifyExists("Server")) return -1;

    string strIndex = "";
    string strIndices = "";
    string strFinalIndex = "";
    bool bUseStdin = false;

    if (VerifyExists("Args", false)) {
        strIndex = OT_CLI_GetValueByKey(Args, "index");
        strIndices = OT_CLI_GetValueByKey(Args, "indices");

        if (VerifyStringVal(strIndex)) {
            strFinalIndex = strIndex;
        }
        else if (VerifyStringVal(strIndices)) {
            strFinalIndex = strIndices;
        }
        else {
            bUseStdin = true;
        }
    }
    else {
        bUseStdin = true;
    }

    if (bUseStdin) {
        OT_Command::mainInpayments();

        otOut << "\nPlease enter the index (in your payments inbox)\nof "
                 "the instrument you are discarding: ";
        strIndex = OT_CLI_ReadLine();

        if (VerifyStringVal(strIndex)) {
            strFinalIndex = strIndex;
        }
        else {
            return -1;
        }
    }

    return details_discard_incoming(Server, MyNym, strFinalIndex);
}

OT_COMMANDS_OT int32_t
    OT_Command::details_cancel_outgoing(const string& strMyNym,
                                        const string& strMyAcct,
                                        const string& strIndices)
{
    // NOTE: You can't just discard a sent cheque
    // from your outpayment box. Why not?\nJust because you remove your record
    // of the outgoing cheque, doesn't mean you didn't already send it. (The
    // recipient still received it, and still has it, whether you remove it from
    // your outbox or not.) If you really want to cancel the cheque, then you
    // need to do it in such a way that it will fail-as-cancelled when the
    // recipient tries to deposit it. Otherwise, he would get that money whether
    // you kept your own outgoing record or not. Therefore TODO: SOME server
    // message must be performed here, which actually cancels the transaction
    // number itself that appears on the cheque. This is the only way to insure
    // that the cheque can't be used by the recipient (and even this will only
    // work if you beat him to the punch -- otherwise if he deposits it before
    // you cancel it, then it's already too late and he has the money.) THIS is
    // why RecordPayment, regarding outpayments, only works on expired
    // instruments -- because if it's not expired, you don't just want to remove
    // your record of it. You want to cancel the transaction number itself --
    // and that requires server communication.

    int32_t nCount = OTAPI_Wrap::GetNym_OutpaymentsCount(strMyNym);
    if (0 > nCount) {
        otOut << "Error: cannot retrieve outpayments for Nym: " << strMyNym
              << "\n";
        return -1;
    }

    int32_t nIndicesCount = OTAPI_Wrap::NumList_Count(strIndices);
    int32_t nSuccess = 1;

    if (nCount == 0) {
        otOut << "\n(Outpayment box is empty)\n\n";
        return 0;
    }

    for (int32_t nIndex = (nCount - 1); nIndex >= 0; --nIndex) {
        // - If "all" was passed, we process the item.
        // - If indices are specified, but the current index is not on
        //   that list, then continue...
        if (!("all" == strIndices) && nIndicesCount > 0 &&
            !OTAPI_Wrap::NumList_VerifyQuery(strIndices,
                                             std::to_string(nIndex))) {
            continue;
        }

        // If it IS "all," OR, if there are indices and the current index
        // was found in them.
        string strServer =
            OTAPI_Wrap::GetNym_OutpaymentsServerIDByIndex(strMyNym, nIndex);
        if (!VerifyStringVal(strServer)) {
            otOut << "ERROR: Failed retrieving server ID from "
                     "outpayment at index: " << nIndex << "\n";
            continue;
        }

        string strPaymentContents =
            OTAPI_Wrap::GetNym_OutpaymentsContentsByIndex(strMyNym, nIndex);

        if (!VerifyStringVal(strPaymentContents)) continue;

        //                        string strPaymentAssetID   =
        // OTAPI_Wrap::Instrmnt_GetAssetID (strPaymentContents)
        string strPaymentType =
            OTAPI_Wrap::Instrmnt_GetType(strPaymentContents);

        // Done: Put the code here where we message the server
        // to cancel all relevant transaction
        // numbers for the instrument. If it's a cheque, there's
        // only one number. But if it's a smart
        // contract, there could be many numbers. Seems like
        // best thing is to just activate it, but
        // have a "rejected" flag which causes the activation to
        // fail. (That way, all the other parties
        // will be properly notified, which the server already
        // does.) We don't even need to remove it
        // from the outpayment box, because the failure
        // notification from the server will already cause
        // the OTClient to remove it from the outpayment box.
        //
        // Ah-ha! ANY outgoing payment plan or smart contract is
        // necessarily incomplete: it's outgoing
        // because it was sent to the next party so he could
        // sign it, too, and probably activate it.
        // Presumably he has not done so yet (if I am 'beating
        // him to the punch' by cancelling it before
        // he can activate it) and therefore the plan or smart
        // contract still is missing at least one
        // signer, so it is GUARANTEED to fail verification if I
        // try to activate it myself. (Good.)
        //
        // This means I can just take whatever instrument
        // appears outgoing, and try to activate it myself.
        // It will definitely fail activation, and then the
        // failure notice will already be sent from that,
        // to all the parties, and they can harvest back their
        // numbers automatically as necessary.
        //
        // The one problem is, though this works for payment
        // plans and smart contracts, it will not work
        // for cheques. The cheque is made out to someone else,
        // and he is the one normally who needs to
        // deposit it. Plus, I can't deposit a cheque into the
        // same account it's drawn on.
        //
        // UPDATE: Now when a cheque is deposited into the same
        // account it was drawn on, that will be
        // interpreted by the server as a request to CANCEL the
        // cheque.

        if ("SMARTCONTRACT" == strPaymentType) {
            // Just take the smart contract from the outpayment
            // box, and try to
            // activate it. It WILL fail, and then the failure
            // message will be
            // propagated to all the other parties to the
            // contract. (Which will
            // result in its automatic removal from the
            // outpayment box.)
            if (!VerifyStringVal(strMyAcct)) {
                otOut << "You must provide an account owned by "
                         "this Nym, which will be used for the "
                         "cancellation. Try adding --myacct "
                         "ACCT_ID\nNOTE: in the future we "
                         "should just look up one of the "
                         "accounts from the smart contract "
                         "itself, since the current Nym has "
                         "already confirmed the contract. But "
                         "for now I'm just collecting the acct "
                         "ID at the command line, since it's "
                         "faster.\n";
            }
            else {
                string strResponse = MadeEasy::activate_smart_contract(
                    strServer, strMyNym, strMyAcct, "acct_agent_name",
                    strPaymentContents);

                otOut << "Okay I just tried to activate the "
                         "smart contract. (As a way of "
                         "cancelling it.)\nSo while we expect "
                         "this 'activation' to fail, it should "
                         "have the desired effect of "
                         "cancelling the smart contract and "
                         "sending failure notices to all the "
                         "parties.\n";

                if (VerifyStringVal(strResponse)) {
                    otOut << "\n Here's the server reply: \n" << strResponse
                          << "\n";

                    int32_t nTransCancelled =
                        OTAPI_Wrap::Message_IsTransactionCanceled(
                            strServer, strMyNym, strMyAcct, strResponse);

                    if (1 == nTransCancelled) {
                        otOut << "\n Success canceling!\n";
                    }
                    else {
                        otOut << "\n Error canceling!\n";
                        nSuccess = -1;
                    }
                }
                else {
                    otOut << "Strange, tried to cancel, but "
                             "received a nullptr server "
                             "reply.\n";
                }
            }
        }
        else if ("PAYMENT PLAN" == strPaymentType) {
            // Just take the payment plan from the outpayment
            // box, and try to
            // activate it. It WILL fail, and then the failure
            // message will be
            // propagated to the other party to the contract.
            // (Which will
            // result in its automatic removal from the
            // outpayment box.)

            string strResponse = MadeEasy::cancel_payment_plan(
                strServer, strMyNym, strPaymentContents);

            otOut << "Okay I just tried to activate the "
                     "payment plan. (As a way of cancelling "
                     "it.)\nSo while we expect this "
                     "'activation' to fail, it should have the "
                     "desired effect of cancelling the payment "
                     "plan and sending failure notices to all "
                     "the parties.\n";

            if (VerifyStringVal(strResponse)) {
                otOut << "\n Here's the server reply: \n" << strResponse
                      << "\n";

                int32_t nTransCancelled =
                    OTAPI_Wrap::Message_IsTransactionCanceled(
                        strServer, strMyNym, strMyAcct, strResponse);

                if (1 == nTransCancelled) {
                    otOut << "\n Success canceling!\n";
                }
                else {
                    otOut << "\n Error canceling!\n";
                    nSuccess = -1;
                }
            }
            else {
                otOut << "Strange, tried to cancel, but "
                         "received a nullptr server reply.\n";
            }
        }
        else if ("PURSE" == strPaymentType) {
            // This is a tricky one -- why would anyone EVER
            // want to discard outgoing cash?
            // Normally your incentive would be to do the
            // opposite: Keep a copy of all outgoing
            // cash until the copy itself expires (when the cash
            // expires.) This way it's always
            // recoverable in the event of a "worst case"
            // situation.
            //
            // So what do we do in this case? Nevertheless, the
            // user has explicitly just instructed
            // the client to DISCARD OUTGOING CASH.
            //
            // Perhaps we should just ask the user to CONFIRM
            // that he wants to erase the cash,
            // and make SURE that he understands the
            // consequences of that choice.

            // removes payment instrument (from payments in
            // or out box)
            bool bRecorded = OTAPI_Wrap::RecordPayment(strServer, strMyNym,
                                                       false, nIndex, false);
            if (!bRecorded) {
                nSuccess = -1;
            }
            else {
                otOut << "Discarded cash purse:\n\n" << strPaymentContents
                      << "\n";
            }
            otOut << (bRecorded ? "Success" : "Failure")
                  << " discarding cash purse from "
                     "outpayment box at index: " << nIndex << ".\n\n";
        }
        else // CHEQUE VOUCHER INVOICE
        {
            int32_t nDepositCheque = -1;
            bool bIsVoucher = ("VOUCHER" == strPaymentType);

            // Get the nym and account IDs from the cheque
            // itself.
            string strSenderAcctID =
                (bIsVoucher ? OTAPI_Wrap::Instrmnt_GetRemitterAcctID(
                                  strPaymentContents)
                            : OTAPI_Wrap::Instrmnt_GetSenderAcctID(
                                  strPaymentContents));
            string strSenderUserID =
                (bIsVoucher ? OTAPI_Wrap::Instrmnt_GetRemitterUserID(
                                  strPaymentContents)
                            : OTAPI_Wrap::Instrmnt_GetSenderUserID(
                                  strPaymentContents));

            if (!VerifyStringVal(strSenderAcctID)) {
                otOut << "Failure trying to retrieve asset "
                         "account ID from instrument.\n";
            }
            else if (!VerifyStringVal(strSenderUserID)) {
                otOut << "Failure trying to retrieve Sender "
                         "Nym ID from instrument.\n";
            }
            else if (!(strSenderUserID == strMyNym)) {
                otOut << "Failure, very strange: Sender Nym ID "
                         "on the instrument doesn't match the "
                         "Nym ID.\n";
            }
            else {
                nDepositCheque = OT_Command::details_deposit_cheque(
                    strServer, strSenderAcctID, strSenderUserID,
                    strPaymentContents, strPaymentType);

                otOut << "\n" << (1 == nDepositCheque ? "Success" : "Failure")
                      << " canceling cheque of type: " << strPaymentType
                      << "\n";
            }
        }
    }
    return nSuccess;
}

OT_COMMANDS_OT int32_t OT_Command::mainCancel()
{
    otOut << "Usage:   cancel --mynym NYM_ID --args \"index "
             "INDEX_GOES_HERE\"\n\nThis command cancels an outgoing instrument "
             "from the outpayment box.\n(Usually used for cancelling a cheque, "
             "payment plan, or smart contract.)\nThis, of course, will fail on "
             "the server side, if the recipient has already deposited the "
             "cheque.\n\n";

    if (!VerifyExists("MyNym")) return -1;

    string strIndex = "";
    string strIndices = "";
    string strFinalIndex = "";
    bool bUseStdin = false;

    if (VerifyExists("Args", false)) {
        strIndex = OT_CLI_GetValueByKey(Args, "index");
        strIndices = OT_CLI_GetValueByKey(Args, "indices");

        if (VerifyStringVal(strIndex)) {
            strFinalIndex = strIndex;
        }
        else if (VerifyStringVal(strIndices)) {
            strFinalIndex = strIndices;
        }
        else {
            bUseStdin = true;
        }
    }
    else {
        bUseStdin = true;
    }

    if (bUseStdin) {
        int32_t nCount = OTAPI_Wrap::GetNym_OutpaymentsCount(MyNym);

        for (int32_t nPayments = 0; nPayments < nCount; ++nPayments) {
            show_outpayment(MyNym, nPayments, false);
        }

        otOut << "\nPlease enter the index (in your outpayment box)\nof "
                 "the instrument you are cancelling: ";
        strIndex = OT_CLI_ReadLine();

        if (VerifyStringVal(strIndex)) {
            strFinalIndex = strIndex;
        }
        else {
            return -1;
        }
    }

    string strMyAcct = "";
    if (VerifyExists("MyAcct", false)) {
        strMyAcct = MyAcct;
    }

    return details_cancel_outgoing(MyNym, strMyAcct, strFinalIndex);
}

/*
We might also need something like: OT_Command::main_show_cron_items
(Because we can't trigger a clause on a running smart contract,
unless we are able to list the running smart contracts and thus
ascertain the transaction number of the one whose clause we wish
to trigger.)
*/

OT_COMMANDS_OT int32_t OT_Command::details_trigger_clause(
    const string& strServerID, const string& strMyNymID,
    const string& strTransNum, const string& strClause, const string& strParam)
{
    string strResponse = MadeEasy::trigger_clause(
        strServerID, strMyNymID, strTransNum, strClause, strParam);
    int32_t nMessageSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nMessageSuccess) {
        otOut << "For whatever reason, our attempt to trigger the clause has "
                 "failed.\n";
    }
    else {
        otOut << "Success!\n";
    }

    return nMessageSuccess;
}

OT_COMMANDS_OT int32_t OT_Command::mainTriggerClause()
{
    otOut << "Usage:   STRING GOES HERE";

    if (!VerifyExists("Server") || !VerifyExists("MyNym")) return -1;

    // At this point we need to collect some other data:
    // First, we need the transaction number for the smart contract.
    // Second, we need the name of the clause we will be triggering.
    // Third we need (optionally) any string parameter that the user
    // may wish to pass into the clause when it gets triggered.

    string strTransNum = "";
    string strClause = "";
    string strParam = "";

    if (VerifyExists("Args", false)) {
        string strTempTransNum = OT_CLI_GetValueByKey(Args, "id");
        string strTempClause = OT_CLI_GetValueByKey(Args, "clause");
        string strTempParam = OT_CLI_GetValueByKey(Args, "param");

        // TRANSACTION ID
        if (!VerifyStringVal(strTempTransNum)) {
            otOut << "Please enter the transaction ID for the running "
                     "smart contract: ";
            string strInputTransNum = OT_CLI_ReadLine();

            if (VerifyStringVal(strInputTransNum)) {
                strTransNum = strInputTransNum;
            }
        }
        else {
            strTransNum = strTempTransNum;
        }

        // CLAUSE NAME
        if (!VerifyStringVal(strTempClause)) {
            otOut << "Please enter the name of the clause you wish to "
                     "trigger: ";
            string strInputClause = OT_CLI_ReadLine();

            if (VerifyStringVal(strInputClause)) {
                strClause = strInputClause;
            }
        }
        else {
            strClause = strTempClause;
        }

        // OPTIONAL PARAMETER
        if (!VerifyStringVal(strTempParam)) {
            otOut << "You can pass an optional parameter string (to pass "
                     "to the clause itself)\nfollowed by a ~ by itself on "
                     "a blank line. (Just use ~ to leave it blank.)\n\n";
            string strInputParam = OT_CLI_ReadUntilEOF();

            if (VerifyStringVal(strInputParam)) {
                strParam = strInputParam;
            }
        }
        else {
            strParam = strTempParam;
        }
    }

    if (VerifyStringVal(strTransNum) && VerifyStringVal(strClause)) {
        return details_trigger_clause(Server, MyNym, strTransNum, strClause,
                                      strParam);
    }
    return -1;
}

OT_COMMANDS_OT string
    OT_Command::find_revokedID_for_subcred(const string& strMyNymID,
                                           const string& strInputID)
{
    int32_t nCredCount = OTAPI_Wrap::GetNym_RevokedCredCount(strMyNymID);

    if (1 > nCredCount) {
        return "";
    }

    for (int32_t nCurrent = 0; nCurrent < nCredCount; ++nCurrent) {
        string strCredID =
            OTAPI_Wrap::GetNym_RevokedCredID(strMyNymID, nCurrent);

        int32_t nSubCredCount =
            OTAPI_Wrap::GetNym_SubcredentialCount(strMyNymID, strCredID);

        for (int32_t nCurrentSubCred = 0; nCurrentSubCred < nSubCredCount;
             ++nCurrentSubCred) {
            string strSubCredID = OTAPI_Wrap::GetNym_SubCredentialID(
                strMyNymID, strCredID, nCurrentSubCred);

            if (strInputID == strSubCredID) {
                return strCredID;
            }
        }
    }
    return "";
}

OT_COMMANDS_OT string
    OT_Command::find_masterID_for_subcred(const string& strMyNymID,
                                          const string& strInputID)
{
    int32_t nCredCount = OTAPI_Wrap::GetNym_CredentialCount(strMyNymID);

    if (0 >= nCredCount) {
        return "";
    }

    for (int32_t nCurrent = 0; nCurrent < nCredCount; ++nCurrent) {
        string strCredID =
            OTAPI_Wrap::GetNym_CredentialID(strMyNymID, nCurrent);

        int32_t nSubCredCount =
            OTAPI_Wrap::GetNym_SubcredentialCount(strMyNymID, strCredID);

        for (int32_t nCurrentSubCred = 0; nCurrentSubCred < nSubCredCount;
             ++nCurrentSubCred) {
            string strSubCredID = OTAPI_Wrap::GetNym_SubCredentialID(
                strMyNymID, strCredID, nCurrentSubCred);

            if (strInputID == strSubCredID) {
                return strCredID;
            }
        }
    }
    return "";
}

// Takes a Nym ID and a credential ID and displays the contents
// for the credential.
// The credential might be a master credential, or a subcredential.
// It also might be a revoked credential.
// Handle all cases.
//
OT_COMMANDS_OT bool OT_Command::details_show_credential(
    const string& strMyNymID, const string& strCredID)
{
    string strCredContents =
        OTAPI_Wrap::GetNym_CredentialContents(strMyNymID, strCredID);

    if (VerifyStringVal(strCredContents)) {
        otOut << "Master Credential contents:\n";
        cout << strCredContents << "\n";
        return true;
    }

    strCredContents =
        OTAPI_Wrap::GetNym_RevokedCredContents(strMyNymID, strCredID);

    if (VerifyStringVal(strCredContents)) {
        otOut << "Revoked Credential contents:\n";
        cout << strCredContents << "\n";
        return true;
    }

    // It MUST be a subcredential by this point, either for a master
    // or revoked master credential.
    bool bIsRevoked = false;
    string strMasterID = find_masterID_for_subcred(strMyNymID, strCredID);

    if (!VerifyStringVal(strMasterID)) {
        strMasterID = find_revokedID_for_subcred(strMyNymID, strCredID);
        bIsRevoked = true;
    }

    if (!VerifyStringVal(strMasterID)) {
        otOut << "Sorry, unable to find any credentials associated with that "
                 "ID.\n";
        return false;
    }

    strCredContents = OTAPI_Wrap::GetNym_SubCredentialContents(
        strMyNymID, strMasterID, strCredID);

    if (!VerifyStringVal(strCredContents)) {
        otOut << "Error retrieving sub-credential contents.\n";
        return false;
    }

    if (bIsRevoked) {
        otOut << "Revoked Subcredential contents:\n";
    }
    else {
        otOut << "Subcredential contents:\n";
    }
    cout << strCredContents << "\n";
    otOut << "\n";
    return true;
}

OT_COMMANDS_OT int32_t OT_Command::mainShowCredential()
{
    otOut << "Usage:   showcredential --mynym NYM_ID --args \"id "
             "CREDENTIAL_ID\"\n\nThis command displays the contents of a given "
             "credential (for a given Nym.)\n\n";

    if (!VerifyExists("MyNym")) return -1;

    string strCredID = "";

    if (VerifyExists("Args", false)) {
        string strTempCredID = OT_CLI_GetValueByKey(Args, "id");

        if (!VerifyStringVal(strTempCredID)) {
            otOut << "Please enter the ID for the credential: ";
            string strInput = OT_CLI_ReadLine();

            if (VerifyStringVal(strInput)) {
                strCredID = strInput;
            }
        }
        else {
            strCredID = strTempCredID;
        }
    }

    if (VerifyStringVal(strCredID) &&
        details_show_credential(MyNym, strCredID)) {
        return 1;
    }

    return -1;
}

OT_COMMANDS_OT int32_t
    OT_Command::details_show_credentials(const string& strMyNymID)
{
    int32_t nReturnVal = -1;

    int32_t nCredCount = OTAPI_Wrap::GetNym_CredentialCount(strMyNymID);

    if (0 > nCredCount) {
        return -1;
    }

    if (nCredCount >= 1) {
        nReturnVal = 1;

        otOut << "Idx     Credential ID\n---------------------------\n";

        for (int32_t nCurrent = 0; nCurrent < nCredCount; ++nCurrent) {
            string strCredID =
                OTAPI_Wrap::GetNym_CredentialID(strMyNymID, nCurrent);
            cout << nCurrent << ":      " << strCredID << "\n";

            int32_t nSubCredCount =
                OTAPI_Wrap::GetNym_SubcredentialCount(strMyNymID, strCredID);

            if (nSubCredCount >= 1) {

                otOut << "        ---------------------------\n        Idx     "
                         "Subcredential ID\n        "
                         "---------------------------\n";

                for (int32_t nCurrentSubCred = 0;
                     nCurrentSubCred < nSubCredCount; ++nCurrentSubCred) {
                    string strSubCredID = OTAPI_Wrap::GetNym_SubCredentialID(
                        strMyNymID, strCredID, nCurrentSubCred);
                    cout << "        " << nCurrentSubCred << ":      "
                         << strSubCredID << "\n";
                }
            }
        }

        otOut << "\n";
    }
    else {
        nReturnVal = 0;
    }

    // Next: REVOKED credentials.

    nCredCount = OTAPI_Wrap::GetNym_RevokedCredCount(strMyNymID);

    if (0 > nCredCount) {
        return -1;
    }

    if (nCredCount >= 1) {
        nReturnVal = 1;

        otOut << "Idx     Revoked Credential ID\n---------------------------\n";

        for (int32_t nCurrent = 0; nCurrent < nCredCount; ++nCurrent) {
            string strCredID =
                OTAPI_Wrap::GetNym_RevokedCredID(strMyNymID, nCurrent);
            cout << nCurrent << ":      " << strCredID << "\n";

            int32_t nSubCredCount =
                OTAPI_Wrap::GetNym_SubcredentialCount(strMyNymID, strCredID);

            if (nSubCredCount >= 1) {

                otOut << "        ---------------------------\n        Idx     "
                         "Revoked Subcredential ID\n        "
                         "---------------------------\n";

                for (int32_t nCurrentSubCred = 0;
                     nCurrentSubCred < nSubCredCount; ++nCurrentSubCred) {
                    string strSubCredID = OTAPI_Wrap::GetNym_SubCredentialID(
                        strMyNymID, strCredID, nCurrentSubCred);
                    cout << "        " << nCurrentSubCred << ":      "
                         << strSubCredID << "\n";
                }
            }
        }

        otOut << "\n";
    }

    return nReturnVal;
}

OT_COMMANDS_OT int32_t OT_Command::mainShowCredentials()
{
    otOut << "Usage:   credentials --mynym NYM_ID\n\nThis command displays the "
             "list of credentials for a given Nym.\n\n";

    if (VerifyExists("MyNym")) {
        return details_show_credentials(MyNym);
    }

    return -1;
}

OT_COMMANDS_OT bool OT_Command::stat_partyagent(const string& strSmartContract,
                                                const string& strPartyName,
                                                const string& strAgentName,
                                                int32_t nIndex)
{
    string strAgentID = OTAPI_Wrap::Party_GetAgentID(
        strSmartContract, strPartyName, strAgentName);

    if (VerifyStringVal(strAgentID)) {
        cout << "--------------------\n " << nIndex << " : Agent '"
             << strAgentName << "' (party '" << strPartyName
             << "') has NymID: " << strAgentID << " ('"
             << OTAPI_Wrap::GetNym_Name(strAgentID) << "')\n";
    }
    else {
        cout << " " << nIndex << " : Agent '" << strAgentName << "' (party '"
             << strPartyName << "') has no NymID assigned (yet.)\n";
    }

    return true;
}

OT_COMMANDS_OT bool OT_Command::stat_partyagent_index(
    const string& strSmartContract, const string& strPartyName,
    int32_t nCurrentAgent)
{
    string strAgentName = OTAPI_Wrap::Party_GetAgentNameByIndex(
        strSmartContract, strPartyName, nCurrentAgent);

    if (!VerifyStringVal(strAgentName)) {
        otOut << "Error: Failed retrieving Agent Name from party '"
              << strPartyName << "' at agent index: " << nCurrentAgent << "\n";
        return false;
    }
    return stat_partyagent(strSmartContract, strPartyName, strAgentName,
                           nCurrentAgent);
}

OT_COMMANDS_OT bool OT_Command::stat_partyagents(const string& strSmartContract,
                                                 const string& strPartyName,
                                                 int32_t nDepth)
{
    int32_t nAgentCount =
        OTAPI_Wrap::Party_GetAgentCount(strSmartContract, strPartyName);

    if (nAgentCount < 0) {
        otOut << "Error: Party '" << strPartyName
              << "' has bad value for number of authorized agents.\n";
        return false;
    }

    if (nDepth > 0) {
        for (int32_t nCurrentAgent = 0; nCurrentAgent < nAgentCount;
             ++nCurrentAgent) {
            if (!stat_partyagent_index(strSmartContract, strPartyName,
                                       nCurrentAgent)) {
                return false;
            }
        }
    }
    else {
        cout << "Party '" << strPartyName << "' has " << nAgentCount
             << (1 == nAgentCount ? " agent." : " agents.") << "\n";
    }

    return true;
}

OT_COMMANDS_OT bool OT_Command::stat_partyaccount(
    const string& strSmartContract, const string& strPartyName,
    const string& strAcctName, int32_t nCurrentAccount)
{
    string strAcctAssetID = OTAPI_Wrap::Party_GetAcctAssetID(
        strSmartContract, strPartyName, strAcctName);
    string strAcctID = OTAPI_Wrap::Party_GetAcctID(strSmartContract,
                                                   strPartyName, strAcctName);
    string strAcctAgentName = OTAPI_Wrap::Party_GetAcctAgentName(
        strSmartContract, strPartyName, strAcctName);

    if (VerifyStringVal(strAcctAssetID)) {
        cout << "-------------------\nAccount '" << strAcctName << "' (index "
             << nCurrentAccount << " on Party '" << strPartyName
             << "') has asset type: " << strAcctAssetID << " ("
             << OTAPI_Wrap::GetAssetType_Name(strAcctAssetID) << ")\n";
    }

    if (VerifyStringVal(strAcctID)) {
        cout << "Account '" << strAcctName << "' (party '" << strPartyName
             << "') is confirmed as Account ID: " << strAcctID << " ("
             << OTAPI_Wrap::GetAccountWallet_Name(strAcctID) << ")\n";
    }

    if (VerifyStringVal(strAcctAgentName)) {
        cout << "Account '" << strAcctName << "' (party '" << strPartyName
             << "') is managed by agent: " << strAcctAgentName << "\n";
    }

    return true;
}

OT_COMMANDS_OT bool OT_Command::stat_partyaccount_index(
    const string& strSmartContract, const string& strPartyName,
    int32_t nCurrentAccount)
{
    string strAcctName = OTAPI_Wrap::Party_GetAcctNameByIndex(
        strSmartContract, strPartyName, nCurrentAccount);

    if (!VerifyStringVal(strAcctName)) {
        otOut << "Error: Failed retrieving Asset Account Name from party '"
              << strPartyName << "' at account index: " << nCurrentAccount
              << "\n";
        return false;
    }
    return stat_partyaccount(strSmartContract, strPartyName, strAcctName,
                             nCurrentAccount);
}

OT_COMMANDS_OT bool OT_Command::stat_partyaccounts(
    const string& strSmartContract, const string& strPartyName, int32_t nDepth)
{
    int32_t nAccountCount =
        OTAPI_Wrap::Party_GetAcctCount(strSmartContract, strPartyName);

    if (nAccountCount < 0) {
        otOut << "Error: Party '" << strPartyName
              << "' has bad value for number of asset accounts.\n";
        return false;
    }

    if (nDepth > 0) {
        for (int32_t nCurrentAccount = 0; nCurrentAccount < nAccountCount;
             ++nCurrentAccount) {
            if (!stat_partyaccount_index(strSmartContract, strPartyName,
                                         nCurrentAccount)) {
                return false;
            }
        }
    }
    else {
        cout << "Party '" << strPartyName << "' has " << nAccountCount
             << (1 == nAccountCount ? " asset account." : " asset accounts.")
             << "\n";
    }

    return true;
}

OT_COMMANDS_OT bool OT_Command::show_unconfirmed_parties(
    const string& strSmartContract, int32_t& nPartyCount)
{
    if (0 == nPartyCount) {
        nPartyCount = OTAPI_Wrap::Smart_GetPartyCount(strSmartContract);
        if (nPartyCount < 0) {
            otOut << "Failure: There are apparently no PARTIES to this smart "
                     "contract.\n";
            return false;
        }
    }

    // See if the smart contract is already confirmed by all parties.
    if (OTAPI_Wrap::Smart_AreAllPartiesConfirmed(strSmartContract)) {
        otOut << "Looks like all the parties on this contract are already "
                 "confirmed.\n(Perhaps you want to 'activate' it instead.)\n";
        return false;
    }

    int32_t nUnconfirmedPartyCount = 0;

    for (int32_t nCurrentParty = 0; nCurrentParty < nPartyCount;
         ++nCurrentParty) {
        string strPartyName =
            OTAPI_Wrap::Smart_GetPartyByIndex(strSmartContract, nCurrentParty);

        if (!VerifyStringVal(strPartyName)) {
            otOut << "Error: Unable to retrieve smart contract party's name at "
                     "index: " << nCurrentParty << "\n";
            return false;
        }

        bool bIsPartyConfirmed =
            OTAPI_Wrap::Smart_IsPartyConfirmed(strSmartContract, strPartyName);
        if (!bIsPartyConfirmed) {
            string strIndex = std::to_string(nCurrentParty);
            otOut << strIndex << " : Unconfirmed party: " << strPartyName
                  << "\n";
            ++nUnconfirmedPartyCount;
        }
    }

    if (nUnconfirmedPartyCount <= 0) {
        otOut << "SHOULD NEVER HAPPEN: OT_API_Smart_AreAllPartiesConfirmed was "
                 "already verified as false, above.\nSo then why am I at the "
                 "bottom of this loop and I haven't found any unconfirmed "
                 "parties?\n";
        return false;
    }

    return true;
}

// The merchant (sender of proposal / payee of proposal) proposes the payment
// plan
// to the customer (recipient of proposal / payer of proposal.) So this function
// is
// called by the merchant.
//
OT_COMMANDS_OT int32_t OT_Command::details_propose_plan(
    const string& strServerID, const string& strMyNymID,
    const string& strMyAcctID, const string& strHisNymID,
    const string& strHisAcctID, const string& strDates,
    const string& strConsideration, const string& strInitialPayment,
    const string& strPaymentPlan, const string& strExpiry)
{
    if (!VerifyStringVal(strConsideration)) {
        otOut << "Sorry, but you must describe the consideration to create a "
                 "recurring payment plan.\n";
        return -1;
    }

    // Make sure strMyNymID aka recipient of money (aka sender of payment plan
    // instrument)
    // has enough transaction numbers to propose the plan in the first place.
    //
    if (!MadeEasy::insure_enough_nums(2, strServerID, strMyNymID)) {
        return -1;
    }

    // OTAPI_Wrap::EasyProposePlan is a version of ProposePaymentPlan that
    // compresses it into a
    // fewer number of arguments. (Then it expands them and calls
    // ProposePaymentPlan.)
    // Basically this version has ALL the same parameters, but it stuffs two or
    // three at a time into
    // a single parameter, as a comma-separated list in string form.
    // See details for each parameter, in the comment below.
    string strPlan = OTAPI_Wrap::EasyProposePlan(
        strServerID, strDates, strHisAcctID, strHisNymID, strConsideration,
        strMyAcctID, strMyNymID, strInitialPayment, strPaymentPlan, strExpiry);

    if (!VerifyStringVal(strPlan)) return -1;

    // The "propose" step (performed by the merchant) includes the
    // merchant's confirmation / signing.
    // (So there's no need for the merchant to additionally "confirm" the
    // plan before sending it -- he
    // already has done that by this point, just as part of the proposal
    // itself.)

    // Send the payment plan to the payer (the customer). If it fails,
    // harvest the transaction numbers
    // back from the payment plan. Return 1 if success.

    string strResponse = MadeEasy::send_user_payment(strServerID, strMyNymID,
                                                     strHisNymID, strPlan);
    int32_t nMessageSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nMessageSuccess) {
        otOut << "For whatever reason, our attempt to send the payment "
                 "plan on to the next user has failed.\n";
        int32_t nHarvested = OTAPI_Wrap::Msg_HarvestTransactionNumbers(
            strPlan, strMyNymID, false, false, false, false, false);
        otOut << "details_propose: OT_API_Msg_HarvestTransactionNumbers: "
              << nHarvested << "\n";
        return -1;
    }
    return nMessageSuccess;
}

// payment plan -- called  by recipient. (Who generates the proposal.)
OT_COMMANDS_OT int32_t OT_Command::mainProposePlan()
{
    otOut << "Usage:   propose   (For a merchant to propose a payment plan to "
             "a customer.)\nMandatory: --server SERVER_ID --mynym PAYEE_NYM_ID "
             "--hisnym PAYER_NYM_ID\nMandatory: --myacct PAYEE_ACCT_ID "
             "--hisacct PAYER_NYM_ID\nAdditional arguments:\n Date Range: "
             "--args \" date_range \\\"from,to\\\" \"\nFROM should be in "
             "seconds from Jan 1970, and TO is added to that number.\nDefault "
             "FROM is the current time, and default TO is 'no expiry.'\nAlso: "
             "--args \" consideration \\\"like a memo\\\" \" \nAlso: --args \" "
             "initial_payment \\\"amount,delay\\\" \" \nDefault 'amount' (0 or "
             "\"\") == no initial payment. Default 'delay' (0 or \"\") is "
             "seconds from creation date.\nAlso: --args \" payment_plan "
             "\\\"amount,delay,period\\\" \" \n'amount' is a recurring "
             "payment. 'delay' and 'period' cause 30 days if you pass 0 or "
             "\"\".\n'delay' and 'period' are both measured in seconds. Period "
             "is time between\neach payment. Delay is time until first "
             "payment.\nAlso: --args \" plan_expiry \\\"length,number\\\" \" "
             "\n'length' is maximum lifetime in seconds. 'number' is maximum "
             "number of payments. 0 or \"\" is unlimited (for both.)\n\n";

    if (!VerifyExists("Server") || !VerifyExists("MyNym") ||
        !VerifyExists("MyAcct") || !VerifyExists("HisNym") ||
        !VerifyExists("HisAcct"))
        return -1;

    string strDates = "";
    string strConsideration = "";
    string strInitialPayment = "";
    string strPaymentPlan = "";
    string strExpiry = "";

    if (VerifyExists("Args", false)) {
        string strTempDates = OT_CLI_GetValueByKey(Args, "date_range");
        string strTempConsideration =
            OT_CLI_GetValueByKey(Args, "consideration");
        string strTempInitialPayment =
            OT_CLI_GetValueByKey(Args, "initial_payment");
        string strTempPaymentPlan = OT_CLI_GetValueByKey(Args, "payment_plan");
        string strTempExpiry = OT_CLI_GetValueByKey(Args, "plan_expiry");

        if (VerifyStringVal(strTempDates)) {
            strDates = strTempDates;
        }
        if (VerifyStringVal(strTempConsideration)) {
            strConsideration = strTempConsideration;
        }
        if (VerifyStringVal(strTempInitialPayment)) {
            strInitialPayment = strTempInitialPayment;
        }
        if (VerifyStringVal(strTempPaymentPlan)) {
            strPaymentPlan = strTempPaymentPlan;
        }
        if (VerifyStringVal(strTempExpiry)) {
            strExpiry = strTempExpiry;
        }
    }
    otOut << "\n";

    if (!VerifyStringVal(strConsideration)) {
        otOut << "\nDescribe the product or service being received in "
                 "return for the\nrecurring payments, followed by a ~ by "
                 "itself on a blank line:\n\n";

        strConsideration = OT_CLI_ReadUntilEOF();

        if (!VerifyStringVal(strConsideration)) {
            return -1;
        }
    }

    otOut << "date_range (from,to): " << strDates << "\n";
    otOut << "consideration: " << strConsideration << "\n";
    otOut << "initial_payment (amount,delay): " << strInitialPayment << "\n";
    otOut << "payment_plan (amount,delay,period): " << strPaymentPlan << "\n";
    otOut << "plan_expiry (length,number): " << strExpiry << "\n";

    return details_propose_plan(Server, MyNym, MyAcct, HisNym, HisAcct,
                                strDates, strConsideration, strInitialPayment,
                                strPaymentPlan, strExpiry);
}

// NOTE: if nIndex is NOT -1, then it's in the payments inbox for Server and
// MyNym, and we
// need to go ahead and
// the serverID must be found on the instrument, or must be provided as Server,
// or we must ask
// the user to enter it.
//
OT_COMMANDS_OT int32_t
    OT_Command::details_confirm_plan(const string& strPlan, const int32_t)
{
    string strLocation = "details_confirm_plan";

    string strServerID = OTAPI_Wrap::Instrmnt_GetServerID(strPlan);
    string strSenderUserID = OTAPI_Wrap::Instrmnt_GetSenderUserID(strPlan);
    string strSenderAcctID = OTAPI_Wrap::Instrmnt_GetSenderAcctID(strPlan);
    string strRecipientUserID =
        OTAPI_Wrap::Instrmnt_GetRecipientUserID(strPlan);

    if (!VerifyStringVal(strServerID) || !VerifyStringVal(strSenderUserID) ||
        !VerifyStringVal(strSenderAcctID) ||
        !VerifyStringVal(strRecipientUserID) || !VerifyStringVal(strPlan))
        return -1;

    // Make sure strSenderUserID aka sender of money (aka recipient of
    // payment plan
    // instrument in his inbox, which if confirmed will have him sending
    // money...)
    // has enough transaction numbers to confirm the plan.
    //
    if (!MadeEasy::insure_enough_nums(2, strServerID, strSenderUserID)) {
        return -1;
    }

    string strConfirmed = OTAPI_Wrap::ConfirmPaymentPlan(
        strServerID, strSenderUserID, strSenderAcctID, strRecipientUserID,
        strPlan);

    if (!VerifyStringVal(strConfirmed)) return -1;

    // Below this point, inside this block, if we fail, then we need to
    // harvest the transaction
    // numbers back from the payment plan that we confirmed
    // (strConfirmed.)
    string strResponse = MadeEasy::deposit_payment_plan(
        strServerID, strSenderUserID, strConfirmed);
    string strAttempt = "deposit_payment_plan";
    int32_t nMessageSuccess = VerifyMessageSuccess(strResponse);

    // These are cases where the transaction never even got to TRY to
    // run, since the message itself
    // failed inside the server before it even got that far.
    if (-1 == nMessageSuccess) {
        // In this case, the server never even SAW the transaction,
        // because its enveloping message already
        // failed before we even got that far. So we can harvest, as
        // though it was never attempted...
        otOut << "Message error: " << strAttempt << ".\n";
        int32_t nHarvested = OTAPI_Wrap::Msg_HarvestTransactionNumbers(
            strConfirmed, strSenderUserID, false, false, false, false, false);
        otOut << strLocation
              << ": OT_API_Msg_HarvestTransactionNumbers: " << nHarvested
              << "\n";
        return -1;
    }
    else if (0 == nMessageSuccess) {
        otOut << "Server reply (" << strAttempt << "): Message failure.\n";
        int32_t nHarvested = OTAPI_Wrap::Msg_HarvestTransactionNumbers(
            strConfirmed, strSenderUserID, false, false, false, false, false);
        otOut << strLocation
              << ": OT_API_Msg_HarvestTransactionNumbers: " << nHarvested
              << "\n";
        return 0;
    }
    // (else 1: message was processed successfully -- though not
    // necessarily the transaction inside it.)

    // BELOW THIS POINT, the transaction has definitely processed. It
    // may have succeeded or failed,
    // but either way, we don't need to harvest any numbers, since OT
    // will internally handle those
    // when it processes the server's reply to the transaction attempt.
    int32_t nInterpretReply = InterpretTransactionMsgReply(
        strServerID, strSenderUserID, strSenderAcctID, strAttempt, strResponse);
    if (1 == nInterpretReply) {
        bool bRetrieved = MadeEasy::retrieve_account(
            strServerID, strSenderUserID, strSenderAcctID, true);

        otOut << "\nServer response (" << strAttempt
              << "): SUCCESS activating payment plan!\n";
        otOut << (bRetrieved ? "Success" : "Failed")
              << " retrieving intermediary files for account.\n";
    }
    return nInterpretReply;
}

// NOTE: if nIndex is -1, then it's assumed the instrument was PASTED in, and
// therefore
// the serverID must be found on the instrument, or must be provided as Server,
// or we must ask
// the user to enter it. Then the Nym [and potentially AcctID] must be
// ascertained by walking
// the user through the instrument, and by making him choose a Nym and Acct(s)
// for the party
// he's confirming as.
//
// But if an index is provided, then it's ASSUMED the index goes to the payments
// inbox, and
// that therefore Server and MyNym must have been provided at the command line
// already, or
// otherwise that index could not logically have been matched up to the proper
// box.
//
// If SOME parties have already confirmed the contract before you, then it
// SHOULD already have
// a serverID attached to it. If a Server is also provided at the command line,
// then the two must
// match, since the ID cannot be changed after that point.
//

OT_COMMANDS_OT int32_t
    OT_Command::details_confirm_smart_contract(string& strSmartContract,
                                               int32_t nIndex)
{
    string strLocation = "details_confirm_smart_contract";

    int32_t nPartyCount = OTAPI_Wrap::Smart_GetPartyCount(strSmartContract);

    if (nPartyCount <= 0) {
        otOut << "Failed trying to confirm a smart contract: there are "
                 "apparently no PARTIES to this contract!\n";
        return -1;
    }

    // We don't need MyAcct except when actually ACTIVATING the smart contract
    // on the server.
    // This variable might get set later to MyAcct, if it matches one of the
    // accounts being
    // confirmed. (Meaning if this variable is set by the time we reach the
    // bottom, then we
    // can use it for activation, if/when needed.)
    string strMyAcctID = "";
    string strMyAcctAgentName = "";

    string strServerID = "";
    string strNymID = "";

    map<string, string> map_ID;
    map<string, string> map_agent;

    // By this point we know it's a smart contract, and there is at least
    // one party that is still unconfirmed on this contract.
    // Therefore let's loop through the parties and display the
    // unconfirmed ones.
    if (show_unconfirmed_parties(strSmartContract, nPartyCount)) {

        // By this point I know that nUnconfirmedPartyCount is larger than 0.
        // Therefore we displayed the unconfirmed ones...
        // Let's ask the user to choose one to confirm as....
        otOut << "\n\nWhich party are you? Enter the number, from the list "
                 "above: ";

        string strPartyIndex = OT_CLI_ReadLine();

        if (VerifyStringVal(strPartyIndex)) {
            int32_t nCurrentParty = std::stol(strPartyIndex);

            if (nCurrentParty >= 0 && nCurrentParty < nPartyCount) {
                string strPartyName = OTAPI_Wrap::Smart_GetPartyByIndex(
                    strSmartContract, nCurrentParty);

                if (!VerifyStringVal(strPartyName)) {
                    otOut << "Error: Unable to retrieve smart contract party's "
                             "name at index: " << nCurrentParty << "\n";
                    return -1;
                }

                if (OTAPI_Wrap::Smart_IsPartyConfirmed(strSmartContract,
                                                       strPartyName)) {
                    otOut << "Failure: the party at that index is _already_ "
                             "confirmed.\n";
                    return -1;
                }

                // At this point we know it's a valid index for an unconfirmed
                // party.
                // And we know that strPartyName is the party's name.
                //
                // So how many accounts does strPartyName have? We must confirm
                // those, too.
                int32_t nAccountCount = OTAPI_Wrap::Party_GetAcctCount(
                    strSmartContract, strPartyName);

                if (nAccountCount < 0) {
                    otOut
                        << "Error: Party '" << strPartyName
                        << "' has a bad value for number of asset accounts.\n";
                    return -1;
                }

                // Confirm the accounts.
                if (nAccountCount > 0) {
                    // In the loop, if we have to devise the serverID, we store
                    // it in this var so we don't devise twice.
                    string strFoundServerID = "";
                    string strFoundMyNymID = "";
                    int32_t nCurrentAcctCount = nAccountCount;

                    while (nCurrentAcctCount > 0) {
                        otOut << "\n";
                        stat_partyaccounts(strSmartContract, strPartyName, 2);

                        otOut << "\n There are " << nCurrentAcctCount
                              << " asset accounts remaining to be "
                                 "confirmed.\nEnter the index for an "
                                 "UNconfirmed account: ";

                        string strAcctIndex = OT_CLI_ReadLine();

                        if (!VerifyStringVal(strAcctIndex)) {
                            return -1;
                        }

                        int32_t nAcctIndex = std::stol(strAcctIndex);

                        if (nAcctIndex < 0) {
                            otOut << "Error: Bad Index: " << strAcctIndex
                                  << "\n";
                            return -1;
                        }

                        string strAcctName =
                            OTAPI_Wrap::Party_GetAcctNameByIndex(
                                strSmartContract, strPartyName, nAcctIndex);

                        if (!VerifyStringVal(strAcctName)) {
                            otOut << "Error: Unable to retrieve account name "
                                     "at index " << strAcctIndex
                                  << " for Party: " << strPartyName << "\n";
                            return -1;
                        }

                        bool bAlreadyThere = false;

                        for (auto x = map_ID.begin(); x != map_ID.end(); ++x) {
                            if (x->first == strAcctName) {
                                bAlreadyThere = true;
                            }
                        }

                        string strAcctID = OTAPI_Wrap::Party_GetAcctID(
                            strSmartContract, strPartyName, strAcctName);

                        if (bAlreadyThere || VerifyStringVal(strAcctID)) {
                            otOut << "\nSorry, but the account at index "
                                  << strAcctIndex
                                  << " is already confirmed with account ID: "
                                  << strAcctID
                                  << " \nYou are just going to have to pick a "
                                     "different account.\n";
                        }
                        else {
                            // The account is NOT already confirmed (so we can
                            // confirm it, once we select the Acct ID to use.)
                            string strServerOnContract =
                                OTAPI_Wrap::Instrmnt_GetServerID(
                                    strSmartContract);
                            bool bServerOnContract =
                                VerifyStringVal(strServerOnContract);

                            // Now we need to ascertain the server and Nym ID...

                            // The instrument was selected from the payments
                            // inbox at a given index.
                            if (-1 != nIndex) {
                                strServerID = Server;
                                strNymID = MyNym;

                                // If there's a server listed on the contract
                                // (there may not be, if this is the FIRST
                                // confirmation) AND if that Server ID doesn't
                                // match the one passed in on the command
                                // line...
                                if (bServerOnContract &&
                                    (strServerID != strServerOnContract)) {
                                    string strServerName =
                                        OTAPI_Wrap::GetServer_Name(
                                            strServerOnContract);
                                    if (!VerifyStringVal(strServerName)) {
                                        strServerName = "";
                                    }
                                    otOut << "The server ID on the smart "
                                             "contract, " << strServerOnContract
                                          << " (" << strServerName
                                          << ") doesn't match the server "
                                             "selected at the command line "
                                             "with the --server option.\n";
                                    return -1;
                                }
                            }
                            else // the instrument was pasted in.
                            {
                                if (bServerOnContract) {
                                    strServerID = strServerOnContract;
                                }
                                else if (VerifyExists("Server", false)) {
                                    strServerID = Server;
                                }
                                else if (VerifyStringVal(strFoundServerID)) {
                                    strServerID = strFoundServerID;
                                }
                                else {
                                    // There's no server on the contract, AND
                                    // there's no server chosen at the
                                    // command line.
                                    int32_t nServerCount =
                                        OTAPI_Wrap::GetServerCount();

                                    if (nServerCount < 1) {
                                        otOut << "Sorry, there aren't any "
                                                 "server contracts in this "
                                                 "wallet. Try 'opentxs "
                                                 "addserver'\n";
                                        return -1;
                                    }

                                    OT_Command::mainShowServers();

                                    otOut << "Paste a server ID: ";

                                    string strServerIDInput = OT_CLI_ReadLine();

                                    if (!VerifyStringVal(strServerIDInput)) {
                                        return -1;
                                    }

                                    strServerID = strServerIDInput;
                                    strFoundServerID = strServerIDInput;
                                }

                                if (VerifyExists("MyNym", false)) {
                                    strNymID = MyNym;
                                }
                                else if (VerifyStringVal(strFoundMyNymID)) {
                                    strNymID = strFoundMyNymID;
                                }
                                else {
                                    int32_t nNymCount =
                                        OTAPI_Wrap::GetNymCount();

                                    if (nNymCount < 1) {
                                        otOut << "There are apparently no Nyms "
                                                 "in this wallet. Try 'opentxs "
                                                 "newnym'\n";
                                        return -1;
                                    }
                                    while (true) {
                                        cout << "------------------------------"
                                                "------------------------------"
                                                "------\n";
                                        cout << " ** NYMS (filtered by "
                                                "serverID: " << strServerID
                                             << "): \n\n";

                                        bool bFoundANym = false;

                                        for (int32_t i = 0; i < nNymCount;
                                             ++i) {
                                            string strIndex = std::to_string(i);
                                            string strLoopNymID =
                                                OTAPI_Wrap::GetNym_ID(i);

                                            if (OTAPI_Wrap::
                                                    IsNym_RegisteredAtServer(
                                                        strLoopNymID,
                                                        strServerID)) {
                                                bFoundANym = true;
                                                string strName =
                                                    OTAPI_Wrap::GetNym_Name(
                                                        strLoopNymID);
                                                otOut << strIndex << ": "
                                                      << strLoopNymID
                                                      << " ---  " << strName
                                                      << "\n";
                                            }
                                        }

                                        if (!bFoundANym) {
                                            otOut
                                                << "Unfortunately, didn't find "
                                                   "ANY Nyms registered at "
                                                   "server: " << strServerID
                                                << "\n";
                                            return -1;
                                        }

                                        otOut << "\n Select a Nym by index: ";

                                        string strNymIndex = OT_CLI_ReadLine();

                                        if (!VerifyStringVal(strNymIndex)) {
                                            return -1;
                                        }

                                        int32_t nNymIndex =
                                            std::stol(strNymIndex);
                                        if (nNymIndex < 0) {
                                            otOut
                                                << "Bad index: " << strNymIndex
                                                << "\n";
                                            return -1;
                                        }

                                        string strTempNymID =
                                            OTAPI_Wrap::GetNym_ID(nNymIndex);

                                        if (!VerifyStringVal(strTempNymID)) {
                                            otOut << "Error retrieving NymID "
                                                     "from wallet at index: "
                                                  << strNymIndex << "\n";
                                            return -1;
                                        }

                                        if (OTAPI_Wrap::
                                                IsNym_RegisteredAtServer(
                                                    strTempNymID,
                                                    strServerID)) {
                                            strNymID = strTempNymID;
                                            strFoundMyNymID = strTempNymID;
                                            break;
                                        }
                                        else {
                                            otOut << "Sorry, that Nym isn't "
                                                     "registered at server: "
                                                  << strServerID
                                                  << "\nPlease choose another "
                                                     "Nym\n";
                                        }
                                    }
                                }
                            }

                            // By this point, we definitely have strServerID and
                            // strNymID

                            // Loop through all the accounts for that
                            // Server/Nym, display them, allow the user to
                            // choose one and then store it in a list somewhere
                            // for the actual confirmation.
                            // (Which we will do only after ALL accounts have
                            // been selected.) We will also make sure
                            // to prevent from selecting an account that already
                            // appears on that list.
                            //
                            // Also, when we actually add an Acct ID to the
                            // list, we need to decrement nCurrentAcctCount,
                            // so that we are finished once all the accounts
                            // from the smart contract template have had
                            // actual acct IDs selected for each.
                            string strTemplateAssetID =
                                OTAPI_Wrap::Party_GetAcctAssetID(
                                    strSmartContract, strPartyName,
                                    strAcctName);
                            bool bFoundTemplateAssetID =
                                VerifyStringVal(strTemplateAssetID);

                            bool bFoundAccounts = false;
                            int32_t nCountAcctsWallet =
                                OTAPI_Wrap::GetAccountCount();

                            otOut << "\nAccounts by index (filtered by "
                                     "serverID and nymID):\n\n";

                            for (int32_t i = 0; i < nCountAcctsWallet; ++i) {
                                string strWalletAcctID =
                                    OTAPI_Wrap::GetAccountWallet_ID(i);
                                if (!VerifyStringVal(strWalletAcctID)) {
                                    otOut << "Error reading account ID based "
                                             "on index: " << i << "\n";
                                    return -1;
                                }

                                string strAcctServerID =
                                    OTAPI_Wrap::GetAccountWallet_ServerID(
                                        strWalletAcctID);
                                string strAcctNymID =
                                    OTAPI_Wrap::GetAccountWallet_NymID(
                                        strWalletAcctID);
                                string strAcctAssetID =
                                    OTAPI_Wrap::GetAccountWallet_AssetTypeID(
                                        strWalletAcctID);

                                bool bAlreadyOnTheMap = false;

                                for (auto x = map_ID.begin(); x != map_ID.end();
                                     ++x) {
                                    if (x->second == strWalletAcctID) {
                                        bAlreadyOnTheMap = true;
                                    }
                                }

                                if ((strServerID == strAcctServerID) &&
                                    (strNymID == strAcctNymID)) {
                                    // If the smart contract doesn't specify the
                                    // asset type ID of the account, or if
                                    // it DOES specify, AND they match, then
                                    // it's a viable choice. Display it.
                                    if (!bFoundTemplateAssetID ||
                                        (bFoundTemplateAssetID &&
                                         (strTemplateAssetID ==
                                          strAcctAssetID))) {
                                        // DO NOT display any accounts that have
                                        // already been selected! (Search the
                                        // temp map where we've been stashing
                                        // them.)
                                        if (!bAlreadyOnTheMap) {
                                            bFoundAccounts = true;
                                            string strAccountName = OTAPI_Wrap::
                                                GetAccountWallet_Name(
                                                    strWalletAcctID);
                                            if (!VerifyStringVal(
                                                    strAccountName)) {
                                                strAccountName = "";
                                            }
                                            otOut << i << " : "
                                                  << strWalletAcctID << " ("
                                                  << strAccountName << ")\n";
                                        }
                                    }
                                }
                            }

                            if (!bFoundAccounts) {
                                otOut << "Unfortunately, looks like there are "
                                         "no accounts matching the specified "
                                         "Nym (" << strNymID << ") and Server ("
                                      << strServerID
                                      << ")\nTry:  opentxs newaccount --mynym "
                                      << strNymID << " --server " << strServerID
                                      << " \n";
                                return -1;
                            }

                            otOut << "\nChoose an account by index (for '"
                                  << strAcctName << "'): ";

                            string strSelectedAcctIndex = OT_CLI_ReadLine();

                            if (!VerifyStringVal(strSelectedAcctIndex)) {
                                return -1;
                            }

                            int32_t nSelectedAcctIndex =
                                std::stol(strSelectedAcctIndex);

                            if (nSelectedAcctIndex < 0 ||
                                nSelectedAcctIndex >= nCountAcctsWallet) {
                                otOut << "Bad index: " << strSelectedAcctIndex
                                      << "\n";
                                return -1;
                            }

                            string strWalletAcctID =
                                OTAPI_Wrap::GetAccountWallet_ID(
                                    nSelectedAcctIndex);
                            if (!VerifyStringVal(strWalletAcctID)) {
                                otOut << "Error reading account ID based on "
                                         "index: " << nSelectedAcctIndex
                                      << "\n";
                                return -1;
                            }

                            string strAcctServerID =
                                OTAPI_Wrap::GetAccountWallet_ServerID(
                                    strWalletAcctID);
                            string strAcctNymID =
                                OTAPI_Wrap::GetAccountWallet_NymID(
                                    strWalletAcctID);
                            string strAcctAssetID =
                                OTAPI_Wrap::GetAccountWallet_AssetTypeID(
                                    strWalletAcctID);

                            if ((strServerID == strAcctServerID) &&
                                (strNymID == strAcctNymID)) {
                                // If the smart contract doesn't specify the
                                // asset type ID of the account, or if
                                // it DOES specify, AND they match, then it's a
                                // viable choice. Allow it.
                                if (!bFoundTemplateAssetID ||
                                    (bFoundTemplateAssetID &&
                                     (strTemplateAssetID == strAcctAssetID))) {

                                    bool bAlreadyOnIt = false;

                                    for (auto x = map_ID.begin();
                                         x != map_ID.end(); ++x) {
                                        if (x->second == strWalletAcctID) {
                                            bAlreadyOnIt = true;
                                        }
                                    }

                                    if (bAlreadyOnIt) {
                                        otOut << "Sorry, you already selected "
                                                 "this account. Choose "
                                                 "another.\n";
                                    }
                                    else {
                                        // strWalletAcctID has been selected for
                                        // strPartyName's account, strAcctName.
                                        // Add these to a map or whatever, to
                                        // save them until this loop is
                                        // complete.

                                        string strAgentName =
                                            OTAPI_Wrap::Party_GetAcctAgentName(
                                                strSmartContract, strPartyName,
                                                strAcctName);

                                        if (!VerifyStringVal(strAgentName)) {
                                            otOut << "\n";

                                            if (stat_partyagents(
                                                    strSmartContract,
                                                    strPartyName, 3)) {
                                                otOut << "\n (This choice is "
                                                         "arbitrary, but you "
                                                         "must pick "
                                                         "one.)\nEnter the "
                                                         "index for an agent, "
                                                         "to have authority "
                                                         "over that account: ";

                                                string strAgentIndex =
                                                    OT_CLI_ReadLine();

                                                if (!VerifyStringVal(
                                                        strAgentIndex)) {
                                                    return -1;
                                                }

                                                int32_t nAgentIndex =
                                                    std::stol(strAgentIndex);

                                                if (nAgentIndex < 0) {
                                                    otOut
                                                        << "Error: Bad Index: "
                                                        << strAgentIndex
                                                        << "\n";
                                                    return -1;
                                                }

                                                strAgentName = OTAPI_Wrap::
                                                    Party_GetAgentNameByIndex(
                                                        strSmartContract,
                                                        strPartyName,
                                                        nAgentIndex);

                                                if (!VerifyStringVal(
                                                        strAgentName)) {
                                                    otOut << "Error: Unable to "
                                                             "retrieve agent "
                                                             "name at index "
                                                          << strAgentIndex
                                                          << " for Party: "
                                                          << strPartyName
                                                          << "\n";
                                                    return -1;
                                                }

                                            }
                                            else {
                                                otOut
                                                    << "Failed finding the "
                                                       "agent's name for "
                                                       "party: " << strPartyName
                                                    << " Account: "
                                                    << strAcctName
                                                    << " \n And then failed "
                                                       "finding any agents on "
                                                       "this smart contract at "
                                                       "ALL.\n";
                                                return -1;
                                            }
                                        }

                                        map_ID[strAcctName] = strWalletAcctID;
                                        map_agent[strAcctName] = strAgentName;

                                        nCurrentAcctCount -= 1;
                                    }
                                }
                            }
                        }
                    }

                    for (auto x = map_ID.begin(); x != map_ID.end(); ++x) {
                        int32_t numCountNeeded =
                            OTAPI_Wrap::SmartContract_CountNumsNeeded(
                                strSmartContract, map_agent[x->first]) +
                            1;

                        if (!MadeEasy::insure_enough_nums(
                                numCountNeeded, strServerID, strNymID)) {
                            otOut << "\n** Sorry -- Nym (" << strNymID
                                  << ") doesn't have enough transaction "
                                     "numbers to confirm this smart contract. "
                                     "Come back when you have more.\n";
                            return -1;
                        }
                    }

                    // CONFIRM THE ACCOUNTS HERE
                    //
                    // By this point we KNOW we have enough transaction numbers
                    //
                    // Note: Any failure below this point needs to harvest back
                    // ALL transaction numbers.
                    // Because we haven't even TRIED to activate it, therefore
                    // ALL numbers on the contract
                    // are still good (even the opening number.)
                    //
                    // Whereas after a failed activation, we'd need to harvest
                    // only the closing numbers, and
                    // not the opening numbers. But in here, this is
                    // confirmation, not activation.
                    for (auto x = map_ID.begin(); x != map_ID.end(); ++x) {
                        // Here we check to see if MyAcct exists -- if so we
                        // compare it to the current acctID
                        // and if they match, we set strMyAcctID. Later on,
                        // if/when activating, we can just use
                        // strMyAcctID to activate. (Otherwise we will have to
                        // pick one from the confirmed accounts.)
                        if (!VerifyStringVal(strMyAcctID) &&
                            VerifyExists("MyAcct", false) &&
                            (MyAcct == x->second)) {
                            strMyAcctID = MyAcct;
                            strMyAcctAgentName = map_agent[x->first];
                        }

                        // CONFIRM A THEORETICAL ACCT BY GIVING IT A REAL ACCT
                        // ID.
                        string strTempSmart =
                            OTAPI_Wrap::SmartContract_ConfirmAccount(
                                strSmartContract, strNymID, strPartyName,
                                x->first, map_agent[x->first], x->second);
                        if (VerifyStringVal(strTempSmart)) {
                            strSmartContract = strTempSmart;
                        }
                        else {
                            otOut << "Failure while calling "
                                     "OT_API_SmartContract_ConfirmAccount. "
                                     "Acct Name: " << x->first
                                  << "  Agent Name: " << map_agent[x->first]
                                  << "  Acct ID: " << x->second << " \n";
                            int32_t nHarvested =
                                OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                                    strSmartContract, strNymID, false, false,
                                    false, false, false);
                            otOut << strLocation
                                  << ": OT_API_Msg_HarvestTransactionNumbers: "
                                  << nHarvested << "\n";
                            return -1;
                        }
                    }

                } // if (nCurrentAcctCount > 0)

                // CONFIRM THE NYM HERE.
                //
                // NOTE: confirming the Party will drop a copy into the
                // outpayments box.
                // After that, if you are not the last party, you will send it
                // on to
                // the next party. This will also drop it into the outpayments
                // box, but
                // when that happens, it will automatically first remove the
                // prior
                // one that had been dropped. This way there is only one copy in
                // the outbox, not two.
                //
                // If you ARE the last party, then we will activate it here, and
                // when the server reply is received, it will be removed from
                // the
                // outbox and moved to the record box.
                //
                // Also, whether it succeeds or fails activation, either way, a
                // notice
                // will be dropped to all the parties, informing them of this
                // fact.
                // The activating party will ignore this, since he already
                // processes
                // the server reply directly when he receives it. (And there's a
                // copy of
                // that in his nymbox to make SURE he receives it, so he
                // DEFINITELY
                // received it already.)
                // But each of the other parties will then move the notice from
                // their
                // outbox to their record box. (And harvest the transaction
                // numbers
                // accordingly.)
                // A party can "reject" a smart contract by activating it
                // without signing
                // it. That way it will fail activation, and all the other
                // parties will get
                // the failure notice, and harvest their numbers back as
                // appropriate. But if
                // he is "rude" and simply discards the contract WITHOUT
                // rejecting it, then
                // they will never get the failure notice. However, they will
                // also never
                // get the activation notice, since it was never activated in
                // that case, and
                // thus they will be safe to harvest their numbers back when it
                // expires.
                // (A well-designed wallet will do this automatically.)

                string strTempSmart = OTAPI_Wrap::SmartContract_ConfirmParty(
                    strSmartContract, strPartyName, strNymID);

                if (VerifyStringVal(strTempSmart)) {
                    strSmartContract = strTempSmart;
                }
                else {
                    otOut << "Failure while calling "
                             "OT_API_SmartContract_ConfirmParty. Party Name: "
                          << strPartyName << "  NymID: " << strNymID << "\n";
                    int32_t nHarvested =
                        OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                            strSmartContract, strNymID, false, false, false,
                            false, false);
                    otOut << strLocation
                          << ": OT_API_Msg_HarvestTransactionNumbers: "
                          << nHarvested << "\n";
                    return -1;
                }

                // If you are the last party to sign, then ACTIVATE THE SMART
                // CONTRACT.

                // NOTE: No matter which party you are (perhaps you are the
                // middle one), when you confirm
                // the contract, you will send it on to the NEXT UNCONFIRMED
                // ONE. This means you don't know
                // which party it will be, since all the unconfirmed parties
                // have no NymID (yet.) Rather,
                // it's YOUR problem to provide the NymID you're sending the
                // contract on to. And then it's HIS
                // problem to decide which party he will sign on as. (Unless you
                // are the LAST PARTY to confirm,
                // in which case YOU are the activator.)
                if (OTAPI_Wrap::Smart_AreAllPartiesConfirmed(
                        strSmartContract)) {
                    // The logic: If all the parties are confirmed, then just
                    // activate the thing. I must be the activator.
                    // We need the ACCT_ID that we're using to activate it with,
                    // and we need the AGENT NAME for that account.
                    // strMyAcctID and strMyAcctAgentName might ALREADY be set
                    // -- otherwise we need to set those.
                    if (!VerifyStringVal(strMyAcctID) ||
                        !VerifyStringVal(strMyAcctAgentName)) {
                        // We can't just use ANY account ID, but we must use one
                        // that is listed as one of the accounts for
                        // the party activating the contract. So we have to
                        // display those accounts, and the user must choose
                        // which one it's going to be. From there we can get the
                        // account ID and the agent name and call
                        // activate_smart_contract.
                        stat_partyaccounts(strSmartContract, strPartyName,
                                           2); // nDepth=2

                        otOut << "\n\n Enter the index for the account you'll "
                                 "use to ACTIVATE the smart contract: ";

                        string strAcctIndex = OT_CLI_ReadLine();

                        if (!VerifyStringVal(strAcctIndex)) {
                            int32_t nHarvested =
                                OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                                    strSmartContract, strNymID, false, false,
                                    false, false, false);
                            otOut << strLocation
                                  << ": OT_API_Msg_HarvestTransactionNumbers: "
                                  << nHarvested << "\n";
                            return -1;
                        }

                        int32_t nAcctIndex = std::stol(strAcctIndex);

                        if (nAcctIndex < 0 || nAcctIndex >= nAccountCount) {
                            otOut << "Error: Bad Index: " << strAcctIndex
                                  << " (Acceptable values are 0 through "
                                  << (nAccountCount - 1) << ")\n";
                            int32_t nHarvested =
                                OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                                    strSmartContract, strNymID, false, false,
                                    false, false, false);
                            otOut << strLocation
                                  << ": OT_API_Msg_HarvestTransactionNumbers: "
                                  << nHarvested << "\n";
                            return -1;
                        }

                        string strAcctName =
                            OTAPI_Wrap::Party_GetAcctNameByIndex(
                                strSmartContract, strPartyName, nAcctIndex);

                        if (!VerifyStringVal(strAcctName)) {
                            otOut << "Error: Unable to retrieve account name "
                                     "at index " << strAcctIndex
                                  << " for Party: " << strPartyName << "\n";
                            int32_t nHarvested =
                                OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                                    strSmartContract, strNymID, false, false,
                                    false, false, false);
                            otOut << strLocation
                                  << ": OT_API_Msg_HarvestTransactionNumbers: "
                                  << nHarvested << "\n";
                            return -1;
                        }

                        string strAcctID = OTAPI_Wrap::Party_GetAcctID(
                            strSmartContract, strPartyName, strAcctName);

                        if (!VerifyStringVal(strAcctID)) {
                            otOut << "Strange: the account at index "
                                  << strAcctIndex
                                  << " is not yet confirmed with any account "
                                     "ID. (Failure.)\n";
                            int32_t nHarvested =
                                OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                                    strSmartContract, strNymID, false, false,
                                    false, false, false);
                            otOut << strLocation
                                  << ": OT_API_Msg_HarvestTransactionNumbers: "
                                  << nHarvested << "\n";
                            return -1;
                        }
                        strMyAcctID = strAcctID;

                        string strAcctAgentName =
                            OTAPI_Wrap::Party_GetAcctAgentName(
                                strSmartContract, strPartyName, strAcctName);

                        if (!VerifyStringVal(strAcctAgentName)) {
                            otOut << "Strange: the account named: '"
                                  << strAcctName
                                  << "' doesn't seem to have any agent "
                                     "confirmed to it. (Failure.)\n";
                            int32_t nHarvested =
                                OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                                    strSmartContract, strNymID, false, false,
                                    false, false, false);
                            otOut << strLocation
                                  << ": OT_API_Msg_HarvestTransactionNumbers: "
                                  << nHarvested << "\n";
                            return -1;
                        }
                        strMyAcctAgentName = strAcctAgentName;
                    }

                    // NOTE: Above, you see us harvesting the transaction
                    // numbers if any error occurs, since we haven't even tried
                    // to send the instrument yet.
                    //
                    // But in this next section, we only want to do that if the
                    // transaction was definitely not processed by the server.
                    // (Whether that processing succeeded or failed being a
                    // separate question.)
                    string strResponse = MadeEasy::activate_smart_contract(
                        strServerID, strNymID, strMyAcctID, strMyAcctAgentName,
                        strSmartContract);
                    string strAttempt = "activate_smart_contract";
                    int32_t nMessageSuccess = VerifyMessageSuccess(strResponse);

                    if (-1 == nMessageSuccess) {
                        // In this case, the server never even SAW the
                        // transaction, because its enveloping message already
                        // failed before we even got that far. So we can
                        // harvest, as though it was never attempted...
                        otOut << "Message error: " << strAttempt << ".\n";
                        int32_t nHarvested =
                            OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                                strSmartContract, strNymID, false, false, false,
                                false, false);
                        otOut << strLocation
                              << ": OT_API_Msg_HarvestTransactionNumbers: "
                              << nHarvested << "\n";
                        return -1;
                    }
                    else if (0 == nMessageSuccess) {
                        otOut << "Server reply (" << strAttempt
                              << "): Message failure.\n";
                        int32_t nHarvested =
                            OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                                strSmartContract, strNymID, false, false, false,
                                false, false);
                        otOut << strLocation
                              << ": OT_API_Msg_HarvestTransactionNumbers: "
                              << nHarvested << "\n";
                        return 0;
                    }
                    // (else 1: message was processed successfully -- though not
                    // necessarily the transaction inside it.)

                    // BELOW THIS POINT, the transaction has definitely
                    // processed. It may have succeeded or failed,
                    // but either way, we don't need to harvest any numbers,
                    // since OT will internally handle those
                    // when it processes the server's reply to the transaction
                    // attempt.
                    int32_t nInterpretReply = InterpretTransactionMsgReply(
                        strServerID, strNymID, strMyAcctID, strAttempt,
                        strResponse);

                    if (1 == nInterpretReply) {
                        bool bRetrieved = MadeEasy::retrieve_account(
                            strServerID, strNymID, strMyAcctID, true);

                        otOut << "\nServer response (" << strAttempt
                              << "): SUCCESS activating smart contract!\n";
                        otOut
                            << (bRetrieved ? "Success" : "Failed")
                            << " retrieving intermediary files for account.\n";
                    }
                    return nInterpretReply;
                }
                else // SEND INSTRUMENT TO NEXT PARTY.
                {
                    // But if all the parties are NOT confirmed, then we need to
                    // send it to the next guy. In that case:
                    // If HisNym is provided, and it's different than strNymID,
                    // then use it. He's the next receipient.
                    // If HisNym is NOT provided, then display the list of
                    // NymIDs, and allow the user to paste one. We can
                    // probably select him based on abbreviated ID or Name as
                    // well (I think there's an API call for that...)
                    string strHisNymID = "";

                    if (VerifyExists("HisNym", false)) {
                        strHisNymID = HisNym;
                    }

                    // If strHisNymID doesn't exist, or it's the same as
                    // strNymID, then ask the user to select
                    // a NymID for the recipient.
                    if (!VerifyStringVal(strHisNymID) ||
                        (strHisNymID == strNymID)) {
                        OT_Command::mainShowNyms();

                        otOut << "\nOnce you confirm this contract, then we "
                                 "need to send it on to the\nnext party so he "
                                 "can confirm it, too.\n\nPlease PASTE a "
                                 "recipient Nym ID (the next party): ";
                        string strRecipientNymID = OT_CLI_ReadLine();
                        if (!VerifyStringVal(strRecipientNymID)) {
                            int32_t nHarvested =
                                OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                                    strSmartContract, strNymID, false, false,
                                    false, false, false);
                            otOut << strLocation
                                  << ": OT_API_Msg_HarvestTransactionNumbers: "
                                  << nHarvested << "\n";
                            return -1;
                        }

                        string strHisNymParam = strRecipientNymID;

                        // IF we are able to resolve the HisNymId from a
                        // partial, then we
                        // replace the partial with the full version.
                        // (Otherwise we assume it's already a full ID and we
                        // don't mess with it.)
                        string strHisNymFromPartial =
                            OTAPI_Wrap::Wallet_GetNymIDFromPartial(
                                strRecipientNymID);

                        if (VerifyStringVal(strHisNymFromPartial)) {
                            strHisNymParam = strHisNymFromPartial;
                        }

                        strHisNymID = strHisNymParam;

                        if (strHisNymID == strNymID) {
                            otOut << "\nSorry, but YOU cannot simultaneously "
                                     "be the SENDER _and_ the RECIPIENT. "
                                     "Choose another nym for one or the "
                                     "other.\n";
                            int32_t nHarvested =
                                OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                                    strSmartContract, strNymID, false, false,
                                    false, false, false);
                            otOut << strLocation
                                  << ": OT_API_Msg_HarvestTransactionNumbers: "
                                  << nHarvested << "\n";
                            return -1;
                        }
                    }

                    string strResponse = MadeEasy::send_user_payment(
                        strServerID, strNymID, strHisNymID, strSmartContract);

                    int32_t nMessageSuccess = VerifyMessageSuccess(strResponse);

                    if (1 != nMessageSuccess) {
                        otOut << "\nFor whatever reason, our attempt to send "
                                 "the instrument on to the next user has "
                                 "failed.\n";
                        int32_t nHarvested =
                            OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                                strSmartContract, strNymID, false, false, false,
                                false, false);
                        otOut << strLocation
                              << ": OT_API_Msg_HarvestTransactionNumbers: "
                              << nHarvested << "\n";
                    }
                    else // Success. (Remove the payment instrument we just
                           // successfully sent, from our payments inbox.)
                    {
                        // In the case of smart contracts, it might be sent on
                        // to a chain of 2 or 3 users,
                        // before finally being activated by the last one in the
                        // chain. All of the users in
                        // the chain (except the first one) will thus have a
                        // copy of the smart contract in
                        // their payments inbox AND outbox.
                        //
                        // But once the smart contract has successfully been
                        // sent on to the next user, and
                        // thus a copy of it is in my outbox already, then there
                        // is definitely no reason for
                        // a copy of it to stay in my inbox as well. Might as
                        // well remove that copy.
                        //
                        // We can't really expect to remove the payments inbox
                        // copy inside OT itself, when we
                        // receive the server's atSendUserInstrument reply
                        // message, without opening up the
                        // (encrypted) contents. (Although that would actually
                        // be ideal, since it would cover
                        // all cases included dropped messages...) But we CAN
                        // easily remove it RIGHT HERE.
                        // Perhaps in the future I WILL move this code to the
                        // atSendUserInstrument reply processing,
                        // but that will require it to be encrypted to my own
                        // key as well as the recipient's,
                        // which we already do for sending cash, but which we up
                        // until now have not done for
                        // the other instruments. So perhaps we'll start doing
                        // that sometime in the future, and
                        // then move this code.
                        //
                        // In the meantime, this is good enough.

                        otOut << "Success sending the agreement on to the next "
                                 "party.\n";

                        if (-1 != nIndex) {
                            OTAPI_Wrap::RecordPayment(strServerID, strNymID,
                                                      true, nIndex, false);
                        }
                    }
                    return nMessageSuccess;
                }
            }
        }
    }
    // else there aren't any unconfirmed parties, and it already displayed an
    // error message related to that.

    return -1;
}

// smart contract and payment plan
OT_COMMANDS_OT int32_t OT_Command::mainConfirm()
{
    int32_t nIndex = -1;

    if (VerifyExists("Args", false)) {
        string strIndex = OT_CLI_GetValueByKey(Args, "index");

        if (VerifyStringVal(strIndex)) {
            int32_t nTempIndex = std::stol(strIndex);

            if (nTempIndex >= 0) {
                nIndex = nTempIndex;
            }
        }
    }
    otOut << "\n\n";
    string strInstrument = "";

    if (-1 == nIndex) {
        if (VerifyExists("Server", false) && VerifyExists("MyNym", false)) {
            OT_Command::mainInpayments();

            otOut << "If this is in reference to a smart contract or payment "
                     "plan in your payments\ninbox, please enter the index to "
                     "confirm it (or just hit enter, to paste a contract): ";
            string strIndex = OT_CLI_ReadLine();

            if (VerifyStringVal(strIndex)) {
                int32_t nTempIndex = std::stol(strIndex);

                if (nTempIndex >= 0) {
                    nIndex = nTempIndex;

                    strInstrument = MadeEasy::get_payment_instrument(
                        Server, MyNym, nIndex, "");

                    if (!VerifyStringVal(strInstrument)) {
                        otOut << "\n Unable to get instrument from payments "
                                 "inbox, based on index: " << nIndex << ".\n";
                        return -1;
                    }
                }
                else {
                    return -1;
                }
            }
            else {
                otOut << "\nPlease paste the smart contract or payment "
                         "plan,\nfollowed by a ~ by itself on a blank "
                         "line:\n\n";

                strInstrument = OT_CLI_ReadUntilEOF();

                if (!VerifyStringVal(strInstrument)) {
                    return -1;
                }
            }
        }
        else {
            otOut << "\nPlease paste the smart contract or payment "
                     "plan,\nfollowed by a ~ by itself on a blank line:\n\n";

            strInstrument = OT_CLI_ReadUntilEOF();

            if (!VerifyStringVal(strInstrument)) {
                return -1;
            }
        }
    }
    else {
        if (VerifyExists("Server") && VerifyExists("MyNym")) {
            strInstrument =
                MadeEasy::get_payment_instrument(Server, MyNym, nIndex, "");

            if (!VerifyStringVal(strInstrument)) {
                otOut << "\n Unable to get instrument from payments inbox, "
                         "based on index: " << nIndex << ".\n";
                return -1;
            }
        }
        else {
            otOut << "Since you specified an index, make sure to also specify "
                     "--server and --mynym,\nso you use that index on the "
                     "right payments inbox.\n";
            return -1;
        }
    }

    string strInstrumentType = OTAPI_Wrap::Instrmnt_GetType(strInstrument);

    if (!VerifyStringVal(strInstrumentType)) {
        otOut << "\n\nFailure: Unable to determine instrument type. Expected "
                 "SMART CONTRACT or PAYMENT PLAN.\n";
        return -1;
    }

    // Is the instrument yet valid, or is it expired?
    // Handle both those cases here...
    time64_t tFrom = OTAPI_Wrap::Instrmnt_GetValidFrom(strInstrument);
    time64_t tTo = OTAPI_Wrap::Instrmnt_GetValidTo(strInstrument);
    time64_t tTime = OTAPI_Wrap::GetTime();

    if (tTime < tFrom) {
        if (-1 == nIndex) {
            otOut << "The instrument is not yet within its valid date range. "
                     "(Skipping.)\n";
        }
        else {
            otOut << "The instrument at index " << nIndex
                  << " is not yet within its valid date range. (Skipping.)\n";
        }
        return 0;
    }
    if ((tTo > OT_TIME_ZERO) && (tTime > tTo)) {
        if (-1 == nIndex) {
            otOut << "The instrument is expired. (Harvesting the transaction "
                     "numbers.)\n";
            if (OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                    strInstrument, MyNym, false, false, false, false, false)) {
                return 0;
            }
        }
        else {
            otOut << "The instrument at index " << nIndex
                  << " is expired. (Moving it to the record box.)\n";

            // Since this instrument is expired, remove it from the payments
            // inbox, and move to record box.

            // Note: this harvests.
            if (OTAPI_Wrap::RecordPayment(Server, MyNym, true, nIndex, true)) {
                return 0;
            }
        }
        return -1;
    }

    if ("SMARTCONTRACT" == strInstrumentType) {
        // Success! (It's a smart contract for sure.)
        //
        // BY this point, we know that strInstrument contains a string.
        // We also know that it came from the payments inbox at nIndex, or
        // that it was pasted in and nIndex is -1. We also know that if nIndex
        // is NOT -1, that means Server and MyNym are set, because that's the
        // box we must have gotten the smart contract out of (because someone
        // else sent it to us for confirmation...)
        return details_confirm_smart_contract(strInstrument, nIndex);

    }
    else if ("PAYMENT PLAN" == strInstrumentType) {
        // Success! (It's a payment plan for sure.)
        return details_confirm_plan(strInstrument, nIndex);
    }
    else // CHEQUE VOUCHER INVOICE PURSE
    {
        otOut << "\nFailure: Instrument is not a smart contract or payment "
                 "plan, but rather a: " << strInstrumentType << "\n";
        return -1;
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainEncode()
{
    otOut << "Please enter multiple lines of input to be encoded, followed by "
             "an EOF or a ~ by itself on a blank line:\n\n";
    string strInput = OT_CLI_ReadUntilEOF();

    otOut << "\n\n--------------------------------------\n You entered:\n"
          << strInput << "\n\n";

    string strOutput = OTAPI_Wrap::Encode(strInput, true);

    otOut << "-------------------------------------- \n Encoded:\n\n";

    if (VerifyStringVal(strOutput)) {
        cout << strOutput << "\n";

        otOut << "\n\n";
        return 1;
    }

    otOut << "\n\n";

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainDecode()
{
    otOut << "Please enter multiple lines of OT-armored text to be decoded, "
             "followed by an EOF or a ~ by itself on a blank line:\n\n";
    string strInput = OT_CLI_ReadUntilEOF();

    otOut << "\n\n--------------------------------------\n You entered:\n"
          << strInput << "\n\n";

    string strOutput = OTAPI_Wrap::Decode(strInput, true);

    otOut << "--------------------------------------\n Decoded:\n\n";

    if (VerifyStringVal(strOutput)) {
        cout << strOutput << "\n";

        otOut << "\n\n";
        return 1;
    }

    otOut << "\n\n";

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainEncrypt()
{
    if (!VerifyExists("HisNym")) return -1;

    otOut << "Please enter multiple lines of input to be "
             "encrypted,\nfollowed by an EOF or a ~ by itself on a blank "
             "line:\n\n";
    string strInput = OT_CLI_ReadUntilEOF();

    otOut << "\n\n--------------------------------------\n You entered:\n"
          << strInput << "\n\n";

    string strOutput = OTAPI_Wrap::Encrypt(HisNym, strInput);

    if (VerifyStringVal(strOutput)) {

        otOut << "-------------------------------------- \n Encrypted:\n\n";

        cout << strOutput << "\n";

        otOut << "\n\n";
        return 1;
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainDecrypt()
{
    if (!VerifyExists("MyNym")) return -1;

    otOut << "Please enter multiple lines of input to be decrypted, "
             "followed by an EOF or a ~ by itself on a blank line:\n\n";
    string strInput = OT_CLI_ReadUntilEOF();

    otOut << "\n\n--------------------------------------\n You entered:\n"
          << strInput << "\n\n";

    string strOutput = OTAPI_Wrap::Decrypt(MyNym, strInput);

    if (VerifyStringVal(strOutput)) {

        otOut << "-------------------------------------- \n Decrypted:\n\n";

        cout << strOutput << "\n";

        otOut << "\n\n";
        return 1;
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainPasswordEncrypt()
{
    otOut << "Please enter a symmetric key, followed by a ~ by itself on a "
             "blank line:\n\n";
    string strKey = OT_CLI_ReadUntilEOF();

    otOut << "Please enter the plaintext, followed by a ~ by itself on a blank "
             "line:\n\n";

    string strPlaintext = OT_CLI_ReadUntilEOF();

    if (!VerifyStringVal(strKey) || !VerifyStringVal(strPlaintext)) return -1;

    string strCiphertext = OTAPI_Wrap::SymmetricEncrypt(strKey, strPlaintext);

    if (VerifyStringVal(strCiphertext)) {

        otOut << "-------------------------------------- \n "
                 "strCiphertext:\n\n";

        cout << strCiphertext << "\n";

        otOut << "\n\n";
        return 1;
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainPasswordDecrypt()
{
    otOut << "Please enter a symmetric key, followed by a ~ by itself on a "
             "blank line:\n\n";
    string strKey = OT_CLI_ReadUntilEOF();

    otOut << "Please enter the symmetrically-encrypted ciphertext, followed by "
             "a ~ by itself on a blank line:\n\n";

    string strCiphertext = OT_CLI_ReadUntilEOF();

    if (VerifyStringVal(strKey) && VerifyStringVal(strCiphertext)) {
        string strPlaintext =
            OTAPI_Wrap::SymmetricDecrypt(strKey, strCiphertext);

        if (VerifyStringVal(strPlaintext)) {

            otOut << "-------------------------------------- \n Plaintext:\n\n";

            cout << strPlaintext << "\n";

            otOut << "\n\n";
            return 1;
        }
    }

    return -1;
}

OT_COMMANDS_OT bool OT_Command::details_import_nym(
    const string& strNymImportFile, string& strOutNymID)
{
    strOutNymID = OTAPI_Wrap::Wallet_ImportNym(strNymImportFile);

    bool bVerified = VerifyStringVal(strOutNymID);

    return bVerified;
}

OT_COMMANDS_OT int32_t OT_Command::mainImportNym()
{
    otOut << "Usage:   importnym\n";

    otOut << "Paste your exported Nym here (for import), followed by a ~ by "
             "itself on a blank line: \n";

    string strNymFile = OT_CLI_ReadUntilEOF();

    if (!VerifyStringVal(strNymFile)) {
        return -1;
    }

    string strOutNymID;
    bool bDone = details_import_nym(strNymFile, strOutNymID);

    if (!bDone) {
        otOut << "\n\n FAILED trying to import Nym.\n";
        return -1;
    }

    otOut << "\n\n SUCCESS importing Nym: " << strOutNymID << "\n\n";
    return 1;
}

OT_COMMANDS_OT string OT_Command::details_export_nym(const string& strNymID)
{
    string strExportedNym = OTAPI_Wrap::Wallet_ExportNym(strNymID);

    return strExportedNym;
}

OT_COMMANDS_OT int32_t OT_Command::mainExportNym()
{
    otOut << "Usage:   exportnym --mynym NYM_ID\n";

    if (!VerifyExists("MyNym")) return -1;

    string strExportedNym = details_export_nym(MyNym);

    if (!VerifyStringVal(strExportedNym)) {
        otOut << "\n\n FAILED trying to export Nym: " << MyNym << "\n";
        return -1;
    }

    otOut << "\n\n SUCCESS exporting Nym: " << MyNym << "\n\n";
    cout << strExportedNym << "\n";
    otOut << "\n";
    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainChangePw()
{
    if (OTAPI_Wrap::Wallet_ChangePassphrase()) {
        return 1;
    }

    return -1;
}

OT_COMMANDS_OT int32_t
    OT_Command::details_send_transfer(const string& strMyAcctID,
                                      const string& strHisAcctID,
                                      const string& strAmount,
                                      const string& strNote)
{
    string strMyNymID = OTAPI_Wrap::GetAccountWallet_NymID(strMyAcctID);
    if (!VerifyStringVal(strMyNymID)) {
        otOut << "Failure: Unable to find NymID (for sender) based on myacct. "
                 "Use: --myacct ACCT_ID\n";
        otOut << "The designated asset account must be yours. OT will find the "
                 "Nym based on the account.\n\n";
        return -1;
    }
    if (VerifyExists("MyNym", false) && !(MyNym == strMyNymID)) {
        otOut << "\n\nFailure: MyNym was provided (" << MyNym
              << "), but didn't match the Nym who owns MyAcct. To override, "
                 "use: --mynym " << strMyNymID << "\n\n";
        return -1;
    }

    string strMyServerID = OTAPI_Wrap::GetAccountWallet_ServerID(strMyAcctID);
    if (!VerifyStringVal(strMyServerID)) {
        otOut << "Failure: Unable to find ServerID based on myacct. Use: "
                 "--myacct ACCT_ID\n";
        otOut << "The designated asset account must be yours. OT will find the "
                 "Server based on the account.\n\n";
        return -1;
    }
    if (VerifyExists("Server", false) && !(Server == strMyServerID)) {
        otOut << "\n\nFailure: Server was provided (" << Server
              << "), but didn't match the Server where MyAcct is based. To "
                 "override, use: --server " << strMyServerID << "\n\n";
        return -1;
    }

    // NEXT, we get the Server based on HIS account.
    //
    // That way, we can make sure both accounts are on the same server, before
    // we go riding off into the sunset sending transfer requests to the server.
    string strHisServerID = OTAPI_Wrap::GetAccountWallet_ServerID(strHisAcctID);
    if (!VerifyStringVal(strHisServerID)) {
        otOut << "HisAcct is not in the wallet, so I'm assuming it's on the "
                 "same server as MyAcct. (Proceeding.)\n";

        strHisServerID = strMyServerID;
    }

    if (!(strMyServerID == strHisServerID)) {
        otOut << "\n\nFailure: HisAcct is not on the same server as MyAcct "
                 "(he's on " << strHisServerID << " but MyAcct is on "
              << strMyServerID
              << "). You must choose either a different sender account (using "
                 "--myacct ACCT_ID) or a different recipient account (using "
                 "--hisacct ACCT_ID)\n\n";
        return -1;
    }

    string strAssetTypeID =
        OTAPI_Wrap::GetAccountWallet_AssetTypeID(strMyAcctID);

    int64_t lAmount = OTAPI_Wrap::StringToAmount(strAssetTypeID, strAmount);
    string strResponse = MadeEasy::send_transfer(
        strMyServerID, strMyNymID, strMyAcctID, strHisAcctID, lAmount, strNote);
    string strAttempt = "send_transfer";

    int32_t nInterpretReply = InterpretTransactionMsgReply(
        strMyServerID, strMyNymID, strMyAcctID, strAttempt, strResponse);

    if (1 == nInterpretReply) {
        bool bRetrieved = MadeEasy::retrieve_account(strMyServerID, strMyNymID,
                                                     strMyAcctID, true);

        otOut << "Server response (" << strAttempt
              << "): SUCCESS sending transfer!\n";
        otOut << (bRetrieved ? "Success" : "Failed")
              << " retrieving intermediary files for account.\n";
    }

    return nInterpretReply;
}

// HERE, WE GET ALL THE ARGUMENTS TOGETHER,
// and then call the above function.
//
OT_COMMANDS_OT int32_t OT_Command::mainTransfer()
{
    otOut << "Usage:   transfer --myacct YOUR_ASSET_ACCT --hisacct "
             "RECIPIENT_ASSET_ACCT\n\nAlso NECESSARY: --args \"amount "
             "PUT_AMOUNT_HERE\"\nAnd OPTIONALLY: --args \"memo \\\"Just a memo "
             "for the transfer.\\\"\"\n\n";

    if (!VerifyExists("MyAcct") || !VerifyExists("HisAcct")) return -1;

    string strAssetTypeID = OTAPI_Wrap::GetAccountWallet_AssetTypeID(MyAcct);
    if (!VerifyStringVal(strAssetTypeID)) {
        otOut << "\n\nFailure: Unable to find asset ID based on myacct. "
                 "Use: --myacct ACCT_ID\n";
        return -1;
    }

    string strAmount = "0";
    string strNote = "";

    string strDefaultAmount = "10";
    string strDefaultNote = "(blank memo field)";

    // If custom arguments have been passed on the command line,
    // then grab them and use them instead of asking the user to enter them
    // at the command line.
    if (VerifyExists("Args", false)) {
        string strNewAmount = OT_CLI_GetValueByKey(Args, "amount");
        string strNewNote = OT_CLI_GetValueByKey(Args, "memo");

        // Set the values based on the custom arguments, for those found.
        if (VerifyStringVal(strNewAmount)) {
            strAmount = strNewAmount;
        }
        if (VerifyStringVal(strNewNote)) {
            strNote = strNewNote;
        }
    }

    // If the transfer parameters aren't provided, then we
    // ask the user to supply them at the command line.
    if (!VerifyStringVal(strAmount) ||
        (OTAPI_Wrap::StringToAmount(strAssetTypeID, strAmount) < 1)) {
        otOut << "Enter the amount[" << strDefaultAmount << "]: ";
        strAmount = OT_CLI_ReadLine();
    }
    if (!VerifyStringVal(strNote)) {
        otOut << "Optionally, enter a memo on a single line[" << strDefaultNote
              << "]: ";
        strNote = OT_CLI_ReadLine();
    }

    if (!VerifyStringVal(strAmount) ||
        (OTAPI_Wrap::StringToAmount(strAssetTypeID, strAmount) < 1)) {
        strAmount = strDefaultAmount;
    }
    if (!VerifyStringVal(strNote)) {
        strNote = strDefaultNote;
    }

    return details_send_transfer(MyAcct, HisAcct, strAmount, strNote);
}

OT_COMMANDS_OT int32_t OT_Command::mainEditNym()
{
    otOut << "Usage:   editnym --mynym YOUR_NYM_ID\nAlso optionally:         "
             "--args \"label \\\"PUT LABEL HERE\\\"\"\n";

    if (!VerifyExists("MyNym")) return -1;

    string strLabel = "";

    string strDefaultLabel = "(blank label)";

    // If custom arguments have been passed on the command line,
    // then grab them and use them instead of asking the user to enter them
    // at the command line.
    if (VerifyExists("Args", false)) {
        string strNewLabel = OT_CLI_GetValueByKey(Args, "label");

        // Set the value based on the custom argument.
        if (VerifyStringVal(strNewLabel)) {
            strLabel = strNewLabel;
        }
    }

    // If the label isn't provided, then we ask the
    // user to supply it at the command line.
    if (!VerifyStringVal(strLabel)) {
        otOut << "Enter MyNym's new label [" << strDefaultLabel << "]: ";
        strLabel = OT_CLI_ReadLine();
    }

    if (!VerifyStringVal(strLabel)) {
        strLabel = strDefaultLabel;
    }

    bool bSet = OTAPI_Wrap::SetNym_Name(MyNym, MyNym, strLabel);

    if (!bSet) {
        otOut << "\n\n FAILED trying to set MyNym's label to: " << strLabel
              << "\n";
        otOut << "MyNym ID: " << MyNym << "\n\n";
        return -1;
    }

    otOut << "\n\n SUCCESS setting MyNym's label to: " << strLabel << "\n";
    otOut << "MyNym ID: " << MyNym << "\n\n";
    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainEditAccount()
{
    otOut << "Usage:   editaccount --myacct YOUR_ACCT_ID\nAlso optionally:     "
             "     --args \"label \\\"PUT LABEL HERE\\\"\"\n";

    if (!VerifyExists("MyAcct")) return -1;

    string strMyNymID = OTAPI_Wrap::GetAccountWallet_NymID(MyAcct);
    if (!VerifyStringVal(strMyNymID)) {
        otOut << "\n\nFailure: Unable to find NymID based on myacct. Use: "
                 "--myacct ACCT_ID\n";
        otOut << "The designated asset account must be yours. OT will find "
                 "the Nym based on the account.\n\n";
        return -1;
    }
    if (VerifyExists("MyNym") && !(MyNym == strMyNymID)) {
        otOut << "\n\nFailure: MyNym was provided, but didn't match the "
                 "Nym who owns MyAcct. To override, use: --mynym " << strMyNymID
              << "\n\n";
        return -1;
    }
    string strLabel = "";

    string strDefaultLabel = "(blank label)";

    // If custom arguments have been passed on the command line,
    // then grab them and use them instead of asking the user to enter them
    // at the command line.
    if (VerifyExists("Args", false)) {
        string strNewLabel = OT_CLI_GetValueByKey(Args, "label");

        // Set the value based on the custom argument.
        if (VerifyStringVal(strNewLabel)) {
            strLabel = strNewLabel;
        }
    }

    // If the label isn't provided, then we ask the
    // user to supply it at the command line.
    if (!VerifyStringVal(strLabel)) {
        otOut << "Enter MyAcct's new label [" << strDefaultLabel << "]: ";
        strLabel = OT_CLI_ReadLine();
    }

    if (!VerifyStringVal(strLabel)) {
        strLabel = strDefaultLabel;
    }

    bool bSet = OTAPI_Wrap::SetAccountWallet_Name(MyAcct, strMyNymID, strLabel);

    if (!bSet) {
        otOut << "\n\n FAILED trying to set MyAcct's label to: " << strLabel
              << "\n";
        otOut << "MyAcct ID: " << MyAcct << "\n";
        otOut << " MyNym ID: " << strMyNymID << "\n\n";
        return -1;
    }

    otOut << "\n\n SUCCESS setting MyAcct's label to: " << strLabel << "\n";
    otOut << "MyAcct ID: " << MyAcct << "\n";
    otOut << " MyNym ID: " << strMyNymID << "\n\n";
    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainEditAsset()
{
    otOut << "Usage:   editasset --mypurse ASSET_TYPE_ID\nAlso optionally:     "
             "      --args \"label \\\"PUT LABEL HERE\\\"\"\n";

    if (!VerifyExists("MyPurse")) return -1;

    string strLabel = "";

    string strDefaultLabel = "(blank label)";

    // If custom arguments have been passed on the command line,
    // then grab them and use them instead of asking the user to enter them
    // at the command line.
    if (VerifyExists("Args", false)) {
        string strNewLabel = OT_CLI_GetValueByKey(Args, "label");

        // Set the value based on the custom argument.
        if (VerifyStringVal(strNewLabel)) {
            strLabel = strNewLabel;
        }
    }

    // If the label isn't provided, then we ask the
    // user to supply it at the command line.
    if (!VerifyStringVal(strLabel)) {
        otOut << "Enter MyPurse's new label [" << strDefaultLabel << "]: ";
        strLabel = OT_CLI_ReadLine();
    }

    if (!VerifyStringVal(strLabel)) {
        strLabel = strDefaultLabel;
    }

    bool bSet = OTAPI_Wrap::SetAssetType_Name(MyPurse, strLabel);

    if (!bSet) {
        otOut << "\n\n FAILED trying to set MyPurse's label to: " << strLabel
              << "\n";
        otOut << "MyPurse (AssetTypeID): " << MyPurse << "\n\n";
        return -1;
    }

    otOut << "\n\n SUCCESS setting MyPurse's label to: " << strLabel << "\n";
    otOut << "MyPurse (AssetTypeID): " << MyPurse << "\n\n";
    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainEditServer()
{
    otOut << "Usage:   editserver --server SERVER_ID\nAlso optionally:         "
             "   --args \"label \\\"PUT LABEL HERE\\\"\"\n";

    if (!VerifyExists("Server")) return -1;

    string strLabel = "";

    string strDefaultLabel = "(blank label)";

    // If custom arguments have been passed on the command line,
    // then grab them and use them instead of asking the user to enter them
    // at the command line.
    if (VerifyExists("Args", false)) {
        string strNewLabel = OT_CLI_GetValueByKey(Args, "label");

        // Set the value based on the custom argument.
        if (VerifyStringVal(strNewLabel)) {
            strLabel = strNewLabel;
        }
    }

    // If the label isn't provided, then we ask the
    // user to supply it at the command line.
    if (!VerifyStringVal(strLabel)) {
        otOut << "Enter Server's new label [" << strDefaultLabel << "]: ";
        strLabel = OT_CLI_ReadLine();
    }

    if (!VerifyStringVal(strLabel)) {
        strLabel = strDefaultLabel;
    }

    bool bSet = OTAPI_Wrap::SetServer_Name(Server, strLabel);

    if (!bSet) {
        otOut << "\n\n FAILED trying to set Server's label to: " << strLabel
              << "\n";
        otOut << "Server ID: " << Server << "\n\n";
        return -1;
    }

    otOut << "\n\n SUCCESS setting Server's label to: " << strLabel << "\n";
    otOut << "Server ID: " << Server << "\n\n";
    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainSendMessage()
{
    otOut << "Usage:   sendmessage --server <SERVER_ID> --mynym <YOUR_NYM_ID> "
             "--hisnym <RECIPIENT_NYM_ID>\n\n";

    if (!VerifyExists("Server") || !VerifyExists("MyNym") ||
        !VerifyExists("HisNym"))
        return -1;

    string strName = OTAPI_Wrap::GetServer_Name(Server);
    if (!VerifyStringVal(strName)) {
        otOut << "Error: unkknown server ID: " << Server << "\n";
        return -1;
    }

    string myPubKey =
        MadeEasy::load_or_retrieve_encrypt_key(Server, MyNym, MyNym);
    if (!VerifyStringVal(myPubKey)) {
        otOut << "\n Failed: Unknown MyNym ID: " << MyNym << "\n";
        return -1;
    }

    string hisPubKey =
        MadeEasy::load_or_retrieve_encrypt_key(Server, MyNym, HisNym);
    if (!VerifyStringVal(hisPubKey)) {
        otOut << "\n Failed: Unknown HisNym ID: " << HisNym << "\n";
        return -1;
    }

    otOut << "Please enter your message on multiple lines, optionally "
             "beginning with a \"Subject: \" line.\n";
    otOut << "Use Ctrl-C to cancel, otherwise finish your message with an "
             "EOF or a ~ by itself on a blank line:\n\n";

    string strMessage = OT_CLI_ReadUntilEOF();

    string strResponse =
        MadeEasy::send_user_msg(Server, MyNym, HisNym, strMessage);
    if (1 != VerifyMessageSuccess(strResponse)) {
        otOut << "send_user_msg: Failed.\n";
        return -1;
    }

    otOut << "Success in send_user_msg! Server response:\n\n";
    cout << strResponse << "\n";
    otOut << "\nSuccess in send_user_msg.\n\n";
    return 1;
}

OT_COMMANDS_OT int32_t
    OT_Command::details_write_cheque(string& strCheque, bool bIsInvoice)
{
    Utility MsgUtil;

    if (!VerifyExists("MyAcct")) return -1;

    string strMyNymID = OTAPI_Wrap::GetAccountWallet_NymID(MyAcct);
    if (!VerifyStringVal(strMyNymID)) {
        otOut << "Failure: Unable to find NymID (for sender) based on "
                 "myacct. Use: --myacct ACCT_ID\n";
        otOut << "The designated asset account must be yours. OT will find "
                 "the Nym based on the account.\n\n";
        return -1;
    }
    if (VerifyExists("MyNym", false) && !(MyNym == strMyNymID)) {
        otOut << "\n\nFailure: MyNym was provided (" << MyNym
              << "), but didn't match the Nym who owns MyAcct. To "
                 "override, use: --mynym " << strMyNymID << "\n\n";
        return -1;
    }

    string strMyServerID = OTAPI_Wrap::GetAccountWallet_ServerID(MyAcct);
    if (!VerifyStringVal(strMyServerID)) {
        otOut << "Failure: Unable to find ServerID based on myacct. Use: "
                 "--myacct ACCT_ID\n";
        otOut << "The designated asset account must be yours. OT will find "
                 "the Server based on the account.\n\n";
        return -1;
    }
    if (VerifyExists("Server", false) && !(Server == strMyServerID)) {
        otOut << "\n\nFailure: Server was provided (" << Server
              << "), but didn't match the Server where MyAcct is based. To "
                 "override, use: --server " << strMyServerID << "\n\n";
        return -1;
    }
    //
    // When we pass in HisNym at the command line, using this option:
    // --hisnym HIS_NYM_ID
    // then OT internally TRIES to look it up on the wallet (and someday on
    // the address
    // book as well) in abbreviated or name form, in order to substitute it
    // with the ACTUAL
    // full NymID, before passing it into the script where we are now. If
    // nothing is found,
    // then it just passes through the ID so we have a chance to download
    // the pubkey and
    // verify that it actually exists.
    //
    // Therefore, if the full "HisNym" ID hasn't ALREADY been found by now,
    // that means we
    // already couldn't find it via abbreviation or name, and therefore we
    // will HAVE to download
    // it from the server since we don't have it already. (And therefore we
    // will NEED to know the
    // FULL ID, since we cannot download a pubkey from a server based on a
    // partial ID.)
    //
    // The point is that when we get here, there's no need to worry about
    // searching for a partial
    // or abbreviated version, and no need to ask load_or_retrieve_pubkey to
    // somehow translate HisNym
    // to a full ID. That would have already happened by now, if one were
    // available. So at this point
    // we basically have to assume the user passed the full and correct ID,
    // and if it's not in the
    // wallet already, then load_or_retrieve_pubkey can go ahead and try to
    // download it as if it is
    // the full and correct ID. If it fails, it fails.
    string strRecipientPubkey = "";
    string strHisNymID = "";

    if (VerifyExists("HisNym", false)) {
        strRecipientPubkey = MadeEasy::load_or_retrieve_encrypt_key(
            strMyServerID, strMyNymID, HisNym);
        // Note: even thogh send_user_payment already calls
        // load_or_retrieve_pubkey, we do it
        // here anyway, BEFORE trying to write the cheque. (No point writing
        // a cheque to HisNym until
        // we're sure it's real...)
        if (!VerifyStringVal(strRecipientPubkey)) {
            otOut << "\n\nFailure: Unable to load or download pubkey for "
                     "HisNym based on given value (" << HisNym
                  << "). To override, use: --hisnym HIS_NYM_ID\n\n";
            return -1;
        }

        strHisNymID = HisNym;
    }

    // Make sure we have at least one transaction number (to write the
    // cheque...)
    if (!MadeEasy::insure_enough_nums(10, strMyServerID, strMyNymID)) {
        return -1;
    }

    // - At this point we have MyAcct writing a cheque to HisNym.
    // - We have strMyServerID and strMyNymID and we know they match MyAcct.
    // (And we know they match --server and --mynym as well, otherwise the
    // user would have been be forced to override at the command line before
    // allowing him to continue.)
    // - We know that HisNym is a valid enough ID that we were able to load
    // his public key, or download it and THEN load it up. By this point it
    // was
    // successful either way.
    // - We know that the Nym has at least 1 valid transaction number, with
    // which to write the cheque. (If he didn't, we would have downloaded it
    // in
    // the above block.)
    // - Therefore let's collect all the other cheque-related data, and
    // write
    // the actual cheque, and return it to the caller.

    string strDefaultAmount = "1";
    string strDefaultMemo = "(memo field)";
    int64_t nDefaultLength = OTTimeGetSecondsFromTime(OT_TIME_MONTH_IN_SECONDS);

    string strAmount = "0";
    string strMemo = "";
    int64_t nLength = OTTimeGetSecondsFromTime(OT_TIME_ZERO);

    string strAssetTypeID = OTAPI_Wrap::GetAccountWallet_AssetTypeID(MyAcct);

    if (VerifyExists("Args")) {
        string strNewAmount = OT_CLI_GetValueByKey(Args, "amount");
        string strNewMemo = OT_CLI_GetValueByKey(Args, "memo");
        string strNewLength = OT_CLI_GetValueByKey(Args, "validfor");

        if (VerifyStringVal(strNewMemo)) {
            strMemo = strNewMemo;
        }

        if (VerifyStringVal(strNewAmount) &&
            (OTAPI_Wrap::StringToAmount(strAssetTypeID, strNewAmount) > 0)) {
            strAmount = strNewAmount;
        }

        if (VerifyStringVal(strNewLength) && (std::stol(strNewLength) > 0)) {
            nLength = std::stol(strNewLength);
        }
    }

    // If the transfer parameters aren't provided, then we
    // ask the user to supply them at the command line.
    if (!VerifyStringVal(strAmount) ||
        (OTAPI_Wrap::StringToAmount(strAssetTypeID, strAmount) < 1)) {
        otOut << "Enter the amount[" << strDefaultAmount << "]: ";
        strAmount = OT_CLI_ReadLine();
    }
    if (!VerifyStringVal(strMemo)) {
        otOut << "Optionally, enter a note on a single line[" << strDefaultMemo
              << "]: ";
        strMemo = OT_CLI_ReadLine();
    }
    if (nLength < 1) {
        otOut << "Enter the 'valid for' time period, in seconds (default "
                 "is 30 days.) [" << nDefaultLength << "]: ";
        string strTemp = OT_CLI_ReadLine();

        if (VerifyStringVal(strTemp) && (std::stol(strTemp) > 0)) {
            nLength = std::stol(strTemp);
        }
    }

    if (!VerifyStringVal(strAmount) ||
        (OTAPI_Wrap::StringToAmount(strAssetTypeID, strAmount) < 1)) {
        strAmount = strDefaultAmount;
    }

    if (!VerifyStringVal(strMemo)) {
        strMemo = strDefaultMemo;
    }

    if (nLength < 1) {
        nLength = nDefaultLength;
    }

    // Todo: use Args feature here to allow an option to override nLength.
    // If it's not used, go with the default of 30 days (above.)

    string strLength = std::to_string(nLength);

    time64_t tFrom = OTAPI_Wrap::GetTime();
    int64_t tLength = std::stoll(strLength);
    time64_t tTo = OTTimeAddTimeInterval(tFrom, tLength);

    if (bIsInvoice) {
        int64_t lTempAmount = (int64_t(-1) * OTAPI_Wrap::StringToAmount(
                                                 strAssetTypeID, strAmount));
        strAmount = OTAPI_Wrap::FormatAmount(strAssetTypeID, lTempAmount);
    }

    int64_t lAmount = OTAPI_Wrap::StringToAmount(strAssetTypeID, strAmount);

    strCheque =
        OTAPI_Wrap::WriteCheque(strMyServerID, lAmount, tFrom, tTo, MyAcct,
                                strMyNymID, strMemo, strHisNymID);

    // Record it in the records?
    // Update: We wouldn't record that here. Instead,
    // OTAPI_Wrap::WriteCheque should drop a notice
    // into the payments outbox, the same as it does when you "sendcheque"
    // (after all, the same
    // resolution would be expected once it is cashed.)

    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainWriteCheque()
{
    otOut << "Usage:   writecheque  --myacct <MY_ACCT_ID> --hisnym "
             "<HIS_NYM_ID>\nOptionally: --args \"memo \\\"one-line memo "
             "allowed here.\\\" amount AMOUNT\"\nAdditionally: --args "
             "\"validfor IN_SECONDS\" \nThis command WRITES but DOES NOT SEND "
             "the cheque. (Use sendcheque for that.)\n\n";

    string strCheque = "";
    int32_t nReturnVal = details_write_cheque(strCheque, false);

    if ((1 == nReturnVal) && VerifyStringVal(strCheque)) {

        otOut << "\n-------------------------------------------\n the "
                 "cheque:\n\n";

        cout << strCheque << "\n";

        otOut << "\n";
    }

    return nReturnVal;
}

OT_COMMANDS_OT int32_t OT_Command::mainWriteInvoice()
{
    otOut << "Usage:   writeinvoice  --myacct <MY_ACCT_ID> --hisnym "
             "<HIS_NYM_ID>\nOptionally: --args \"memo \\\"one-line memo "
             "allowed here.\\\" amount AMOUNT\"\nAdditionally: --args "
             "\"validfor IN_SECONDS\" \nThis command WRITES but DOES NOT SEND "
             "the invoice. (Use sendinvoice for that.)\n";

    string strCheque = "";
    int32_t nReturnVal = details_write_cheque(strCheque, true);

    if ((1 == nReturnVal) && VerifyStringVal(strCheque)) {

        otOut << "\n-------------------------------------------\n the "
                 "invoice:\n\n";

        cout << strCheque << "\n";

        otOut << "\n";
    }

    return nReturnVal;
}

OT_COMMANDS_OT int32_t OT_Command::mainSendCash()
{

    otOut << "Usage:   sendcash  --[myacct|mypurse] <ID> --hisnym "
             "<RECIPIENT_NYM_ID>\nFor mypurse, the server and nym are also "
             "required: --server <SERVER_ID> --mynym <NYM_ID> \nOptionally: "
             "--args \"passwd true\"  (To send a password-protected "
             "purse.)\nOptionally: --args \"memo \\\"one-line memo allowed "
             "here.\\\" amount AMOUNT\"\nThis command sends cash to the "
             "recipient, from your purse if specified, and withdrawing first "
             "from your account, if necessary.\n\n";

    string strServerID = "";
    string strMyAcctID = "";
    string strMyNymID = "";
    string strAssetTypeID = "";

    bool bServerExists = VerifyExists("Server", false);
    bool bMyPurseExists = VerifyExists("MyPurse", false);
    bool bMyAcctExists = VerifyExists("MyAcct", false);
    bool bMyNymExists = VerifyExists("MyNym", false);

    if (!bMyPurseExists && !bMyAcctExists) {
        otOut << "\n You must provide EITHER --myacct <ACCT_ID> OR --mypurse "
                 "<ASSET_TYPE_ID>\nFor mypurse, the server and nym are also "
                 "required: --server <SERVER_ID> --mynym <NYM_ID> \n";
        return -1;
    }

    if (bMyPurseExists) {
        strAssetTypeID = MyPurse;

        if (bServerExists && bMyNymExists) {
            strServerID = Server;
            strMyNymID = MyNym;
        }
        else {
            otOut << "\n MyPurse was specified, but you must also specify a "
                     "server and nym:  --server <SERVER_ID> --mynym "
                     "<NYM_ID>\nMaybe we can find them using MyAcct...\n";
        }
    }
    // NOTE: If MyNym and Server are not provided here, but MyAcct IS, the nym
    // and server can still be derived from MyAcct.
    // The purse will still be used, but only if it has the same asset type ID
    // as MyAcct does. (Meaning there was no point,
    // then, in specifying the purse at all, since all the info was derived from
    // the account in that case.)

    if (bMyAcctExists) {
        string strAcctAssetID =
            OTAPI_Wrap::GetAccountWallet_AssetTypeID(MyAcct);
        if (!VerifyStringVal(strAcctAssetID)) {
            otOut
                << "\n Failed trying to retrieve AssetTypeID based on MyAcct: "
                << MyAcct << "\n";
            return -1;
        }

        string strAcctServerID = OTAPI_Wrap::GetAccountWallet_ServerID(MyAcct);
        if (!VerifyStringVal(strAcctServerID)) {
            otOut << "\n Failed trying to retrieve ServerID based on MyAcct: "
                  << MyAcct << "\n";
            return -1;
        }

        string strAcctNymID = OTAPI_Wrap::GetAccountWallet_NymID(MyAcct);
        if (!VerifyStringVal(strAcctNymID)) {
            otOut << "\n Failed trying to retrieve NymID based on MyAcct: "
                  << MyAcct << "\n";
            return -1;
        }

        if (VerifyStringVal(strServerID) && strAcctServerID != strServerID) {
            otOut << "\n Server ID provided on the command line doesn't match "
                     "the Server ID for MyAcct. Expected: " << strAcctServerID
                  << "\n Try adding: --server " << strAcctServerID << "\n";
            return -1;
        }

        if (VerifyStringVal(strAssetTypeID) &&
            strAcctAssetID != strAssetTypeID) {
            otOut << "\n Asset Type ID provided on the command line doesn't "
                     "match the Asset Type ID of MyAcct. Expected: "
                  << strAcctAssetID << "\n Try adding: --mypurse "
                  << strAcctAssetID << "\n";
            return -1;
        }

        if (VerifyStringVal(strMyNymID) && strAcctNymID != strMyNymID) {
            otOut << "\n Nym ID provided on the command line doesn't match the "
                     "Nym ID for MyAcct. Expected: " << strAcctNymID
                  << "\n Try adding: --mynym " << strAcctNymID << "\n";
            return -1;
        }

        strServerID = strAcctServerID;
        strAssetTypeID = strAcctAssetID;
        strMyNymID = strAcctNymID;
        strMyAcctID = MyAcct;
    }

    // Since MyPurse might have been specified with NO server or Nym, and with
    // MyAcct NOT specified,
    // we could potentially end up down here without a server or Nym still. This
    // is only possible because
    // we allow the purse to fall through, in case those values are picked up
    // from the account instead.
    // But by this point, we have to MAKE SURE we have a server and Nym. So
    // let's do that now:

    if (!VerifyStringVal(strServerID)) {
        otOut << "\n Failed: Missing Server ID. Try adding: --server "
                 "<SERVER_ID>\n";
        return -1;
    }

    if (!VerifyStringVal(strMyNymID)) {
        otOut << "\n Failed: Missing MyNym ID. Try adding: --mynym <NYM_ID>\n";
        return -1;
    }

    // Below this point, we KNOW there's EITHER a purse OR account (or both.)
    // We know if there is only a purse specified, that it properly also has
    // a serverID and nymID. (MyPurse itself being the asset type ID.)
    //
    // We know that if a purse AND account are both specified, that they have
    // matching asset types, AND matching server and nym IDs. You might think
    // there'd be no point in doing that, since you could just not specify the
    // purse at all, and only specify the account. And you'd be right! Either
    // way,
    // we'll try to pay it first out of the cash purse, and only withdraw from
    // the account if we're unable to pay it solely through the purse first.
    //
    // We know that strServerID is correct, strAssetTypeID is correct, and
    // strMyNymID
    // is correct, whether we're using an account or a purse. And we know that
    // if
    // we're using an account, we know that strMyAcctID is correct as well.
    //
    // Below this point we can just try to pay it from the purse, and if unable
    // to,
    // try to get the remaining funds from the account, IF that's available.
    if (!VerifyExists("HisNym")) {
        otOut
            << "\n Failed: Missing HisNym ID. Try adding: --hisnym <NYM_ID>\n";
        return -1;
    }

    // Make sure the recipient exists first.
    // That will avoid starting the cash withdrawal process unnecessarily
    string hisPubKey =
        MadeEasy::load_or_retrieve_encrypt_key(strServerID, strMyNymID, HisNym);
    if (!VerifyStringVal(hisPubKey)) {
        otOut << "\n Failed: Unknown HisNym ID: " << HisNym << "\n";
        return -1;
    }

    string strAmount = "0";
    string strMemo = "";

    string strDefaultAmount = "1";
    string strDefaultMemo = "(blank memo field)";

    string strIndices = "";
    bool bPasswordProtected = false;

    // If custom arguments have been passed on the command line,
    // then grab them and use them instead of asking the user to enter them
    // at the command line.
    if (VerifyExists("Args", false)) {
        strIndices = OT_CLI_GetValueByKey(Args, "indices");

        string strPasswordProtected = OT_CLI_GetValueByKey(Args, "passwd");

        string strNewAmount = OT_CLI_GetValueByKey(Args, "amount");
        string strNewMemo = OT_CLI_GetValueByKey(Args, "memo");

        if (VerifyStringVal(strNewAmount)) {
            strAmount = strNewAmount;
        }
        if (VerifyStringVal(strNewMemo)) {
            strMemo = strNewMemo;
        }
        if (VerifyStringVal(strPasswordProtected) &&
            ("true" == strPasswordProtected)) {
            bPasswordProtected = true;
        }
    }

    // If the withdrawal parameters aren't provided, then we
    // ask the user to supply them at the command line.
    if ((!VerifyStringVal(strAmount) ||
         (OTAPI_Wrap::StringToAmount(strAssetTypeID, strAmount) < 1)) &&
        !VerifyStringVal(strIndices)) {
        otOut << "Enter the amount[" << strDefaultAmount << "]: ";
        strAmount = OT_CLI_ReadLine();
    }

    if (!VerifyStringVal(strAmount) ||
        (OTAPI_Wrap::StringToAmount(strAssetTypeID, strAmount) < 1)) {
        strAmount = strDefaultAmount;
    }

    if (!VerifyStringVal(strMemo)) {
        otOut << "Optionally, enter a memo on a single line[" << strDefaultMemo
              << "]: ";
        strMemo = OT_CLI_ReadLine();
    }

    string strResponse = "";

    int32_t nReturnVal = details_send_cash(
        strResponse, strServerID, strAssetTypeID, strMyNymID, strMyAcctID,
        HisNym, strMemo, strAmount, strIndices, bPasswordProtected);

    if (1 != nReturnVal) {
        otOut << "mainSendCash: Failed in details_send_cash.\n";
        return -1;
    }

    otOut << "Success in sendcash! Server response:\n\n";
    cout << strResponse << "\n";
    otOut << "(Success in sendcash)\n";

    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainSendCheque()
{
    otOut << "Usage:   sendcheque  --myacct <MY_ACCT_ID> --hisnym "
             "<RECIPIENT_NYM_ID>\nOptionally: --args \"memo \\\"one-line memo "
             "allowed here.\\\" amount AMOUNT\"\nAdditionally: --args "
             "\"validfor IN_SECONDS\" \nThis command WRITES AND SENDS the "
             "cheque.\n(Use 'writecheque', not 'sendcheque', if you don't want "
             "it to be immediately SENT.)\n";

    string strCheque = "";
    int32_t nReturnVal = details_write_cheque(strCheque, false);

    if ((1 == nReturnVal) && VerifyStringVal(strCheque)) {

        otOut << "\n-------------------------------------------\n the "
                 "cheque:\n\n";

        cout << strCheque << "\n";

        otOut << "\n";

        // TODO: Store a copy in outpayments box (OR VERIFY THAT IT'S ALREADY
        // BEING DONE, WHICH I BELIEVE THAT IT IS.)

        string strServerID = OTAPI_Wrap::Instrmnt_GetServerID(strCheque);
        string strSenderNymID = OTAPI_Wrap::Instrmnt_GetSenderUserID(strCheque);
        string strRecipientNymID =
            OTAPI_Wrap::Instrmnt_GetRecipientUserID(strCheque);

        if (!VerifyStringVal(strServerID)) {
            otOut << "Unable to retrieve a Server ID from this cheque! Very "
                     "strange. (Failure.)\n";
            return -1;
        }
        if (!VerifyStringVal(strSenderNymID)) {
            otOut << "Unable to retrieve a Sender Nym ID from this cheque! "
                     "Very strange. (Failure.)\n";
            return -1;
        }
        if (!VerifyStringVal(strRecipientNymID)) {
            otOut << "Unable to retrieve a Recipient Nym ID from this cheque! "
                     "Very strange. (Failure.)\n(Although it's fine to WRITE a "
                     "cheque with no recipient, still need the ID to SEND "
                     "one.)\n";
            return -1;
        }

        string strResponse = MadeEasy::send_user_payment(
            strServerID, strSenderNymID, strRecipientNymID, strCheque);
        nReturnVal = VerifyMessageSuccess(strResponse);

        if (1 != nReturnVal) {
            otOut << "sendcheque: Failed.\n";

            if (VerifyStringVal(strResponse)) {
                OTAPI_Wrap::Msg_HarvestTransactionNumbers(
                    strResponse, strSenderNymID, false, false, false, false,
                    false);
            }
        }
        else {
            otOut << "Success in sendcheque! Server response:\n\n";
            cout << strResponse << "\n";
            otOut << "(Success in sendcheque)\n";
            return 1;
        }
    }

    return nReturnVal;
}

OT_COMMANDS_OT int32_t OT_Command::mainSendInvoice()
{
    otOut << "Usage:   sendinvoice  --myacct <MY_ACCT_ID> --hisnym "
             "<RECIPIENT_NYM_ID>\nOptionally: --args \"memo \\\"one-line memo "
             "allowed here.\\\" amount AMOUNT\"\nAdditionally: --args "
             "\"validfor IN_SECONDS\" \nThis command WRITES AND SENDS the "
             "invoice. (Use 'writeinvoice', not 'sendinvoice',\nif you don't "
             "want it to be immediately SENT.)\n";

    string strCheque = "";
    int32_t nReturnVal = details_write_cheque(strCheque, true);

    if ((1 == nReturnVal) && VerifyStringVal(strCheque)) {

        otOut << "\n-------------------------------------------\n the "
                 "invoice:\n\n";

        cout << strCheque << "\n";

        otOut << "\n";

        // TODO: Store a copy in outpayments box (OR VERIFY THAT IT'S ALREADY
        // BEING DONE, WHICH I BELIEVE THAT IT IS.)

        string strServerID = OTAPI_Wrap::Instrmnt_GetServerID(strCheque);
        string strSenderNymID = OTAPI_Wrap::Instrmnt_GetSenderUserID(strCheque);
        string strRecipientNymID =
            OTAPI_Wrap::Instrmnt_GetRecipientUserID(strCheque);

        if (!VerifyStringVal(strServerID)) {
            otOut << "Unable to retrieve a Server ID from this invoice! Very "
                     "strange. (Failure.)\n";
            return -1;
        }
        if (!VerifyStringVal(strSenderNymID)) {
            otOut << "Unable to retrieve a Sender Nym ID from this invoice! "
                     "Very strange. (Failure.)\n";
            return -1;
        }
        if (!VerifyStringVal(strRecipientNymID)) {
            otOut << "Unable to retrieve a Recipient Nym ID (Endorsee) from "
                     "this invoice! Very strange. (Failure.)\n(Although it's "
                     "fine to WRITE a invoice with no endorsee, still need the "
                     "ID to SEND it.)\n";
            return -1;
        }

        string strResponse = MadeEasy::send_user_payment(
            strServerID, strSenderNymID, strRecipientNymID, strCheque);
        nReturnVal = VerifyMessageSuccess(strResponse);

        if (1 != nReturnVal) {
            otOut << "sendinvoice: Failed.\n";
        }
        else {
            otOut << "Success in sendinvoice! Server response:\n\n";
            cout << strResponse << "\n";
            otOut << "(Success in sendinvoice)\n";
            return 1;
        }
    }

    return nReturnVal;
}

OT_COMMANDS_OT int32_t
    OT_Command::details_create_offer(const string& strScale,
                                     const string& strMinIncrement,
                                     const string& strQuantity,
                                     const string& strPrice, bool bSelling,
                                     const string& strLifespan)
{
    // NOTE: The top half of this function has nothing to do with placing a new
    // offer.
    // Instead, as a convenience for knotwork, it first checks to see if there
    // are any
    // existing offers within certain parameters based on this new one, and
    // removes them
    // if so. Only then, after that is done, does it actually place the new
    // offer.
    // (Meaning: most of the code you see here at first is not actually
    // necessary for
    // placing offers, but was done at the request of a server operator.)
    string strLocation = "details_create_offer";

    string strMyNymID = OTAPI_Wrap::GetAccountWallet_NymID(MyAcct);
    string strMyNymID2 = OTAPI_Wrap::GetAccountWallet_NymID(HisAcct);

    if (!VerifyStringVal(strMyNymID) || !VerifyStringVal(strMyNymID2) ||
        (strMyNymID != strMyNymID2)) {
        otOut << "You must supply myacct as your asset account, and hisacct as "
                 "your currency account.\n";
        otOut << "Also, both accounts must be owned by the same Nym (you.)\n\n";
        return -1;
    }

    string strMyServerID = OTAPI_Wrap::GetAccountWallet_ServerID(MyAcct);
    string strMyServerID2 = OTAPI_Wrap::GetAccountWallet_ServerID(HisAcct);

    if (!VerifyStringVal(strMyServerID) || !VerifyStringVal(strMyServerID2) ||
        (strMyServerID != strMyServerID2)) {
        otOut << "Both accounts must be on the same server.\n";
        return -1;
    }
    // Below this point we're safe to use Server.

    // We're about to load up the Nym's market offers so we can see if there are
    // any we should cancel before placing this new offer.
    //
    // Therefore, I'm going to download the Nym's market offers before loading
    // them
    // up, in case we don't even have the current list of them.

    OT_Command::details_get_nym_market_offers(strMyServerID, strMyNymID);

    /*
    me: How about this — when you do "opentxs newoffer" I can alter that
    script to automatically cancel any sell offers for a lower amount
    than my new buy offer, if they're on the same market at the same scale.
    and vice versa. Vice versa meaning, cancel any bid offers for a higher
    amount than my new sell offer.

    knotwork: yeah that would work.

    So when placing a buy offer, check all the other offers I already have at
    the same scale,
    same asset and currency ID. (That is, the same "market" as denoted by
    strMapKey in "opentxs showmyoffers")
    For each, see if it's a sell offer and if so, if the amount is lower than
    the amount on
    the new buy offer, then cancel that sell offer from the market. (Because I
    don't want to buy-high, sell low.)

    Similarly, if placing a sell offer, then check all the other offers I
    already have at the
    same scale, same asset and currency ID, (the same "market" as denoted by
    strMapKey....) For
    each, see if it's a buy offer and if so, if the amount is higher than the
    amount of my new
    sell offer, then cancel that buy offer from the market. (Because I don't
    want some old buy offer
    for $10 laying around for the same stock that I'm SELLING for $8! If I dump
    100 shares, I'll receive
    $800--I don't want my software to automatically turn around and BUY those
    same shares again for $1000!
    That would be a $200 loss.)

    This is done here:
    */

    OTDB::OfferListNym* offerListPtr = loadNymOffers(strMyServerID, strMyNymID);
    OTDB::OfferListNym& offerList = *offerListPtr;

    if (!offerListPtr) {
        otOut << strLocation << ": Unable to load up a (nym) offerList from "
                                "local storage. Probably doesn't exist.\n";
    }
    else {
        // LOOP THROUGH THE OFFERS and sort them into a map_of_maps, key is:
        // scale-assetID-currencyID
        // the value for each key is a sub-map, with the key: transaction ID and
        // value: the offer data itself.
        int32_t nCount = offerList.GetOfferDataNymCount();
        if (nCount > 0) {
            MapOfMaps* map_of_maps = convert_offerlist_to_maps(offerList);

            if (nullptr == map_of_maps) {
                otOut << strLocation << ": Unable to convert offer list to map "
                                        "of offers. Perhaps it's empty?\n";
            }
            else {
                // find_strange_offers is called for each offer, for this nym,
                // as it iterates through the maps. When it's done,
                // extra_vals.the_vector
                // will contain a vector of all the transaction numbers for
                // offers that we
                // should cancel, before placing the new offer. (Such as an
                // offer to sell for
                // 30 clams when our new offer buys for 40...)
                the_lambda_struct extra_vals;

                extra_vals.the_asset_acct = MyAcct;
                extra_vals.the_currency_acct = HisAcct;
                extra_vals.the_scale = strScale;
                extra_vals.the_price = strPrice;
                extra_vals.bSelling = bSelling;

                int32_t nIterated = iterate_nymoffers_maps(
                    *map_of_maps, find_strange_offers, extra_vals);

                if (-1 == nIterated) {
                    otOut << strLocation
                          << ": Error trying to iterate nym's offers.\n";
                    return -1;
                }

                // Okay -- if there are any offers we need to cancel,
                // extra_vals.the_vector now contains
                // the transaction number for each one. Let's remove them from
                // the market before
                // starting up the new offer...

                if (extra_vals.the_vector.size() > 0) {
                    otOut << strLocation
                          << ": FYI, about to cancel at least one market "
                             "offer, before placing the new one, due to price "
                             "inconsistencies between the two...\n";
                }

                for (size_t i = 0; i < extra_vals.the_vector.size(); i++) {
                    otOut
                        << strLocation
                        << ": Canceling market offer with transaction number: "
                        << extra_vals.the_vector[i] << "\n";

                    OT_Command::details_kill_offer(strMyServerID, strMyNymID,
                                                   MyAcct,
                                                   extra_vals.the_vector[i]);
                }
                extra_vals.the_vector.clear();
            }
        }
        else {
            otOut << strLocation << ": FYI, there don't seem to be any "
                                    "existing offers for this nym, so I won't "
                                    "be erasing any older ones.\n";
        }
    }

    // OKAY! Now that we've cleaned out any undesirable offers, let's place the
    // the offer itself!

    string strResponse = MadeEasy::create_market_offer(
        MyAcct, HisAcct, strScale, strMinIncrement, strQuantity, strPrice,
        bSelling, strLifespan, "", "0");
    string strAttempt = "create_market_offer";
    int32_t nInterpretReply = InterpretTransactionMsgReply(
        strMyServerID, strMyNymID, MyAcct, strAttempt, strResponse);

    if (1 == nInterpretReply) {
        otOut << "Server response (" << strAttempt
              << "): SUCCESS placing market offer!\n\n";
    }

    return nInterpretReply;
}

OT_COMMANDS_OT int32_t OT_Command::mainNewOffer()
{
    otOut << "Usage:   newoffer --myacct <YOUR_ASSET_ACCT> --hisacct "
             "<YOUR_CURRENCY_ACCT>\n\n Optional: --args \"type <bid|ask> scale "
             "1 quantity 100 price 101\"\n Optional: --args \"lifespan 86400\" "
             "  (in seconds: 86400 is 1 day--the default.)\n\nWARNING: a price "
             "of 0 is a market order, which means 'purchase/sell at ANY "
             "price'\n\n";

    if (!VerifyExists("MyAcct") || !VerifyExists("HisAcct")) return -1;

    string strScale = "0";        // must be 1, 10, 100, etc
    string strMinIncrement = "0"; // must be 1, 2, 3, etc
    string strQuantity = "0";     // must be >= 1
    string strPrice = "0";        // must be >= 0
    string strType = "";          // must be bid or ask
    string strLifespan = "0";

    string strDefaultScale = "1";        // must be 1, 10, 100, etc
    string strDefaultMinIncrement = "1"; // must be 1, 2, 3, etc
    string strDefaultQuantity = "100";   // must be >= 1
    string strDefaultPrice = "101";      // must be >= 1
    string strDefaultType = "bid";       // must be bid or ask
    string strDefaultLifespan = "86400"; // even if we put "0" here, OT
                                         // would internally default this to
                                         // 86400.

    // If custom arguments have been passed on the command line,
    // then grab them and use them instead of asking the user to enter them
    // at the command line.
    if (VerifyExists("Args")) {
        string strNewScale = OT_CLI_GetValueByKey(Args, "scale");
        string strNewMinInc = OT_CLI_GetValueByKey(Args, "min_inc");
        string strNewQuantity = OT_CLI_GetValueByKey(Args, "quantity");
        string strNewPrice = OT_CLI_GetValueByKey(Args, "price");
        string strNewType = OT_CLI_GetValueByKey(Args, "type");
        string strNewLifespan = OT_CLI_GetValueByKey(Args, "lifespan");

        // Set the values based on the custom arguments, for those found.
        if (VerifyStringVal(strNewScale)) {
            strScale = strNewScale;
        }
        if (VerifyStringVal(strNewMinInc)) {
            strMinIncrement = strNewMinInc;
        }
        if (VerifyStringVal(strNewQuantity)) {
            strQuantity = strNewQuantity;
        }
        if (VerifyStringVal(strNewPrice)) {
            strPrice = strNewPrice;
        }
        if (VerifyStringVal(strNewType)) {
            strType = strNewType;
        }
        if (VerifyStringVal(strNewLifespan)) {
            strLifespan = strNewLifespan;
        }
    }

    // If the offer parameters aren't satisfied yet, then
    // ask the user to supply them at the command line.
    if (!VerifyStringVal(strScale) || (std::stol(strScale) < 1)) {
        otOut << "Enter the market scale (1, 10, 100, etc)[" << strDefaultScale
              << "]: ";
        strScale = OT_CLI_ReadLine();
    }
    if (!VerifyStringVal(strMinIncrement) || (std::stol(strMinIncrement) < 1)) {
        otOut << "Enter the minimum increment[" << strDefaultMinIncrement
              << "]: ";
        strMinIncrement = OT_CLI_ReadLine();
    }
    if (!VerifyStringVal(strQuantity) || (std::stol(strQuantity) < 1)) {
        otOut << "Enter the quantity being purchased/sold["
              << strDefaultQuantity << "]: ";
        strQuantity = OT_CLI_ReadLine();
    }
    if (!VerifyStringVal(strPrice) || (std::stol(strPrice) < 0)) {
        otOut << "Enter the price per scale[" << strDefaultPrice << "]: ";
        strPrice = OT_CLI_ReadLine();
    }
    if (!VerifyStringVal(strType) ||
        ((strType != "bid") && (strType != "ask"))) {
        otOut << "Enter the order type (bid/ask) [" << strDefaultType << "]: ";
        strType = OT_CLI_ReadLine();
    }
    if (!VerifyStringVal(strLifespan) || (std::stol(strLifespan) < 1)) {
        otOut << "(1 hour == 3600, 1 day == 86400)\n Enter the lifespan of "
                 "the offer, in seconds[" << strDefaultLifespan << "]: ";
        strLifespan = OT_CLI_ReadLine();
    }

    if (!VerifyStringVal(strScale) || (std::stol(strScale) < 1)) {
        strScale = strDefaultScale;
    }
    if (!VerifyStringVal(strMinIncrement) || (std::stol(strMinIncrement) < 1)) {
        strMinIncrement = strDefaultMinIncrement;
    }
    if (!VerifyStringVal(strQuantity) || (std::stol(strQuantity) < 1)) {
        strQuantity = strDefaultQuantity;
    }
    if (!VerifyStringVal(strPrice) || (std::stol(strPrice) < 1)) {
        strPrice = strDefaultPrice;
    }
    if (!VerifyStringVal(strType) ||
        ((strType != "bid") && (strType != "ask"))) {
        strType = strDefaultType;
    }
    if (!VerifyStringVal(strLifespan) || (std::stol(strLifespan) < 1)) {
        strLifespan = strDefaultLifespan;
    }

    return details_create_offer(strScale, strMinIncrement, strQuantity,
                                strPrice, strType != "bid", strLifespan);
}

OT_COMMANDS_OT int32_t OT_Command::mainNewServer()
{
    if (!VerifyExists("MyNym")) return -1;

    otOut << "Please enter the XML contents for the contract, followed by "
             "an EOF or a ~ by itself on a blank line:\n\n";
    string strXML = OT_CLI_ReadUntilEOF();

    if (!VerifyStringVal(strXML)) return -1;

    string strContractID = OTAPI_Wrap::CreateServerContract(MyNym, strXML);

    if (!VerifyStringVal(strContractID)) return -1;

    otOut << "-------------------------------------------\nNew "
             "server ID: " << strContractID << "\n\n";

    string strContract = OTAPI_Wrap::GetServer_Contract(strContractID);

    if (!VerifyStringVal(strContract)) return -1;

    otOut << "-------------------------------------------\nNew "
             "server contract:\n\n";

    cout << strContract << "\n";

    otOut << "\n\n";

    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainNewAsset()
{
    if (!VerifyExists("MyNym")) return -1;

    otOut << "Please enter the XML contents for the contract, followed by "
             "an EOF or a ~ by itself on a blank line:\n\n";
    string strXML = OT_CLI_ReadUntilEOF();

    if (!VerifyStringVal(strXML)) return -1;

    string strContractID = OTAPI_Wrap::CreateAssetContract(MyNym, strXML);

    if (!VerifyStringVal(strContractID)) return -1;

    otOut << "-------------------------------------------\nNew "
             "asset ID: " << strContractID << "\n\n";

    string strContract = OTAPI_Wrap::GetAssetType_Contract(strContractID);

    if (!VerifyStringVal(strContract)) return -1;

    otOut << "-------------------------------------------\nNew "
             "asset contract:\n\n";

    cout << strContract << "\n";

    otOut << "\n\n";

    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainNewAccount()
{
    // Just to show how easy it is now, let's try a "create_asset_acct" message.
    // (It sends a NymID to the server, and downloads that Nym's public key.)

    if (!VerifyExists("Server") || !VerifyExists("MyNym") ||
        !VerifyExists("MyPurse"))
        return -1;

    if (!OTAPI_Wrap::IsNym_RegisteredAtServer(MyNym, Server)) {
        // If the Nym's not registered at the server, then register him
        // first.
        OT_Command::mainRegisterNym();
    }

    string strResponse = MadeEasy::create_asset_acct(Server, MyNym, MyPurse);
    if (1 != VerifyMessageSuccess(strResponse)) {
        otOut << "\n\ncreate_asset_acct: Failed.\n\n";
        return -1;
    }

    otOut << "Success in create_asset_acct! Server response:\n\n";
    cout << strResponse << "\n";
    otOut << "\n (Success in create_asset_acct)\n\n";
    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainAddSignature()
{
    // SignContract erases all signatures and affixes a new one alone.
    // But AddSignature, on the other hand, leaves all signatures in place, and
    // simply adds yours to the list.

    if (!VerifyExists("MyNym")) return -1;

    otOut << "Please enter an already-signed contract you wish to add your "
             "signature to, followed by an EOF or a ~ by itself on a blank "
             "line:\n\n";
    string strInput = OT_CLI_ReadUntilEOF();

    otOut << "\n\n You entered:\n" << strInput << "\n\n";

    string strOutput = OTAPI_Wrap::AddSignature(MyNym, strInput);

    otOut << "-------------------------------------------\nSigned:\n\n";

    cout << strOutput << "\n";

    otOut << "\n\n";

    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainSignContract()
{
    // SignContract erases all signatures and affixes a new one alone.
    // But AddSignature, on the other hand, leaves all signatures in place, and
    // simply adds yours to the list.

    otOut << "Usage:   signcontract \n Optionally, you may specify a contract "
             "type:  signcontract --args \"type LEDGER\"\nIn that example, the "
             "output would start with the bookend: -----BEGIN OT SIGNED "
             "LEDGER-----\n(You don't need to specify the type if the bookend "
             "is already present on the input string.)\n\n";

    if (!VerifyExists("MyNym")) return -1;

    otOut << "Please enter a contract to be signed, followed by an EOF or "
             "a ~ by itself on a blank line:\n\n";
    string strInput = OT_CLI_ReadUntilEOF();

    otOut << "\n\n You entered:\n" << strInput << "\n\n";

    string strOutput = OTAPI_Wrap::SignContract(MyNym, strInput);

    if (!VerifyStringVal(strOutput)) {
        // Maybe we need to flat sign (maybe it wasn't already a signed
        // contract...)
        string strContractType;

        if (VerifyExists("Args")) {
            strContractType = OT_CLI_GetValueByKey(Args, "type");
        }

        if (VerifyStringVal(strContractType)) {
            otOut << "A properly-formed-and-signed contract was not "
                     "provided, but a 'type' was... so we'll try "
                     "flatsigning the input text...\n\n";
            strOutput = OTAPI_Wrap::FlatSign(MyNym, strInput, strContractType);
        }
        else {
            otOut << "A properly-formed-and-signed contract was not "
                     "provided, and neither was a 'type' in order to form "
                     "one. (Failure.)\n";
            return -1;
        }
    }

    int32_t nReturnVal = -1;

    if (VerifyStringVal(strOutput)) {

        otOut << "-------------------------------------------\nSigned:\n\n";

        cout << strOutput << "\n";

        nReturnVal = 1;
    }
    else {
        otOut << "\nSorry, but OT was unable to sign. Oh well.";
    }

    otOut << "\n\n";

    return nReturnVal;
}

OT_COMMANDS_OT int32_t OT_Command::details_kill_offer(const string& strServerID,
                                                      const string& strNymID,
                                                      const string& strAcctID,
                                                      const string& strTransNum)
{
    // Just to show how easy it is now, let's try a "kill_market_offer" message.
    // (It kills a market offer.)

    if (!VerifyStringVal(strServerID) || !VerifyStringVal(strNymID) ||
        !VerifyStringVal(strAcctID) || !VerifyStringVal(strTransNum))
        return -1;

    string strResponse = MadeEasy::kill_market_offer(strServerID, strNymID,
                                                     strAcctID, strTransNum);
    if (1 != VerifyMessageSuccess(strResponse)) {
        otOut << "\n\n killoffer: Failed.\n";
    }
    else if (1 != VerifyMsgBalanceAgrmntSuccess(strServerID, strNymID,
                                                  strAcctID, strResponse)) {
        otOut << "\n\n killoffer: Balance agreement failed.\n";
    }
    else if (1 != VerifyMsgTrnxSuccess(strServerID, strNymID, strAcctID,
                                         strResponse)) {
        otOut << "\n\n killoffer: Balance agreement succeeded, but "
                 "transaction failed.\n";
    }
    else {
        otOut << "\n\nSuccess in killoffer! Server response:\n\n";
        cout << strResponse << "\n";
        otOut << "\n";
        return 1;
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainKillOffer()
{

    otOut << "\n\n FYI, used for killing an active market offer.\nUSAGE: "
             "killoffer --args \"transnum <transaction_number>\"\n\n";

    // Just to show how easy it is now, let's try a "kill_market_offer" message.
    // (It kills a market offer.)

    if (!VerifyExists("Server") || !VerifyExists("MyNym") ||
        !VerifyExists("MyAcct") || !VerifyExists("Args"))
        return -1;

    string strTransactionNum = OT_CLI_GetValueByKey(Args, "transnum");

    if (!VerifyStringVal(strTransactionNum)) {
        otOut << "\n\n\nYou need to provide a transaction number...\n\n";
    }
    else {
        return details_kill_offer(Server, MyNym, MyAcct, strTransactionNum);
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainKillPlan()
{
    otOut << "\n\n FYI, used for stopping an active payment plan.\nUSAGE: "
             "killplan --args \"transnum <transaction_number>\"\n\n";

    // Just to show how easy it is now, let's try a "kill_payment_plan" message.
    // It kills an active (already running) payment plan.

    if (!VerifyExists("Server") || !VerifyExists("MyNym") ||
        !VerifyExists("MyAcct") || !VerifyExists("Args"))
        return -1;

    string strTransactionNum = OT_CLI_GetValueByKey(Args, "transnum");

    if (!VerifyStringVal(strTransactionNum)) {
        otOut << "\n\n\nYou need to provide a transaction number...\n\n";
        return -1;
    }

    string strResponse =
        MadeEasy::kill_payment_plan(Server, MyNym, MyAcct, strTransactionNum);
    if (1 != VerifyMessageSuccess(strResponse)) {
        otOut << "\n\nkill_payment_plan: Failed.\n";
    }
    else if (1 != VerifyMsgBalanceAgrmntSuccess(Server, MyNym, MyAcct,
                                                  strResponse)) {
        otOut << "\n\nkill_payment_plan: Balance agreement failed.\n";
    }
    else if (1 != VerifyMsgTrnxSuccess(Server, MyNym, MyAcct, strResponse)) {
        otOut << "\n\nkill_payment_plan: Balance agreement succeeded, "
                 "but transaction failed.\n";
    }
    else {
        otOut << "\n\nSuccess in kill_payment_plan! Server response:\n\n";
        cout << strResponse << "\n";
        otOut << "\n\n";
        return 1;
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainVerifySignature()
{
    if (VerifyExists("HisNym")) {
        otOut << "Please enter a contract you wish to verify with HisNym, "
                 "followed by an EOF or a ~ by itself on a blank line:\n\n";
        string strInput = OT_CLI_ReadUntilEOF();

        otOut << "\n\n--------------------------------------\n You entered:\n"
              << strInput << "\n\n";

        if (OTAPI_Wrap::VerifySignature(HisNym, strInput)) {
            cout << "\n\n *** Verified! ***\n\n\n";
            return 1;
        }
        else {
            cout << "Failed!\n\n\n";
        }
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainShowNyms()
{
    cout << "------------------------------------------------------------------"
            "\n";
    cout << " ** NYMS: \n\n";

    int32_t nNymCount = OTAPI_Wrap::GetNymCount();

    for (int32_t i = 0; i < nNymCount; ++i) {
        string strIndex = std::to_string(i);
        string strID = OTAPI_Wrap::GetNym_ID(i);
        string strName = OTAPI_Wrap::GetNym_Name(strID);

        cout << strIndex << ": " << strID << " ---  " << strName << "\n";
    }

    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainShowServers()
{
    cout << "------------------------------------------------------------------"
            "\n";
    cout << " ** SERVERS: \n\n";
    int32_t nServerCount = OTAPI_Wrap::GetServerCount();

    for (int32_t i = 0; i < nServerCount; ++i) {
        string strIndex = std::to_string(i);
        string strID = OTAPI_Wrap::GetServer_ID(i);
        string strName = OTAPI_Wrap::GetServer_Name(strID);

        cout << strIndex << ": " << strID << " ---  " << strName << "\n";
    }

    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainShowAssets()
{
    cout << "------------------------------------------------------------------"
            "\n";
    cout << " ** ASSET TYPES: \n\n";
    int32_t nAssetTypeCount = OTAPI_Wrap::GetAssetTypeCount();

    for (int32_t i = 0; i < nAssetTypeCount; ++i) {
        string strID = OTAPI_Wrap::GetAssetType_ID(i);
        string strName = OTAPI_Wrap::GetAssetType_Name(strID);

        cout << strID << " ---  " << strName << "\n";
    }

    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainShowAccounts()
{
    cout << "------------------------------------------------------------------"
            "\n";
    cout << " ** ACCOUNTS: \n\n";

    int32_t nAccountCount = OTAPI_Wrap::GetAccountCount();

    for (int32_t i = 0; i < nAccountCount; ++i) {
        if ((i > 0) && (i != (nAccountCount))) {
            cout << "-------------------------------------\n";
        }
        string strID = OTAPI_Wrap::GetAccountWallet_ID(i);
        string strStatAcct = MadeEasy::stat_asset_account(strID);
        bool bSuccess = VerifyStringVal(strStatAcct);
        if (bSuccess) {
            cout << strStatAcct << "\n";
        }
        else {
            cout << "Error trying to stat an asset account: " << strID << "\n";
        }

        cout << "\n";
    }
    cout << "------------------------------------------------------------------"
            "\n";

    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainShowWallet()
{
    cout << "\n";

    OT_Command::mainShowNyms();
    OT_Command::mainShowServers();
    OT_Command::mainShowAssets();
    OT_Command::mainShowAccounts();

    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::details_stat_account(const string& strID)
{

    otOut << "\n\n-------------------------------------------------------------"
             "-----\n";

    string strStatAcct = MadeEasy::stat_asset_account(strID);

    bool bSuccess = VerifyStringVal(strStatAcct);
    int32_t nSuccess = (bSuccess ? 1 : -1);

    if (bSuccess) {
        cout << strStatAcct << "\n";
    }
    else {
        cout << "Error trying to stat an asset account: " << strID << "\n\n";
    }

    otOut << "\n";

    return nSuccess;
}

OT_COMMANDS_OT int32_t OT_Command::mainShowAccount()
{
    if (VerifyExists("MyAcct")) {
        return details_stat_account(MyAcct);
    }
    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::details_account_balance(const string& strID)
{
    string strAssetID = OTAPI_Wrap::GetAccountWallet_AssetTypeID(strID);
    if (!VerifyStringVal(strAssetID)) {
        otOut << "\ndetails_account_balance: Cannot cannot determine asset "
                 "type for: " << strID << "\n";
        return -1;
    }

    string strName = OTAPI_Wrap::GetAccountWallet_Name(strID);
    int64_t lBalance = OTAPI_Wrap::GetAccountWallet_Balance(strID);

    otOut << "\n    Balance: ";
    cout << OTAPI_Wrap::FormatAmount(strAssetID, lBalance) << "\n";
    otOut << strID << "   (" << strName << ")\n\n";

    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainShowBalance()
{
    if (VerifyExists("MyAcct")) {
        return details_account_balance(MyAcct);
    }
    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::details_nym_stat(const string& strID)
{
    string strStats = OTAPI_Wrap::GetNym_Stats(strID);
    if (!VerifyStringVal(strStats)) {
        return -1;
    }

    cout << strStats << "\n";
    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainShowNym()
{
    if (VerifyExists("MyNym")) {
        return details_nym_stat(MyNym);
    }
    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainShowMint()
{
    if (VerifyExists("Server") && VerifyExists("MyNym") &&
        VerifyExists("MyPurse")) {

        string strMint =
            MadeEasy::load_or_retrieve_mint(Server, MyNym, MyPurse);
        if (!VerifyStringVal(strMint)) {
            otOut << "\n\n load_or_retrieve_mint: Failed.\n\n";
            return -1;
        }
        else {
            otOut << "\n\n";
            cout << strMint << "\n";
            otOut << "\n\n";
            return 1;
        }
    }

    return -1;
}

// Creates a new Nym and adds it to the wallet.
// (And sets the display name for the new Nym, in the wallet.)
// Prints the new NymID to stdout.
//
// Returns 1 for success, 0 for failure.
//
OT_COMMANDS_OT int32_t
    OT_Command::details_create_nym(int32_t nKeybits, const string& strName,
                                   const string& strSourceForNymID,
                                   const string& strAltLocation)
{
    string strNymID =
        MadeEasy::create_nym(nKeybits, strSourceForNymID, strAltLocation);

    if (!VerifyStringVal(strNymID)) {
        otOut << "details_create_nym: Failed in "
                 "OT_ME::create_nym(keybits == " << nKeybits << ")\n";
        return -1;
    }
    otOut << "Success creating! " << nKeybits << " keybits, new ID: ";
    cout << strNymID << "\n";
    otOut << "\n";

    bool bSetName = OTAPI_Wrap::SetNym_Name(strNymID, strNymID, strName);
    if (!bSetName) {
        otOut << "Failed in OT_API_SetNym_Name(name == " << strName << ")\n";
        return -1;
    }

    otOut << "Success setting name to: " << strName << "\n\n";
    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainNewNym()
{
    otOut << "Usage:   newnym --args \"keybits 1024 name \\\"Bob's New "
             "Nym\\\"\"  \nOptional: newnym --args \"source "
             "\\\"http://test.com/credential_IDs\\\" \"  \nFYI, a source can "
             "be a URL, a Bitcoin address, a Namecoin address, a public "
             "key,\nor the unique DN info from a traditionally-issued cert. "
             "Hashing the source should\n produce the NymID. Also, the source "
             "should always (somehow) validate the\ncredential IDs, if they "
             "are to be trusted for their purported Nym.\nAnother optional "
             "parameter is 'altlocation' which, in the case of DN info as a "
             "source, would be the \ndownload location where a Cert should be "
             "found with that DN info, or a PKI where the Cert can be "
             "found.\n\nNOTE: If you leave the source BLANK, then OT will just "
             "generate a public key to serve as the source. The\npublic key "
             "will be hashed to form the NymID, and all credentials for that "
             "Nym will need to be signed by\nthe corresponding private key. "
             "That's the only way they can be 'verified by their source.'\n\n";

    int32_t nKeybits = 1024;
    string strName = "New Nym";
    string strSourceForNymID = "";
    string strAltLocation = "";

    if (VerifyExists("Args", false)) {
        string strKeybits = OT_CLI_GetValueByKey(Args, "keybits");
        string strNewName = OT_CLI_GetValueByKey(Args, "name");
        string strNewSource = OT_CLI_GetValueByKey(Args, "source");
        string strNewLocation = OT_CLI_GetValueByKey(Args, "location");

        if (VerifyStringVal(strKeybits) && (std::stol(strKeybits) > 0)) {
            nKeybits = std::stol(strKeybits);
        }

        if (VerifyStringVal(strNewName)) {
            strName = strNewName;
        }

        if (VerifyStringVal(strNewSource)) {
            strSourceForNymID = strNewSource;
        }

        if (VerifyStringVal(strNewLocation)) {
            strAltLocation = strNewLocation;
        }
    }

    return details_create_nym(nKeybits, strName, strSourceForNymID,
                              strAltLocation);
}

/*
call OTAPI_Wrap::LoadInbox() to load the inbox ledger from local storage.

During this time, your user has the opportunity to peruse the inbox, and to
decide which transactions therein he wishes to accept or reject. Usually the
inbox is display on the screen, then the user selects various items to accept or
reject, and then the user clicks “Process Inbox” and then you do this:
Then call OTAPI_Wrap::Ledger_CreateResponse() in order to create a ‘response’
ledger for that inbox, which will be sent to the server to signal your responses
to the various inbox transactions.
Then call OTAPI_Wrap::Ledger_GetCount() (pass it the inbox) to find out how many
transactions are inside of it. Use that count to LOOP through them…
Use OTAPI_Wrap::Ledger_GetTransactionByIndex() to grab each transaction as you
iterate through the inbox. (There are various introspection functions you can
use in the API here if you wish to display the inbox items on the screen for the
user…)
Next call OTAPI_Wrap::Transaction_CreateResponse() for each transaction in the
inbox, to create a response to it, accepting or rejecting it. This function
creates the response and adds it to the response ledger.
Next, call OTAPI_Wrap::Ledger_FinalizeResponse() which will create a Balance
Agreement for the ledger.
Finally, call OTAPI_Wrap::processInbox() to send your message to the server and
process the various items.

If the message was successful, then use
OTAPI_Wrap::Message_GetBalanceAgreementSuccess() and
OTAPI_Wrap::Message_GetTransactionSuccess() as described above in the deposit
cash instructions.
*/

// Done:  add options here for accept transfers, accept receipts, and accept
// all.
// Done:  Then basically make a version for the payments inbox for accept
// payments, accept invoices, and accept all.
//
// (Accepting payments can basically be automated, but accepting invoices
// requires user permission.)
//
// Therefore add:
//   acceptmoney    -- This accepts all incoming transfers and incoming payments
//                     (Not receipts or invoices) for any designated accounts
// and nyms.
//   acceptreceipts -- Accepts all inbox receipts (not transfers.)
//   acceptinvoices -- Accepts all invoices (not payments.)
//   acceptall      -- All of the above.
//
// Todo: Make the above functions also work with specific indices (vs "all")
//

//
// PROCESS INBOX, ACCEPTING ALL ITEMS WITHIN...
//
// Load an asset account's inbox from local storage and iterate through
// the items inside, and fire off a server message accepting them all.
//
// nItemType  == 0 for all, 1 for transfers only, 2 for receipts only.
// strIndices == "" for "all indices"
//
OT_COMMANDS_OT int32_t OT_Command::accept_inbox_items(const string& strMyAcctID,
                                                      int32_t nItemType,
                                                      const string& strIndices)
{
    Utility MsgUtil;

    string strMyNymID = OTAPI_Wrap::GetAccountWallet_NymID(strMyAcctID);
    if (!VerifyStringVal(strMyNymID)) {
        otOut
            << "Failure: Unable to find NymID based on the specified account ( "
            << strMyAcctID << " ).\n";
        return -1;
    }
    if (VerifyExists("MyNym", false) && (MyNym != strMyNymID)) {
        otOut << "Try again: MyNym is specified, but it's not the owner of the "
                 "specified account ( " << strMyAcctID << " ).\n";
        otOut << "To override with the nym for this account, add: --mynym "
              << strMyNymID << " \n\n";
        return -1;
    }

    string strServerID = OTAPI_Wrap::GetAccountWallet_ServerID(strMyAcctID);
    if (!VerifyStringVal(strServerID)) {
        otOut << "Failure: Unable to find Server ID based on the specified "
                 "account ( " << strMyAcctID << " ).\n";
        return -1;
    }
    if (VerifyExists("Server", false) && (Server != strServerID)) {
        otOut << "Try again: Server is specified, but it's not the server for "
                 "the specified account ( " << strMyAcctID << " ).\n";
        otOut << "To override with the server for this account, add: --server "
              << strServerID << " \n\n";
        return -1;
    }

    // User may have already chosen indices (passed in) so we don't want to
    // re-download the inbox unless we HAVE to. But if the hash has changed,
    // that's
    // one clear-cut case where we _do_ have to. Otherwise our balance agreement
    // will fail anyway. So hopefully we can let OT "be smart about it" here
    // instead
    // of just forcing it to download every time even when unnecessary.
    if (!MsgUtil.getIntermediaryFiles(strServerID, strMyNymID,
                                      strMyAcctID)) // boolean
    {
        otOut << "Unable to download necessary intermediary files for this "
                 "inbox/account. (Failure.)\n";
        return -1;
    }

    // Make sure we have at least one transaction number (to process the inbox
    // with.)
    //
    // NOTE: Normally we don't have to do this, because the high-level API is
    // smart
    // enough, when sending server transaction requests, to grab new transaction
    // numbers
    // if it is running low.
    // But in this case, we need the numbers available BEFORE sending the
    // transaction
    // request, because the call to OTAPI_Wrap::Ledger_CreateResponse is where
    // the number
    // is first needed, and that call is made int64_t before the server
    // transaction request
    // is actually sent.
    if (!MadeEasy::insure_enough_nums(10, strServerID, strMyNymID)) {
        return -1;
    }

    string strInbox =
        OTAPI_Wrap::LoadInbox(strServerID, strMyNymID, strMyAcctID);

    if (!VerifyStringVal(strInbox)) {
        otOut << "\n\n OT_API_LoadInbox: Failed.\n\n";
        return -1;
    }

    otOut << "\n\n";

    int32_t nCount = OTAPI_Wrap::Ledger_GetCount(strServerID, strMyNymID,
                                                 strMyAcctID, strInbox);
    if (nCount <= 0) {
        otOut << "The asset account inbox is empty.\n\n";
        return 0;
    }

    // NOTE!!! DO **NOT** create the response ledger until the FIRST
    // iteration of the below loop that actually
    // creates a transaction response! If that "first iteration" never
    // comes (due to receipts being skipped, etc)
    // then OTAPI_Wrap::Transaction_CreateResponse will never get
    // called, and therefore Ledger_CreateResponse should
    // also not be called, either. (Nor should
    // OTAPI_Wrap::Ledger_FinalizeResponse, etc.)
    string strResponseLEDGER = "";

    int32_t nIndicesCount =
        VerifyStringVal(strIndices) ? OTAPI_Wrap::NumList_Count(strIndices) : 0;

    for (int32_t nIndex = 0; nIndex < nCount; ++nIndex) {
        string strTrans = OTAPI_Wrap::Ledger_GetTransactionByIndex(
            strServerID, strMyNymID, strMyAcctID, strInbox, nIndex);

        // nItemType  == 0 for all, 1 for transfers only, 2 for receipts
        // only.
        // strIndices == "" for "all indices"
        if (nItemType > 0) // 0 means "all", so we don't have to skip
                           // anything based on type, if it's 0.
        {
            string strTransType = OTAPI_Wrap::Transaction_GetType(
                strServerID, strMyNymID, strMyAcctID, strTrans);

            // incoming transfer
            if ("pending" == strTransType && 1 != nItemType) {
                // if it IS an incoming transfer, but we're doing
                // receipts, then skip it.
                continue;
            }
            // receipt
            if ("pending" != strTransType && 2 != nItemType) {
                // if it is NOT an incoming transfer, then it's a
                // receipt. If we're not doing receipts, then skip it.
                continue;
            }
        }

        // - If NO indices are specified, process them ALL.
        //
        // - If indices are specified, but the current index is not on
        //   that list, then continue...
        if ((nIndicesCount > 0) &&
            !OTAPI_Wrap::NumList_VerifyQuery(strIndices,
                                             std::to_string(nIndex))) {
            continue;
        }

        // By this point we know we actually have to call
        // OTAPI_Wrap::Transaction_CreateResponse
        // Therefore, if OTAPI_Wrap::Ledger_CreateResponse has
        // not yet been called (which it won't
        // have been, the first time we hit this in this loop),
        // then we call it here this one
        // time, to get things started...
        if (!VerifyStringVal(strResponseLEDGER)) {
            strResponseLEDGER = OTAPI_Wrap::Ledger_CreateResponse(
                strServerID, strMyNymID, strMyAcctID, strInbox);

            if (!VerifyStringVal(strResponseLEDGER)) {
                otOut << "\n\nFailure: "
                         "OT_API_Ledger_CreateResponse "
                         "returned nullptr.\n";
                return -1;
            }
        }

        // By this point, we know the ledger response exists,
        // and we know we have to create
        // a transaction response to go inside of it, so let's
        // do that next...
        string strNEW_ResponseLEDGER = OTAPI_Wrap::Transaction_CreateResponse(
            strServerID, strMyNymID, strMyAcctID, strResponseLEDGER, strTrans,
            true);

        if (!VerifyStringVal(strNEW_ResponseLEDGER)) {
            otOut << "\n\nFailure: "
                     "OT_API_Transaction_CreateResponse "
                     "returned nullptr.\n";
            return -1;
        }
        strResponseLEDGER = strNEW_ResponseLEDGER;
    }

    if (!VerifyStringVal(strResponseLEDGER)) {
        // This means there were receipts in the box, but they were
        // skipped.
        // And after the skipping was done, there were no receipts left.
        // So we can't just say "the box is empty" because it's not. But
        // nevertheless,
        // we aren't actually processing any of them, so we return 0 AS
        // IF the box
        // had been empty. (Because this is not an error condition. Just
        // a "no op".)
        return 0;
    }

    // Below this point, we know strResponseLEDGER needs to be sent,
    // so let's finalize it.
    string strFinalizedResponse = OTAPI_Wrap::Ledger_FinalizeResponse(
        strServerID, strMyNymID, strMyAcctID, strResponseLEDGER);

    if (!VerifyStringVal(strFinalizedResponse)) {
        otOut << "\n\nFailure: OT_API_Ledger_FinalizeResponse returned "
                 "nullptr.\n";
        return -1;
    }

    string strResponse = MadeEasy::process_inbox(
        strServerID, strMyNymID, strMyAcctID, strFinalizedResponse);
    string strAttempt = "process_inbox";

    int32_t nInterpretReply = InterpretTransactionMsgReply(
        strServerID, strMyNymID, strMyAcctID, strAttempt, strResponse);

    if (1 == nInterpretReply) {
        bool bRetrieved = MadeEasy::retrieve_account(strServerID, strMyNymID,
                                                     strMyAcctID, true);

        otOut << "\n\nServer response (" << strAttempt
              << "): SUCCESS processing/accepting inbox.\n";
        otOut << (bRetrieved ? "Success" : "Failed")
              << " retrieving intermediary files for account.\n";
    }

    //
    return nInterpretReply;
}

OT_COMMANDS_OT int32_t OT_Command::mainAcceptReceipts()
{
    otOut << "Usage:   acceptreceipts --myacct FOR_ACCT --args \"indices "
             "3,6,8\"  \n (Sample indices are shown.)\nIf indices are not "
             "specified for myacct's inbox, then OT will\naccept ALL receipts "
             "(but no pending transfers) in that box.\n\n";

    if (VerifyExists("MyAcct")) {
        string strIndices = "";

        if (VerifyExists("Args", false)) {
            strIndices = OT_CLI_GetValueByKey(Args, "indices");
        }

        return accept_inbox_items(MyAcct, 2, strIndices);
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainAcceptInbox()
{
    otOut << "Usage:   acceptinbox --myacct FOR_ACCT --args \"indices 3,6,8\"  "
             "\n (Sample indices are shown.)\nIf indices are not specified for "
             "myacct's inbox, then OT will\naccept ALL transfers and receipts "
             "in that box.\n\n";

    if (!VerifyExists("MyAcct")) return -1;

    string strIndices = "";

    if (VerifyExists("Args", false)) {
        strIndices = OT_CLI_GetValueByKey(Args, "indices");
    }

    return accept_inbox_items(MyAcct, 0, strIndices);
}

OT_COMMANDS_OT int32_t OT_Command::mainAcceptTransfers()
{
    otOut << "Usage:   accepttransfers --myacct FOR_ACCT --args \"indices "
             "3,6,8\"  \n (Sample indices are shown.)\nIf indices are not "
             "specified for myacct's inbox, then OT will\naccept ALL transfers "
             "(but no receipts) in that box.\n\n";

    if (VerifyExists("MyAcct")) {
        string strIndices = "";

        if (VerifyExists("Args", false)) {
            strIndices = OT_CLI_GetValueByKey(Args, "indices");
        }

        return accept_inbox_items(MyAcct, 1, strIndices);
    }

    return -1;
}

// Accept incoming payments and transfers. (NOT receipts or invoices.)
//
OT_COMMANDS_OT int32_t OT_Command::mainAcceptMoney()
{
    otOut << "Usage:   acceptmoney --myacct INTO_ACCT\n";

    if (!VerifyExists("MyAcct")) return -1;

    string strIndices = "";

    int32_t nAcceptedTransfers = accept_inbox_items(MyAcct, 1, strIndices);

    int32_t nAcceptedPurses =
        accept_from_paymentbox(MyAcct, strIndices, "PURSE");

    int32_t nAcceptedCheques =
        accept_from_paymentbox(MyAcct, strIndices, "CHEQUE");

    // FIX: these OR's should become AND's so we can detect any failure
    if (nAcceptedTransfers >= 0 || nAcceptedPurses >= 0 ||
        nAcceptedCheques >= 0) {
        return 1;
    }

    return -1;
}

// Accept all incoming transfers, receipts, payments, and invoices.
//
OT_COMMANDS_OT int32_t OT_Command::mainAcceptAll()
{
    otOut << "Usage:   acceptall --myacct INTO_ACCT\n";

    if (VerifyExists("MyAcct")) {
        string strIndices = "";

        //  Incoming transfers and receipts (asset account inbox.)
        int32_t nAcceptedInbox = accept_inbox_items(MyAcct, 0, strIndices);

        // Incoming payments -- cheques, purses, vouchers (payments inbox for
        // nym)
        int32_t nAcceptedPurses =
            accept_from_paymentbox(MyAcct, strIndices, "PURSE");

        int32_t nAcceptedCheques =
            accept_from_paymentbox(MyAcct, strIndices, "CHEQUE");

        // Invoices LAST (so the MOST money is in the account before it starts
        // paying out.)
        int32_t nAcceptedInvoices =
            accept_from_paymentbox(MyAcct, strIndices, "INVOICE");

        if (nAcceptedInbox >= 0 && nAcceptedPurses >= 0 &&
            nAcceptedCheques >= 0 && nAcceptedInvoices >= 0) {
            return 1;
        }
    }

    return -1;
}

// returns the server response string (or null.)
// Use VerifyStringVal and/or VerifyMessageSuccess on it, for more info.
//
OT_COMMANDS_OT string OT_Command::details_check_user(const string& strServerID,
                                                     const string& strMyNymID,
                                                     const string& strHisNymID)
{
    // Just to show how easy it is now, let's try a "check_user" message.
    // (It sends a NymID to the server, and downloads that Nym's public key.)

    string strServerParam = strServerID;
    string strMyNymParam = strMyNymID;
    string strHisNymParam = strHisNymID;

    string strResponse;

    // NOTE: If this were Server, MyNym, and HisNym, then we'd have already
    // translated the partials to read IDs (in OT itself) but we don't have that
    // guarantee in this function. Maybe strServerID was passed in from user
    // input
    // directly, while inside another script. Partial IDs should still work,
    // right?
    // So here, we translate them just in case.
    if (VerifyStringVal(strServerID) && VerifyStringVal(strMyNymID) &&
        VerifyStringVal(strHisNymID)) {
        // IF we are able to resolve the HisNymId from a partial, then we
        // replace the partial with the full version.
        // (Otherwise we assume it's already a full ID and we don't mess with
        // it.)
        string strServerFromPartial =
            OTAPI_Wrap::Wallet_GetServerIDFromPartial(strServerID);

        if (VerifyStringVal(strServerFromPartial)) {
            strServerParam = strServerFromPartial;
        }

        string strHisNymFromPartial =
            OTAPI_Wrap::Wallet_GetNymIDFromPartial(strHisNymID);

        if (VerifyStringVal(strHisNymFromPartial)) {
            strHisNymParam = strHisNymFromPartial;
        }

        string strMyNymFromPartial =
            OTAPI_Wrap::Wallet_GetNymIDFromPartial(strMyNymID);

        if (VerifyStringVal(strMyNymFromPartial)) {
            strMyNymParam = strMyNymFromPartial;
        }

        strResponse =
            MadeEasy::check_user(strServerParam, strMyNymParam, strHisNymParam);
    }
    else {
        otOut << "\n details_check_user: Bad input... strServerID, strMyNymID, "
                 "or strHisNymID \n";
    }

    return strResponse;
}

OT_COMMANDS_OT int32_t OT_Command::mainCheckNym()
{
    otOut << "Usage:   checknym --mynym MY_NYM_ID --hisnym HIS_NYM_ID \n "
             "Downloads the public key for HisNym.\n\n";

    if (VerifyExists("Server") && VerifyExists("MyNym") &&
        VerifyExists("HisNym")) {
        string strResponse = details_check_user(Server, MyNym, HisNym);

        // -1 is error,
        //  0 is reply received: failure
        //  1 is reply received: success
        if (1 == VerifyMessageSuccess(strResponse)) {
            otOut << "\n\nSuccess in checknym! Server response:\n\n";
            cout << strResponse << "\n";
            otOut << "\n\n";
            return 1;
        }
        else {
            otOut << "\n\n checknym: Failed.\n\n";
        }
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::download_acct_files()
{
    string strMyNymID = OTAPI_Wrap::GetAccountWallet_NymID(MyAcct);
    if (!VerifyStringVal(strMyNymID)) {
        otOut << "Failure: Unable to find NymID based on myacct. Use: --myacct "
                 "ACCT_ID\n";
        otOut << "The designated asset account must be yours. OT will find the "
                 "Nym based on the account.\n\n";
        return -1;
    }

    bool bRetrieved =
        MadeEasy::retrieve_account(Server, strMyNymID, MyAcct, true);

    otOut << "\n\n" << (bRetrieved ? "Success" : "Failed")
          << " retrieving intermediary files for account: " << MyAcct << "\n\n";

    return bRetrieved ? 1 : -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainRefreshAccount()
{
    otOut << "Usage:   refreshaccount --server SERVER_ID --myacct "
             "YOUR_ACCT_ID\n\n";

    if (VerifyExists("Server") && VerifyExists("MyAcct")) {
        return download_acct_files();
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainRefresh()
{
    otOut << "Usage:   refresh --server SERVER_ID --mynym YOUR_NYM_ID --myacct "
             "YOUR_ACCT_ID\n\n";

    if (VerifyExists("Server") && VerifyExists("MyNym") &&
        VerifyExists("MyAcct")) {
        int32_t nSuccess = OT_Command::mainRefreshNym();

        if (-1 == nSuccess) {
            return -1;
        }

        return OT_Command::mainRefreshAccount();
    }

    return -1;
}

OT_COMMANDS_OT int32_t
    OT_Command::details_download_contract(const string& strServerID,
                                          const string& strNymID,
                                          const string& strContractID)
{
    string strRetrieved =
        MadeEasy::retrieve_contract(strServerID, strNymID, strContractID);
    int32_t nRetrieved = VerifyMessageSuccess(strRetrieved);

    string strSuccess = "Error";

    if (1 == nRetrieved) {
        strSuccess = "Success";
    }
    else if (0 == nRetrieved) {
        strSuccess = "Failed";
    }

    otOut << "\n\n " << strSuccess << " retrieving contract: " << strContractID
          << "\n\n";

    return (1 == nRetrieved) ? 1 : -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainGetContract()
{
    otOut << "Usage:   getcontract --server SERVER_ID --mynym YOUR_NYM_ID \n   "
             "                 --args \"contract_id CONTRACT_ID_HERE\"\n\n";

    string strContractID = "";

    if (VerifyExists("Server") && VerifyExists("MyNym")) {

        // If custom arguments have been passed on the command line,
        // then grab them and use them instead of asking the user to enter them
        // at the command line.
        if (VerifyExists("Args")) {
            string strNewContractID = OT_CLI_GetValueByKey(Args, "contract_id");

            // Set the values based on the custom arguments, for those found.
            if (VerifyStringVal(strNewContractID)) {
                strContractID = strNewContractID;
            }
            else {
                otOut << "\n\nMissing --args \"contract_id "
                         "CONTRACT_ID_HERE\"\n\n";
                return -1;
            }

            return details_download_contract(Server, MyNym, strContractID);
        }
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainVerifyReceipt()
{
    // SHOW INBOX
    //
    // Load an asset account's inbox from local storage and display it on the
    // screen.

    if (VerifyExists("Server") && VerifyExists("MyAcct")) {

        string strMyNymID = OTAPI_Wrap::GetAccountWallet_NymID(MyAcct);
        if (!VerifyStringVal(strMyNymID)) {
            otOut << "Failure: Unable to find NymID based on myacct. Use: "
                     "--myacct ACCT_ID\n";
            otOut << "The designated asset account must be yours. OT will find "
                     "the Nym based on the account.\n\n";
            return -1;
        }

        bool bSuccess =
            OTAPI_Wrap::VerifyAccountReceipt(Server, strMyNymID, MyAcct);
        if (!bSuccess) {
            otOut << "\n\n OT_API_VerifyAccountReceipt: Failed. Try using "
                     "refreshaccount and then try verifying again.\n\n";
            return -1;
        }
        else {
            otOut << "\n\nVerify receipt:  success!\n\n";
            return 1;
        }
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainRegisterNym()
{
    otOut << "Usage:   registernym --server SERVER_ID --mynym NYM_ID\n\n(If "
             "you don't have a NymID, then use create_nym.ot first.)\n\n";

    if (VerifyExists("Server") && VerifyExists("MyNym")) {
        string strResponse = MadeEasy::register_nym(Server, MyNym);
        int32_t nSuccess = VerifyMessageSuccess(strResponse);
        switch (nSuccess) {
        case 1:
            otOut << "\n\n Server response:\n\n";
            cout << strResponse << "\n";
            otOut << "\n\n SUCCESS in register_nym!\n(Also, success "
                     "syncronizing the request number.)\n\n";
            break;
        case 0:
            otOut << "\n\n FAILURE in register_nym! Server response:\n\n";
            cout << strResponse << "\n";
            break;
        default:
            otOut << "\n\nError in register_nym!\n";

            if (VerifyStringVal(strResponse)) {
                otOut << "Server response:\n\n";
                cout << strResponse << "\n";
            }
            break;
        }
        otOut << "\n\n";

        return nSuccess;
    }

    return -1;
}

OT_COMMANDS_OT bool OT_Command::details_refresh_nym(const string& strServerID,
                                                    const string& strMyNymID,
                                                    bool bForceDownload)
{
    bool bWasMsgSent = false;
    int32_t nGetAndProcessNymbox = MadeEasy::retrieve_nym(
        strServerID, strMyNymID, bWasMsgSent, bForceDownload);
    bool bReturnVal = false;

    switch (nGetAndProcessNymbox) {
    case 1:
        otOut << "\n\n SUCCESS in refresh nym!\n";
        bReturnVal = true;
        break;
    case 0:
        if (bWasMsgSent) {
            otOut << "\n\n FAILURE in refresh nym!\n";
        }
        else {
            otOut << "\n\n Success in refresh nym! (Skipped processing Nymbox, "
                     "since it's empty.)\n";
            bReturnVal = true;
        }
        break;
    default:
        otOut << "\n\nError in refresh nym! nGetAndProcessNymbox: "
              << nGetAndProcessNymbox << "\n";
        break;
    }

    return bReturnVal;
}

OT_COMMANDS_OT int32_t OT_Command::mainRefreshNym()
{
    otOut << "Usage:   refreshnym --server SERVER_ID --mynym NYM_ID\n\n(If you "
             "don't have a NymID, then use the newnym command first.)\n\n";

    if (VerifyExists("Server") && VerifyExists("MyNym")) {
        bool bReturnVal = details_refresh_nym(Server, MyNym, true);

        otOut << "\n";

        if (bReturnVal) {
            return 1;
        }
    }

    return -1;
}

// Note: nBoxType is 0 for Nymbox, 1 for Inbox, and 2 for Outbox.
OT_COMMANDS_OT int32_t
    OT_Command::details_download_box_receipt(const string& strID,
                                             int32_t nBoxType)
{
    string strMyNymID = MyNym;
    string strAcctID;

    if (0 == nBoxType) {
        strAcctID = MyNym;
    }
    else {
        if (!VerifyExists("MyAcct", false)) {
            otOut << "Failure: Unable to find MyAcct. Use: --myacct ACCT_ID\n";
            otOut << "The designated asset account must be yours. OT will find "
                     "the Nym based on the account.\n\n";
            return -1;
        }
        else {
            strAcctID = MyAcct;

            // (After this point, MyNym contains the NymID, and strMyNymID
            // contains
            // the NymID that corresponds to MyAcct -- they may not necessarily
            // be
            // the same Nym...)
            strMyNymID = OTAPI_Wrap::GetAccountWallet_NymID(strAcctID);
            if (!VerifyStringVal(strMyNymID)) {
                otOut << "Failure: Unable to find NymID based on myacct. Use: "
                         "--myacct ACCT_ID\n";
                otOut << "The designated asset account must be yours. OT will "
                         "find the Nym based on the account.\n\n";
                return -1;
            }

            if (strMyNymID != MyNym) {
                otOut << "Failure: Found a NymID based on myacct, but MyNym is "
                         "not the same ID.\n";
                otOut << "To avoid any confusion, please be explicit with Nym "
                         "ID:   --mynym NYM_ID \n\n";
                return -1;
            }
        }
    }

    string strResponse = MadeEasy::get_box_receipt(Server, strMyNymID,
                                                   strAcctID, nBoxType, strID);
    int32_t nInterpretReply = VerifyMessageSuccess(strResponse);

    if (1 != nInterpretReply) {
        otOut << "get_box_receipt: Failed. nInterpretReply is: "
              << nInterpretReply << "\n";
        otOut << "get_box_receipt: Perhaps that receipt is no longer in the "
                 "box?\n";
    }
    else {
        otOut << "Success in get_box_receipt! Server response:\n\n";
        cout << strResponse << "\n";
        otOut << "\n\n";
        return 1;
    }
    return nInterpretReply;
}

OT_COMMANDS_OT int32_t OT_Command::mainGetReceipt()
{
    otOut << "Usage:   getreceipt --server SERVER_ID --mynym NYM_ID\n\nAlso:   "
             " --args \"box_type BOX_TYPE_ID_HERE id "
             "TRANSACTION_ID_HERE\"\nBox types are 0 (Nymbox), 1 (Inbox), 2 "
             "(Outbox)\n\n";

    string strID;

    if (VerifyExists("Server") && VerifyExists("MyNym")) {

        // If custom arguments have been passed on the command line,
        // then grab them and use them instead of asking the user to enter them
        // at the command line.
        if (VerifyExists("Args")) {
            string strNewID = OT_CLI_GetValueByKey(Args, "id");
            string strNewType = OT_CLI_GetValueByKey(Args, "box_type");

            // Set the values based on the custom arguments, for those found.
            if (VerifyStringVal(strNewID)) {
                strID = strNewID;
            }
            else {
                otOut << "\n\nMissing --args \"id TRANSACTION_ID_HERE\"\n\n";
                return -1;
            }
            int32_t nBoxType = 1;
            if (VerifyStringVal(strNewType)) {
                nBoxType = std::stol(strNewType);
            }

            if (nBoxType < 0 || nBoxType > 2) {
                otOut << "\n\n box_type cannot be <0 or >2. Try:  --args "
                         "\"box_type 1\"\nBox types are 0 (Nymbox), 1 (Inbox), "
                         "2 (Outbox)\n\n";
                return -1;
            }

            // If Inbox or Outbox, make sure we have an acct...
            if ((nBoxType != 0) && !VerifyExists("MyAcct", false)) {
                otOut << "\n\n For inbox (1) or outbox (2) box types, need an "
                         "account ID.\nMissing: --myacct ACCT_ID_HERE \n\n";
                return -1;
            }

            return details_download_box_receipt(strID, nBoxType);
        }
    }

    return -1;
}

//
// WITHDRAW CASH
//
// (from asset account on server to cash purse on client.)
//
OT_COMMANDS_OT int32_t
    OT_Command::details_withdraw_cash(const string& strMyAcctID,
                                      int64_t lAmount)
{
    string strMyNymID = OTAPI_Wrap::GetAccountWallet_NymID(strMyAcctID);
    if (!VerifyStringVal(strMyNymID)) {
        otOut << "Failure: Unable to find NymID based on myacct. Use: --myacct "
                 "ACCT_ID\n";
        otOut << "The designated asset account must be yours. OT will find the "
                 "Nym based on the account.\n\n";
        return -1;
    }

    string strAssetTypeID =
        OTAPI_Wrap::GetAccountWallet_AssetTypeID(strMyAcctID);
    if (!VerifyStringVal(strAssetTypeID)) {
        otOut << "Failure: Unable to find Asset Type ID based on myacct. Use: "
                 "--myacct ACCT_ID\n";
        otOut << "The designated asset account must be yours. OT will find the "
                 "asset type based on the account.\n\n";
        return -1;
    }

    string strServerID = OTAPI_Wrap::GetAccountWallet_ServerID(strMyAcctID);
    if (!VerifyStringVal(strServerID)) {
        otOut << "Unable to find the server ID based on the account. Strange! "
                 "Perhaps specify a different account? Use:  --myacct ACCT_ID "
                 "\n";
        return -1;
    }

    if ((VerifyExists("Server", false) && !(strServerID == Server))) {
        otOut << "This account is on server ( " << strServerID
              << " -- the server is deduced based on the account), but the "
                 "default server is ( " << Server
              << " ). To override it, use:  --server " << strServerID << " \n";
        return -1;
    }

    string assetContract = OTAPI_Wrap::LoadAssetContract(strAssetTypeID);
    if (!VerifyStringVal(assetContract)) {
        string strResponse = MadeEasy::retrieve_contract(
            strServerID, strMyNymID, strAssetTypeID);

        if (1 != VerifyMessageSuccess(strResponse)) {
            otOut << "details_withdraw_cash: Unable to retrieve contract for "
                     "IDs: \n";
            otOut << "  Server ID: " << strServerID << "\n";
            otOut << "   Asset ID: " << strAssetTypeID << "\n";
            return -1;
        }

        assetContract = OTAPI_Wrap::LoadAssetContract(strAssetTypeID);
        if (!VerifyStringVal(assetContract)) {
            otOut << "Failure: Unable to load Asset contract even after "
                     "retrieving it.\n";
            return -1;
        }
    }
    // By this point, we KNOW the appropriate asset contract is available.

    string strMint = MadeEasy::load_or_retrieve_mint(strServerID, strMyNymID,
                                                     strAssetTypeID);

    if (!VerifyStringVal(strMint)) {
        otOut << "Failure: Unable to load or retrieve necessary mint file for "
                 "withdrawal.\n";
        return -1;
    }
    // By this point, we know we can successfully load both:
    // 1. the proper asset contract.
    // 2. the proper (unexpired) mint file.

    string strResponse =
        MadeEasy::withdraw_cash(strServerID, strMyNymID, strMyAcctID, lAmount);
    string strAttempt = "withdraw_cash";

    int32_t nInterpretReply = InterpretTransactionMsgReply(
        strServerID, strMyNymID, strMyAcctID, strAttempt, strResponse);

    if (1 == nInterpretReply) {
        bool bRetrieved = MadeEasy::retrieve_account(strServerID, strMyNymID,
                                                     strMyAcctID, false);

        otOut << "\n\nServer response (" << strAttempt
              << "): SUCCESS withdrawing cash! (From account on server to "
                 "local purse.) \n";
        otOut << (bRetrieved ? "Success" : "Failed")
              << " retrieving intermediary files for account.\n";
    }

    return nInterpretReply;
}

// HERE, WE GET ALL THE ARGUMENTS TOGETHER,
// and then call the above function.
//
OT_COMMANDS_OT int32_t OT_Command::mainWithdrawCash()
{
    otOut << "Usage:   withdraw --myacct YOUR_ASSET_ACCT \nYou can provide an "
             "amount:  --args \"amount PUT_AMOUNT_HERE\"\n\n";

    if (VerifyExists("MyAcct")) {

        string strAssetTypeID =
            OTAPI_Wrap::GetAccountWallet_AssetTypeID(MyAcct);
        if (!VerifyStringVal(strAssetTypeID)) {
            return -1;
        }

        string strAmount = "0";

        string strDefaultAmount = "1";

        // If custom arguments have been passed on the command line,
        // then grab them and use them instead of asking the user to enter them
        // at the command line.
        if (VerifyExists("Args", false)) {
            string strNewAmount = OT_CLI_GetValueByKey(Args, "amount");

            // Set the values based on the custom arguments, for those found.
            if (VerifyStringVal(strNewAmount)) {
                strAmount = strNewAmount;
            }
        }

        // If the withdrawal parameters aren't provided, then we
        // ask the user to supply them at the command line.
        if (!VerifyStringVal(strAmount) ||
            (OTAPI_Wrap::StringToAmount(strAssetTypeID, strAmount) < 1)) {
            otOut << "Enter the amount[" << strDefaultAmount << "]: ";
            strAmount = OT_CLI_ReadLine();
        }

        if (!VerifyStringVal(strAmount) ||
            (OTAPI_Wrap::StringToAmount(strAssetTypeID, strAmount) < 1)) {
            strAmount = strDefaultAmount;
        }

        int64_t lAmount = OTAPI_Wrap::StringToAmount(strAssetTypeID, strAmount);

        return details_withdraw_cash(MyAcct, lAmount);
    }

    return -1;
}

//
// WITHDRAW VOUCHER
//
//(From asset account into instrument form,
// like a cashier's cheque.)
//

OT_COMMANDS_OT int32_t OT_Command::details_withdraw_voucher(string& strOutput)
{
    if (!VerifyExists("HisNym")) {
        return -1;
    }

    string strMyNymID = OTAPI_Wrap::GetAccountWallet_NymID(MyAcct);
    if (!VerifyStringVal(strMyNymID)) {
        otOut << "\n\nFailure: Unable to find NymID based on myacct. Use: "
                 "--myacct ACCT_ID\nThe designated asset account must be "
                 "yours. OT will find the Nym based on the account.\n\n";
        return -1;
    }

    string strServerID = OTAPI_Wrap::GetAccountWallet_ServerID(MyAcct);
    if (!VerifyStringVal(strServerID)) {
        otOut << "\n\nFailure: Unable to find the server ID based on myacct. "
                 "Use: --myacct ACCT_ID\nThe designated asset account must be "
                 "yours. OT will find the server ID based on the account.\n\n";
        return -1;
    }

    string strRecipientPubkey =
        MadeEasy::load_or_retrieve_encrypt_key(strServerID, strMyNymID, HisNym);
    if (!VerifyStringVal(strRecipientPubkey)) {
        otOut << "\n\nFailure: Unable to load or download pubkey for HisNym "
                 "based on given value (" << HisNym
              << "). To override, use: --hisnym HIS_NYM_ID\n\n";
        return -1;
    }

    string strAmount = "0";
    string strMemo = "";

    string strDefaultAmount = "1";
    string strDefaultMemo = "(blank memo field)";

    // If custom arguments have been passed on the command line,
    // then grab them and use them instead of asking the user to enter them
    // at the command line.
    string strAssetTypeID = OTAPI_Wrap::GetAccountWallet_AssetTypeID(MyAcct);

    if (VerifyExists("Args", false)) {
        string strNewAmount = OT_CLI_GetValueByKey(Args, "amount");
        string strNewMemo = OT_CLI_GetValueByKey(Args, "memo");

        // Set the values based on the custom arguments, for those found.
        if (VerifyStringVal(strNewAmount)) {
            strAmount = strNewAmount;
        }
        if (VerifyStringVal(strNewMemo)) {
            strMemo = strNewMemo;
        }
    }

    // If the transfer parameters aren't provided, then we
    // ask the user to supply them at the command line.
    if (!VerifyStringVal(strAmount) ||
        (OTAPI_Wrap::StringToAmount(strAssetTypeID, strAmount) < 1)) {
        otOut << "Enter the amount[" << strDefaultAmount << "]: ";
        strAmount = OT_CLI_ReadLine();
    }
    if (!VerifyStringVal(strMemo)) {
        otOut << "Optionally, enter a memo on a single line[" << strDefaultMemo
              << "]: ";
        strMemo = OT_CLI_ReadLine();
    }

    if (!VerifyStringVal(strAmount) ||
        (OTAPI_Wrap::StringToAmount(strAssetTypeID, strAmount) < 1)) {
        strAmount = strDefaultAmount;
    }
    if (!VerifyStringVal(strMemo)) {
        strMemo = strDefaultMemo;
    }

    int64_t lAmount = OTAPI_Wrap::StringToAmount(strAssetTypeID, strAmount);
    string strResponse = MadeEasy::withdraw_voucher(
        strServerID, strMyNymID, MyAcct, HisNym, strMemo, lAmount);
    string strAttempt = "withdraw_voucher";

    int32_t nInterpretReply = InterpretTransactionMsgReply(
        strServerID, strMyNymID, MyAcct, strAttempt, strResponse);

    if (1 != nInterpretReply) return nInterpretReply;

    string strLedger = OTAPI_Wrap::Message_GetLedger(strResponse);

    if (!VerifyStringVal(strLedger)) {
        otOut << "\n\n details_withdraw_voucher: Error: strLedger is null, "
                 "returned by OT_API_Message_GetLedger.\n";
        return -1;
    }

    string strTransReply = OTAPI_Wrap::Ledger_GetTransactionByIndex(
        strServerID, strMyNymID, MyAcct, strLedger, 0);

    if (!VerifyStringVal(strTransReply)) {
        otOut << "details_withdraw_voucher: Error: strTransReply is "
                 "unexpectedly null, returned by "
                 "OT_API_Ledger_GetTransactionByIndex, argument passed, "
                 "index 0 and ledger:\n\n" << strLedger << "\n\n";
        return -1;
    }

    strOutput = OTAPI_Wrap::Transaction_GetVoucher(strServerID, strMyNymID,
                                                   MyAcct, strTransReply);

    if (!VerifyStringVal(strOutput)) {
        otOut << "details_withdraw_voucher: Error: Voucher is unexpectedly "
                 "null, returned by OT_API_Transaction_GetVoucher with "
                 "strTransReply set to:\n\n" << strTransReply << "\n\n";
        return -1;
    }

    // Save a copy in my own outpayments box. I don't want to lose this
    // voucher since it uses
    // one of my own transaction numbers. (If I later send the voucher
    // to someone, OT is smart
    // enough to remove the first copy from outpayments, when adding the
    // second copy.)
    //
    // Notice how I can send an instrument to myself. This doesn't
    // actually send anything --
    // it just puts a copy into my outpayments box for safe-keeping.
    MadeEasy::send_user_payment(strServerID, strMyNymID, strMyNymID, strOutput);

    bool bRetrieved =
        MadeEasy::retrieve_account(strServerID, strMyNymID, MyAcct, true);
    otOut << (bRetrieved ? "Success" : "Failed")
          << " retrieving intermediary files for account.\n";

    otOut << "details_withdraw_voucher: Voucher returned by "
             "OT_API_Transaction_GetVoucher:\n\n";
    cout << strOutput << "\n";
    otOut << "\n\n";

    otOut << "\n\nServer response (" << strAttempt
          << "): SUCCESS withdrawing voucher (cashier's cheque)!\n";

    return 1;
}

// HERE, WE GET ALL THE ARGUMENTS TOGETHER,
// and then call the above function.
//
OT_COMMANDS_OT int32_t OT_Command::mainWithdrawVoucher()
{
    otOut << "Usage:   withdrawvoucher --myacct YOUR_ASSET_ACCT --hisnym "
             "RECIPIENT_NYM_ID\n\nAlso NECESSARY: --args \"amount "
             "PUT_AMOUNT_HERE\"\nAnd OPTIONALLY: --args \"memo \\\"Just a memo "
             "for the voucher cheque.\\\"\"\n\n";

    if (VerifyExists("MyAcct")) {
        string strVoucher = "";
        return details_withdraw_voucher(strVoucher);
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainSendVoucher()
{
    otOut << "Usage:   sendvoucher  --myacct MY_ASSET_ACCT --hisnym "
             "RECIPIENT_NYM_ID\nOptionally: --args \"memo \\\"one-line memo "
             "allowed here.\\\" amount AMOUNT\"\nServer is deduced from "
             "MyAcct. This command withdraws AND SENDS the\nvoucher. (Use "
             "'withdrawvoucher', not 'sendvoucher', if you don't want it to be "
             "immediately SENT.)\n";

    if (!VerifyExists("MyAcct")) return -1;

    string strCheque = "";
    int32_t nReturnVal = details_withdraw_voucher(strCheque);

    if ((1 != nReturnVal) || !VerifyStringVal(strCheque)) return nReturnVal;

    otOut << "\n-------------------------------------------\n the "
             "voucher:\n\n";

    cout << strCheque << "\n";

    otOut << "\n";

    // TODO: Store a copy in outpayments box (OR VERIFY THAT IT'S
    // ALREADY BEING DONE, WHICH I BELIEVE THAT IT IS.)

    string strServerID = OTAPI_Wrap::Instrmnt_GetServerID(strCheque);
    if (!VerifyStringVal(strServerID)) {
        otOut << "Unable to retrieve a Server ID from this voucher! "
                 "Very strange. (Failure.)\n";
        return -1;
    }

    string strSenderNymID = OTAPI_Wrap::GetAccountWallet_NymID(MyAcct);
    if (!VerifyStringVal(strSenderNymID)) {
        otOut << "Unable to retrieve a Sender Nym ID from this "
                 "voucher! Very strange. (Failure.)\n";
        return -1;
    }

    string strRecipientNymID =
        OTAPI_Wrap::Instrmnt_GetRecipientUserID(strCheque);
    if (!VerifyStringVal(strRecipientNymID)) {
        otOut << "Unable to retrieve a Recipient Nym ID from this "
                 "voucher! Very strange. (Failure.)\n(Although it's "
                 "fine to WITHDRAW a voucher with no recipient, still "
                 "need the recipient ID to SEND one.)\n";
        return -1;
    }

    string strResponse = MadeEasy::send_user_payment(
        strServerID, strSenderNymID, strRecipientNymID, strCheque);
    nReturnVal = VerifyMessageSuccess(strResponse);
    if (1 == nReturnVal) {
        otOut << "Success in sendvoucher! Server response:\n\n";
        cout << strResponse << "\n";
        otOut << "(Success in sendvoucher)\n";
        return 1;
    }

    otOut << "sendvoucher: Failed.\n";

    return nReturnVal;
}

OT_COMMANDS_OT OTDB::MarketList* OT_Command::loadMarketList(
    const string& serverID)
{
    if (!OTDB::Exists("markets", serverID, "market_data.bin", "")) {
        otOut << "The market list file doesn't exist.\n";
        return nullptr;
    }

    otWarn << "Markets file exists...Querying list of markets...\n";

    OTDB::Storable* storable =
        OTDB::QueryObject(OTDB::STORED_OBJ_MARKET_LIST, "markets", serverID,
                          "market_data.bin", "");
    if (nullptr == storable) {
        otOut << "Failed to verify storable object. Probably doesn't exist.\n";
        return nullptr;
    }

    otWarn << "QueryObject worked. Now dynamic casting from storable to "
              "marketlist...\n";

    OTDB::MarketList* marketList = dynamic_cast<OTDB::MarketList*>(storable);
    if (nullptr == marketList) {
        otOut << "Unable to dynamic cast a storable to a marketlist.\n";
        return nullptr;
    }

    return marketList;

    // This WAS a "load or create" sort of function, but I commented out the
    // "create" part because
    // you will literally NEVER need to create this list.
}

OT_COMMANDS_OT int32_t OT_Command::mainShowMarkets()
{
    if (!VerifyExists("Server")) return -1;

    OTDB::MarketList* marketListPtr = loadMarketList(Server);
    if (!marketListPtr) {
        otOut << "Unable to load up marketlist from local storage.\n";
        return -1;
    }
    OTDB::MarketList& marketList = *marketListPtr;

    // LOOP THROUGH THE MARKETS AND PRINT THEM OUT.

    int32_t nCount = marketList.GetMarketDataCount();
    if (0 > nCount) {
        otOut << "Loaded the market list, but GetMarketDataCount returns "
                 "an invalid result. (Failure.)\n";
    }
    else if (nCount <= 0) {
        otOut << "Loaded the market list, but GetMarketDataCount says "
                 "there aren't any markets in the list. (Returning.)\n";
    }
    else {
        otOut << "\nIndex\tScale\tMarket\t\t\t\t\t\tAsset\t\t\t\t\t\tCurren"
                 "cy\n";

        for (int32_t nIndex = 0; nIndex < nCount; ++nIndex) {
            OTDB::MarketData* marketDataPtr = marketList.GetMarketData(nIndex);
            if (!marketDataPtr) {
                otOut << "Unable to reference marketData on marketList, at "
                         "index: " << nIndex << "\n";
                return -1;
            }
            OTDB::MarketData& marketData = *marketDataPtr;

            // OUTPUT THE MARKET DATA...
            cout << nIndex << "\t" << marketData.scale << "\tM "
                 << marketData.market_id << "\tA " << marketData.asset_type_id
                 << "\tC " << marketData.currency_type_id << "\n";
        }
    }

    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainGetMarkets()
{
    otOut
        << "Usage:   getmarketlist --server SERVER_ID --mynym YOUR_NYM_ID\n\n";

    if (!VerifyExists("Server") || !VerifyExists("MyNym")) return -1;

    string strResponse = MadeEasy::get_market_list(Server, MyNym);
    string strAttempt = "get_market_list";
    int32_t nInterpretReply = VerifyMessageSuccess(strResponse);
    if (1 == nInterpretReply) {
        otOut << "Server response (" << strAttempt
              << "): SUCCESS getting market list.\n\n";

        OT_Command::mainShowMarkets();
    }

    return nInterpretReply;
}

OT_COMMANDS_OT OTDB::OfferListMarket* OT_Command::loadMarketOffers(
    const string& serverID, const string& marketID)
{
    OTDB::OfferListMarket* offerList = nullptr;

    if (OTDB::Exists("markets", serverID, "offers", marketID + ".bin")) {
        otWarn << "Offers file exists... Querying file for market offers...\n";
        OTDB::Storable* storable =
            OTDB::QueryObject(OTDB::STORED_OBJ_OFFER_LIST_MARKET, "markets",
                              serverID, "offers", marketID + ".bin");

        if (nullptr == storable) {
            otOut << "Unable to verify storable object. Probably doesn't "
                     "exist.\n";
            return nullptr;
        }

        otWarn << "QueryObject worked. Now dynamic casting from storable to a "
                  "(market) offerList...\n";
        offerList = dynamic_cast<OTDB::OfferListMarket*>(storable);

        if (nullptr == offerList) {
            otOut << "Unable to dynamic cast a storable to a (market) "
                     "offerList.\n";
            return nullptr;
        }
    }

    return offerList;
}

OT_COMMANDS_OT int32_t
    OT_Command::details_show_market_offers(const string& strServerID,
                                           const string& strMarketID)
{
    OTDB::OfferListMarket* offerListPtr =
        loadMarketOffers(strServerID, strMarketID);

    if (!offerListPtr) {
        otOut << "Unable to load up a (market) offerList from local storage.\n";
        return -1;
    }
    OTDB::OfferListMarket& offerList = *offerListPtr;

    // LOOP THROUGH THE BIDS AND PRINT THEM OUT.
    int32_t nBidCount = offerList.GetBidDataCount();
    int32_t nTemp = nBidCount;

    if (nBidCount > 0) {
        otOut << "\n** BIDS **\n\nIndex\tTrans#\tPrice\tAvailable\n";

        for (int32_t nIndex = 0; nIndex < nBidCount; ++nIndex) {
            OTDB::BidData* offerDataPtr = offerList.GetBidData(nIndex);
            if (!offerDataPtr) {
                otOut << "Unable to reference bidData on offerList, at index: "
                      << nIndex << "\n";
                return -1;
            }
            OTDB::BidData& offerData = *offerDataPtr;

            // OUTPUT THE BID OFFER DATA...
            cout << nIndex << "\t" << offerData.transaction_id << "\t"
                 << offerData.price_per_scale << "\t"
                 << offerData.available_assets << "\n";
        }
    }

    // LOOP THROUGH THE ASKS AND PRINT THEM OUT.
    int32_t nAskCount = offerList.GetAskDataCount();

    if (nAskCount > 0) {
        otOut << "\n** ASKS **\n\nIndex\tTrans#\tPrice\tAvailable\n";

        for (int32_t nIndex = 0; nIndex < nAskCount; ++nIndex) {
            nTemp = nIndex;
            OTDB::AskData* offerDataPtr = offerList.GetAskData(nTemp);

            if (!offerDataPtr) {
                otOut << "Unable to reference askData on offerList, at index: "
                      << nIndex << "\n";
                return -1;
            }
            OTDB::AskData& offerData = *offerDataPtr;

            // OUTPUT THE ASK OFFER DATA...
            cout << nIndex << "\t" << offerData.transaction_id << "\t"
                 << offerData.price_per_scale << "\t"
                 << offerData.available_assets << "\n";
        }
    }

    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::impl_show_market_offers(string& strMarket)
{
    otOut << "Usage:   showoffers --server SERVER_ID --mynym "
             "YOUR_NYM_ID\nAlso: --args \"market MARKET_ID\"\n";

    if (!VerifyExists("Server")) return -1;

    // If custom arguments have been passed on the command line,
    // then grab them and use them instead of asking the user to enter them
    // at the command line.
    if (!VerifyStringVal(strMarket) && VerifyExists("Args", false)) {
        string strNewMarket = OT_CLI_GetValueByKey(Args, "market");

        // Set the values based on the custom arguments, for those found.
        if (VerifyStringVal(strNewMarket)) {
            strMarket = strNewMarket;
        }
    }

    // If the transfer parameters aren't provided, then we
    // ask the user to supply them at the command line.
    if (!VerifyStringVal(strMarket)) {
        OT_Command::mainShowMarkets();

        otOut << "\nEnter the market ID: ";
        strMarket = OT_CLI_ReadLine();
    }

    if (!VerifyStringVal(strMarket)) {
        return -1;
    }

    return details_show_market_offers(Server, strMarket);
}

OT_COMMANDS_OT int32_t OT_Command::mainShowOffers()
{
    string strMarket;

    return impl_show_market_offers(strMarket);
}

OT_COMMANDS_OT int32_t OT_Command::mainGetOffers()
{
    otOut << "Usage:   getoffers --server SERVER_ID --mynym YOUR_NYM_ID\nAlso: "
             "--args \"market MARKET_ID depth MAX_DEPTH\"\n";

    if (!VerifyExists("Server") || !VerifyExists("MyNym")) return -1;

    string strMarket = "";
    string strDepth = "";

    string strDefaultDepth = "50";

    // If custom arguments have been passed on the command line,
    // then grab them and use them instead of asking the user to enter them
    // at the command line.
    if (VerifyExists("Args", false)) {
        string strNewMarket = OT_CLI_GetValueByKey(Args, "market");
        string strNewDepth = OT_CLI_GetValueByKey(Args, "depth");

        // Set the values based on the custom arguments, for those found.
        if (VerifyStringVal(strNewMarket)) {
            strMarket = strNewMarket;
        }
        if (VerifyStringVal(strNewDepth)) {
            strDepth = strNewDepth;
        }
    }

    // If the transfer parameters aren't provided, then we
    // ask the user to supply them at the command line.
    if (!VerifyStringVal(strMarket)) {
        OT_Command::mainShowMarkets();

        otOut << "\nEnter the market ID: ";
        strMarket = OT_CLI_ReadLine();
    }
    if (!VerifyStringVal(strDepth)) {
        otOut << "Optionally, enter a max depth on a single line[" << strDepth
              << "]: ";
        strDepth = OT_CLI_ReadLine();
    }

    if (!VerifyStringVal(strMarket)) {
        return -1;
    }
    if (!VerifyStringVal(strDepth)) {
        strDepth = strDefaultDepth;
    }

    int64_t lDepth = std::stoll(strDepth);
    string strResponse =
        MadeEasy::get_market_offers(Server, MyNym, strMarket, lDepth);
    string strAttempt = "get_market_offers";
    int32_t nInterpretReply = VerifyMessageSuccess(strResponse);

    if (1 == nInterpretReply) {
        otOut << "Server response (" << strAttempt
              << "): SUCCESS getting market offers.\n\n";

        impl_show_market_offers(strMarket);
    }

    return nInterpretReply;
}

// FIX: not used by opentxs CLI utility?
OT_COMMANDS_OT int32_t OT_Command::mainAdjustUsageCredits()
{
    otOut << "\n\n  Options: --server SERVER_ID --mynym NYM_ID\n           "
             "--hisnym SUBJECT_NYM_ID\n\nTo adjust (change) a Nym's usage "
             "credits: --args \"adjust POSITIVE_OR_NEGATIVE_VALUE\" \n (Used "
             "for giving or taking away usage credits.)\nFor example, --args "
             "\"adjust 10000\"   or:  --args \"adjust -100\" \n\nFYI, this "
             "command also retrieves the current usage credits,\nand any Nym "
             "can use it on himself, read-only.\n\n";

    if (!VerifyExists("Server") || !VerifyExists("MyNym") ||
        !VerifyExists("HisNym"))
        return -1;

    string strAdjustment = "0";

    if (VerifyExists("Args", false)) {
        string strNewAdjust = OT_CLI_GetValueByKey(Args, "adjust");

        if (VerifyStringVal(strNewAdjust)) {
            strAdjustment = strNewAdjust;
        }
    }

    string strResponse =
        MadeEasy::adjust_usage_credits(Server, MyNym, HisNym, strAdjustment);
    int32_t nStatus = VerifyMessageSuccess(strResponse);
    switch (nStatus) {
    case 1: {
        otOut << "\n\n Server response:\n\n";
        cout << strResponse << "\n";

        // IF THE_MESSAGE is of command type @usageCredits, and IF it was a
        // SUCCESS,
        // then this function returns the usage credits BALANCE (it's a
        // int64_t int32_t, but
        // passed as a string). If you adjusted the balance using the
        // usageCredits
        // command (THE_MESSAGE being the server's reply to that) then you
        // will see
        // the balance AFTER the adjustment. (The current "Usage Credits"
        // balance.)
        int64_t lNewUsageBalance =
            OTAPI_Wrap::Message_GetUsageCredits(strResponse);
        string strNewUsageBalance =
            lNewUsageBalance > -2 ? std::to_string(lNewUsageBalance) : "";

        if (!VerifyStringVal(strNewUsageBalance)) {
            strNewUsageBalance = "(Error while calling API to retrieve "
                                 "usage credits from server reply.)";
        }
        else if (int64_t(-2) == lNewUsageBalance) {
            strNewUsageBalance = "Error code -2, while attempting to "
                                 "retrieve usage credits from this server "
                                 "reply.";
        }
        else if (int64_t(-1) == lNewUsageBalance) {
            strNewUsageBalance = "Either Nym has unlimited usage credits, "
                                 "or server has its usage credit "
                                 "enforcement turned off.\n(Either way, "
                                 "Nym is good to go, since -1 was returned "
                                 "as his 'credits balance'.)";
        }
        else if (int64_t(0) == lNewUsageBalance) {
            strNewUsageBalance = "Nym appears to have exhausted his supply "
                                 "of usage credits.\n(0 was returned as "
                                 "his 'credits balance'.)";
        }
        else {
            strNewUsageBalance = "Nym currently has " +
                                 std::to_string(lNewUsageBalance) +
                                 " usage credits.";
        }

        otOut << "\n\n adjust_usage_credits -- reply: " + strNewUsageBalance +
                     "\n\n(FYI, this server message is the only and proper "
                     "way to query for your own current usage credits "
                     "balance, or also to set someone else's.)\n\n";

        break;
    }
    case 0:
        otOut << "\n\nServer response:\n\n";
        cout << strResponse << "\n";
        otOut << "\n\n FAILURE in adjust_usage_credits!\n\n";
        break;
    default:
        if (VerifyStringVal(strResponse)) {
            otOut << "Server response:\n\n";
            cout << strResponse << "\n";
        }
        otOut << "\n\nError in adjust_usage_credits! nStatus is: " << nStatus
              << "\n";
        break;
    }

    otOut << "\n\n";

    return (0 == nStatus) ? -1 : nStatus;
}

OT_COMMANDS_OT int32_t
    OT_Command::details_show_nym_offers(const string& strServerID,
                                        const string& strNymID)
{
    string strLocation = "details_show_nym_offers";

    OTDB::OfferListNym* offerListPtr = loadNymOffers(strServerID, strNymID);

    if (!offerListPtr) {
        otOut << strLocation << ": Unable to load up a (nym) offerList from "
                                "local storage. Probably doesn't exist.\n";
        return -1;
    }
    OTDB::OfferListNym& offerList = *offerListPtr;

    // LOOP THROUGH THE OFFERS and sort them into a map_of_maps, key is:
    // scale-assetID-currencyID
    // the value for each key is a sub-map, with the key: transaction ID and
    // value: the offer data itself.
    int32_t nCount = offerList.GetOfferDataNymCount();
    if (nCount > 0) {
        MapOfMaps* map_of_mapsPtr = convert_offerlist_to_maps(offerList);

        if (!map_of_mapsPtr) {
            otOut << strLocation << ": Unable to convert offer list to map of "
                                    "offers. Perhaps it's empty?\n";
            return -1;
        }
        MapOfMaps& map_of_maps = *map_of_mapsPtr;

        // output_nymoffer_data is called for each offer, for this nym,
        // as it iterates through the maps.
        //
        // iterate_nymoffers_maps takes a final parameter extra_vals (not seen
        // here)
        // Other sections in the code which use iterate_nymoffers_maps might
        // pass a value
        // here, or expect one to be returned through the same mechansim.
        int32_t nIterated =
            iterate_nymoffers_maps(map_of_maps, output_nymoffer_data);

        if (-1 == nIterated) {
            otOut << strLocation << ": Error trying to iterate nym's offers.\n";
            return -1;
        }
    }

    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainShowMyOffers()
{
    otOut << "Usage:   showmyoffers --server SERVER_ID --mynym YOUR_NYM_ID\n\n";

    if (VerifyExists("Server") && VerifyExists("MyNym")) {
        return details_show_nym_offers(Server, MyNym);
    }

    return -1;
}

OT_COMMANDS_OT string
    OT_Command::details_get_nym_market_offers(const string& strServerID,
                                              const string& strNymID)
{
    string strResponse;

    if (VerifyStringVal(strServerID) && VerifyStringVal(strNymID)) {
        strResponse = MadeEasy::get_nym_market_offers(strServerID, strNymID);
    }

    return strResponse;
}

OT_COMMANDS_OT int32_t OT_Command::mainGetMyOffers()
{

    otOut << "Usage:   getmyoffers --server SERVER_ID --mynym YOUR_NYM_ID\n";

    if (!VerifyExists("Server") || !VerifyExists("MyNym")) return -1;

    string strResponse = details_get_nym_market_offers(Server, MyNym);
    string strAttempt = "get_nym_market_offers";
    int32_t nInterpretReply = VerifyMessageSuccess(strResponse);
    if (1 == nInterpretReply) {
        otOut << "Server response (" << strAttempt
              << "): SUCCESS getting nym's market offers.\n\n";

        OT_Command::mainShowMyOffers();
    }

    return nInterpretReply;
}

//
// PAY DIVIDEND
//
// Just like withdraw voucher...except instead of withdrawing a single voucher
// to yourself,
// it removes the total dividend payout from your account, and then divides it
// up amongst the
// shareholders, sending them EACH a voucher cheque in the amount of strAmount *
// number of shares owned.
//
OT_COMMANDS_OT int32_t OT_Command::details_pay_dividend(const string& strAmount,
                                                        const string& strMemo)
{
    string strMyNymID = OTAPI_Wrap::GetAccountWallet_NymID(MyAcct);
    if (!VerifyStringVal(strMyNymID)) {
        otOut << "\n\nFailure: Unable to find Payer NymID based on myacct. "
                 "Use: --myacct DIVIDEND_SOURCE_ACCT_ID\n";
        otOut << "The designated asset account (that the dividends will be "
                 "paid out of) must be yours. OT will find the Nym based on "
                 "that account.\n\n";
        return -1;
    }

    string strHisPurse;

    if (VerifyExists("HisPurse")) {
        strHisPurse = HisPurse;
    }
    else {
        while (true) {
            otOut << "Enter the SHARES asset type ID (Nym must also be the "
                     "issuer for these shares): ";
            string strHisPartialPurseID = OT_CLI_ReadLine();

            if (VerifyStringVal(strHisPartialPurseID)) {
                strHisPurse = OTAPI_Wrap::Wallet_GetAssetIDFromPartial(
                    strHisPartialPurseID);

                if (!VerifyStringVal(strHisPurse)) {
                    otOut << "\n\nFailure: Unable to find SHARES_ASSET_TYPE_ID "
                             "in your wallet. Use: --hispurse "
                             "SHARES_ASSET_TYPE_ID\n";
                    return -1;
                }
                break;
            }
        }
    }

    string strAssetTypeID = OTAPI_Wrap::GetAccountWallet_AssetTypeID(MyAcct);

    int64_t lAmount = OTAPI_Wrap::StringToAmount(strAssetTypeID, strAmount);
    string strResponse = MadeEasy::pay_dividend(Server, strMyNymID, MyAcct,
                                                strHisPurse, strMemo, lAmount);
    string strAttempt = "pay_dividend";

    int32_t nInterpretReply = InterpretTransactionMsgReply(
        Server, strMyNymID, MyAcct, strAttempt, strResponse);

    if (1 == nInterpretReply) {
        bool bRetrieved =
            MadeEasy::retrieve_account(Server, strMyNymID, MyAcct, true);
        otOut << (bRetrieved ? "Success" : "Failed")
              << " retrieving intermediary files for account.\n";

        otOut << "\n\nServer response (" << strAttempt
              << "): SUCCESS paying out dividends.\n";
    }

    return nInterpretReply;
}

// HERE, WE GET ALL THE ARGUMENTS TOGETHER,
// and then call the above function.
//
OT_COMMANDS_OT int32_t OT_Command::mainPayDividend()
{
    otOut << "Usage:   paydividend --server SERVER_ID --mynym "
             "SHARES_ISSUER_NYM_ID\n             --myacct "
             "DIVIDEND_SOURCE_ACCT_ID --hispurse SHARES_ASSET_TYPE_ID\n\nAlso "
             "necessary: --args \"amount PAYOUT_AMOUNT_PER_SHARE\"\nAnd "
             "OPTIONALLY: --args \"memo \\\"Just a memo for the dividend "
             "payment.\\\"\"\n\n";

    if (!VerifyExists("Server") || !VerifyExists("MyAcct")) return -1;

    string strAmount = "0";
    string strNote = "";

    string strDefaultAmount = "1";
    string strDefaultNote = "(blank memo field)";

    // If custom arguments have been passed on the command line,
    // then grab them and use them instead of asking the user to enter them
    // at the command line.
    if (VerifyExists("Args", false)) {
        string strNewAmount = OT_CLI_GetValueByKey(Args, "amount");
        string strNewNote = OT_CLI_GetValueByKey(Args, "memo");

        // Set the values based on the custom arguments, for those found.
        if (VerifyStringVal(strNewAmount)) {
            strAmount = strNewAmount;
        }
        if (VerifyStringVal(strNewNote)) {
            strNote = strNewNote;
        }
    }

    string strAssetTypeID = OTAPI_Wrap::GetAccountWallet_AssetTypeID(MyAcct);

    // If the transfer parameters aren't provided, then we
    // ask the user to supply them at the command line.
    if (!VerifyStringVal(strAmount) ||
        (OTAPI_Wrap::StringToAmount(strAssetTypeID, strAmount) < 1)) {
        otOut << "Enter the 'Payout Amount Per-Share'[" << strDefaultAmount
              << "]: ";
        strAmount = OT_CLI_ReadLine();
    }
    if (!VerifyStringVal(strNote)) {
        otOut << "Optionally, enter a memo on a single line[" << strDefaultNote
              << "]: ";
        strNote = OT_CLI_ReadLine();
    }

    if (!VerifyStringVal(strAmount) ||
        (OTAPI_Wrap::StringToAmount(strAssetTypeID, strAmount) < 1)) {
        strAmount = strDefaultAmount;
    }
    if (!VerifyStringVal(strNote)) {
        strNote = strDefaultNote;
    }

    return details_pay_dividend(strAmount, strNote);
}

OT_COMMANDS_OT int32_t OT_Command::mainShowPurse()
{
    otOut << "Usage:   showpurse --mypurse ASSET_TYPE_ID --mynym YOUR_NYM_ID "
             "--server SERVER_ID \n\n";

    if (!VerifyExists("Server") || !VerifyExists("MyNym") ||
        !VerifyExists("MyPurse"))
        return -1;

    string strPurse = OTAPI_Wrap::LoadPurse(Server, MyPurse, MyNym);

    if (!VerifyStringVal(strPurse)) {
        otOut << "\n OT_Command::mainShowPurse: Unable to load purse. Does "
                 "it even exist?\n";
        return -1;
    }

    int64_t lAmount =
        OTAPI_Wrap::Purse_GetTotalValue(Server, MyPurse, strPurse);

    cout << "\n\nTOTAL VALUE: " << OTAPI_Wrap::FormatAmount(MyPurse, lAmount)
         << "\n";

    // Loop through purse contents and display tokens.
    int32_t nCount = OTAPI_Wrap::Purse_Count(Server, MyPurse, strPurse);
    if (nCount < 0) {
        otOut << "\n OT_Command::mainShowPurse: Error: Unexpected bad "
                 "value returned from OT_API_Purse_Count.\n\n";
        return -1;
    }

    if (nCount <= 0) return 1;

    cout << "Token count: " << nCount << "\n\n";
    cout << "Index\tValue\tSeries\tValidFrom\tValidTo\t\tStatus\n";

    int32_t nIndex = -1;

    while (nCount > 0) {
        --nCount;
        ++nIndex;

        string strToken =
            OTAPI_Wrap::Purse_Peek(Server, MyPurse, MyNym, strPurse);

        if (!VerifyStringVal(strToken)) {
            otOut << "mainShowPurse: Error: OT_API_Purse_Peek "
                     "unexpectedly returned nullptr instead of "
                     "token.\n";
            return -1;
        }

        string strNewPurse =
            OTAPI_Wrap::Purse_Pop(Server, MyPurse, MyNym, strPurse);

        if (!VerifyStringVal(strNewPurse)) {
            otOut << "mainShowPurse: Error: OT_API_Purse_Pop "
                     "unexpectedly returned nullptr instead of "
                     "updated purse.\n";
            return -1;
        }

        strPurse = strNewPurse;

        int64_t lDenomination =
            OTAPI_Wrap::Token_GetDenomination(Server, MyPurse, strToken);
        int32_t nSeries =
            OTAPI_Wrap::Token_GetSeries(Server, MyPurse, strToken);
        time64_t tValidFrom =
            OTAPI_Wrap::Token_GetValidFrom(Server, MyPurse, strToken);
        time64_t tValidTo =
            OTAPI_Wrap::Token_GetValidTo(Server, MyPurse, strToken);
        time64_t lTime = OTAPI_Wrap::GetTime();

        if (0 > lDenomination) {
            otOut << "Error while showing purse: bad "
                     "lDenomination.\n";
            return -1;
        }
        if (OT_TIME_ZERO > lTime) {
            otOut << "Error while showing purse: bad strTime.\n";
            return -1;
        }

        string strStatus = (lTime > tValidTo) ? "expired" : "valid";

        cout << nIndex << "\t" << lDenomination << "\t" << nSeries << "\t"
             << tValidFrom << "\t" << tValidTo << "\t" << strStatus << "\n";
    }

    return 1;
}

//
// THESE FUNCTIONS were added for the PAYMENTS screen. (They are fairly new.)
//
// Basically there was a need to have DIFFERENT instruments, but to be able to
// treat them as though they are a single type.
//
// In keeping with that, the below functions will work with disparate types.
// You can pass [ CHEQUES / VOUCHERS / INVOICES ] and PAYMENT PLANS, and
// SMART CONTRACTS, and PURSEs into these functions, and they should be able
// to handle any of those types.
//

OT_COMMANDS_OT int32_t OT_Command::details_deposit_cheque(
    const string& strServerID, const string& strMyAcct,
    const string& strMyNymID, const string& strInstrument, const string&)
{
    string strAssetTypeID = OTAPI_Wrap::Instrmnt_GetAssetID(strInstrument);

    if (!VerifyStringVal(strAssetTypeID)) {
        otOut << "\n\nFailure: Unable to find Asset Type ID on the "
                 "instrument.\n\n";
        return -1;
    }

    string strAssetTypeAcct =
        OTAPI_Wrap::GetAccountWallet_AssetTypeID(strMyAcct);
    if (strAssetTypeID != strAssetTypeAcct) {
        otOut << "\n\nFailure: Asset Type ID on the instrument ( "
              << strAssetTypeID << " ) doesn't match the one on the MyAcct ( "
              << strAssetTypeAcct << " )\n\n";
        return -1;
    }

    string strResponse = MadeEasy::deposit_cheque(strServerID, strMyNymID,
                                                  strMyAcct, strInstrument);
    string strAttempt = "deposit_cheque";

    int32_t nInterpretReply = InterpretTransactionMsgReply(
        strServerID, strMyNymID, strMyAcct, strAttempt, strResponse);

    if (1 == nInterpretReply) {
        bool bRetrieved = MadeEasy::retrieve_account(strServerID, strMyNymID,
                                                     strMyAcct, true);

        otOut << "Server response (" << strAttempt << "): SUCCESS!\n";
        otOut << (bRetrieved ? "Success" : "Failed")
              << " retrieving intermediary files for account.\n";
    }

    return nInterpretReply;
}

OT_COMMANDS_OT int32_t
    OT_Command::details_deposit_purse(const string& strServerID,
                                      const string& strMyAcct,
                                      const string& strFromNymID,
                                      const string& strInstrument,
                                      const string& strIndices)
{
    string strLocation = "details_deposit_purse";

    string strTHE_Instrument = "";

    if (VerifyStringVal(strInstrument)) {
        strTHE_Instrument = strInstrument;
    }

    string strAssetTypeID = OTAPI_Wrap::GetAccountWallet_AssetTypeID(strMyAcct);
    if (!VerifyStringVal(strAssetTypeID)) {
        otOut << "\n\nFailure: Unable to find Asset Type ID based on myacct. "
                 "Use: --myacct ACCT_ID\n";
        otOut << "The designated asset account must be yours. OT will find the "
                 "asset type based on the account.\n\n";
        return -1;
    }

    if (VerifyExists("MyPurse", false) && (MyPurse != strAssetTypeID)) {
        otOut << "Warning: Depositing to MyAcct, which is NOT the same asset "
                 "type as the MyPurse asset type specified.\n";
    }

    bool bLoadedPurse = false;

    // If strInstrument wasn't passed, that means we're supposed to load
    // the purse ourselves, from local storage.
    if (!VerifyStringVal(strTHE_Instrument)) {
        strTHE_Instrument =
            OTAPI_Wrap::LoadPurse(strServerID, strAssetTypeID, strFromNymID);

        if (!VerifyStringVal(strTHE_Instrument)) {
            otOut << "\n " << strLocation << ": Unable to load purse from "
                                             "local storage. Does it even "
                                             "exist?\n";
            return -1;
        }

        bLoadedPurse = true;
    }

    // Below this point, we know that strTHE_Instrument contains either the
    // purse as it was passed in
    // to us, or it contains the purse as we loaded it from local storage.
    // If it WAS from local storage, then there's a chance that strIndices
    // contains "all" or "4, 6, 2" etc.
    // If that's the case, then we need to iterate through the purse, and add
    // the denoted token IDs to
    // a vector (selectedTokens) and pass it into depositCashPurse.

    vector<string> vecSelectedTokenIDs;

    // If we loaded the purse (vs the user pasting one in...)
    // then the user might have wanted to deposit only selected indices,
    // rather than ALL the tokens in that purse.
    // So we'll loop through the purse and add any relevant IDs to the
    // "selected" list, since the actual Token IDs must be passed.
    if (bLoadedPurse) {

        // Loop through purse contents...
        int32_t nCount = OTAPI_Wrap::Purse_Count(strServerID, strAssetTypeID,
                                                 strTHE_Instrument);
        if (nCount < 0) {
            otOut << "\n " << strLocation << ": Error: Unexpected bad value "
                                             "returned from "
                                             "OT_API_Purse_Count.\n\n";
            return -1;
        }

        if (nCount < 1) {
            otOut << "\n " << strLocation
                  << ": The purse is empty, so you can't deposit it.\n\n";
            return -1;
        }
        else {
            // Make a copy of the purse passed in, so we can iterate it and find
            // the
            // appropriate Token IDs...

            string strPurse = strTHE_Instrument;

            if (VerifyStringVal(strIndices)) {
                int32_t nIndex = -1;

                while (nCount > 0) {
                    --nCount;
                    ++nIndex;

                    // NOTE: Owner can ONLY be strFromNymID in here, since

                    // is only true in the case where we LOADED the purse from
                    // local storage.
                    // (Therefore this DEFINITELY is not a password-protected
                    // purse.)
                    string strToken = OTAPI_Wrap::Purse_Peek(
                        strServerID, strAssetTypeID, strFromNymID, strPurse);

                    if (!VerifyStringVal(strToken)) {
                        otOut << strLocation << ": Error: OT_API_Purse_Peek "
                                                "unexpectedly returned nullptr "
                                                "instead of token.\n";
                        return -1;
                    }

                    string strNewPurse = OTAPI_Wrap::Purse_Pop(
                        strServerID, strAssetTypeID, strFromNymID, strPurse);

                    if (!VerifyStringVal(strNewPurse)) {
                        otOut << strLocation << ": Error: OT_API_Purse_Pop "
                                                "unexpectedly returned nullptr "
                                                "instead of updated purse.\n";
                        return -1;
                    }

                    strPurse = strNewPurse;

                    string strTokenID = OTAPI_Wrap::Token_GetID(
                        strServerID, strAssetTypeID, strToken);

                    if (!VerifyStringVal(strTokenID)) {
                        otOut << strLocation << ": Error while depositing "
                                                "purse: bad strTokenID.\n";
                        return -1;
                    }

                    if (!("all" == strIndices) &&
                        OTAPI_Wrap::NumList_VerifyQuery(
                            strIndices, std::to_string(nIndex))) {
                        vecSelectedTokenIDs.push_back(strTokenID);
                    }
                }
            }
        }
    }

    int32_t nResult = MadeEasy::depositCashPurse(
        strServerID, strAssetTypeID, strFromNymID, strTHE_Instrument,
        vecSelectedTokenIDs, strMyAcct, bLoadedPurse);

    return nResult;
}

OT_COMMANDS_OT int32_t OT_Command::details_deposit(const string& strServerID,
                                                   const string& strMyAcctID)
{
    string strInstrument = "";

    string strToNymID = OTAPI_Wrap::GetAccountWallet_NymID(strMyAcctID);
    if (!VerifyStringVal(strToNymID)) {
        otOut
            << "\n\nFailure: Unable to find depositor NymID based on account ( "
            << strMyAcctID << " ).\nUse: --myacct ACCT_ID\n";
        otOut << "The designated asset account must be yours. OT will find the "
                 "Nym based on the account.\n\n";
        return -1;
    }

    string strIndices = "";

    if (VerifyExists("Args", false)) {
        // You have the OPTION to pass in indices for tokens in your purse, and
        // deposit
        // will automatically assume "deposit cash" and deposit those tokens.
        // You can also specify to deposit ALL tokens in your cash purse.
        // Without this extra argument, OT will ask you to paste an instrument,
        // and then will dynamically determine its contract type.
        strIndices = OT_CLI_GetValueByKey(Args, "indices");
    }

    if (VerifyStringVal(strIndices)) {
        // Only in the case of cash, it's possible you have some cash in Nym A's
        // purse, but
        // you want to deposit it into Nym B's account. So we have a "to" Nym
        // and a "from" Nym
        // even though they will often be the same.
        string strFromNymID = "";

        if (VerifyExists("MyNym", false)) {
            strFromNymID = MyNym;
        }
        else {
            strFromNymID = strToNymID;
        }

        // In this case, strInstrument is blank.
        // That's how the callee knows that we're working with the local purse.
        // Then strIndices tells him either to use "all" tokens in that purse,
        // or
        // the selected indices.
        return details_deposit_purse(strServerID, strMyAcctID, strFromNymID,
                                     strInstrument, strIndices);
    }

    otOut << "You can deposit a PURSE (containing cash tokens) or a CHEQUE "
             "/ VOUCHER. \n";
    otOut << "Paste your financial instrument here, followed by a ~ by "
             "itself on a blank line: \n";

    strInstrument = OT_CLI_ReadUntilEOF();

    if (!VerifyStringVal(strInstrument)) {
        return -1;
    }

    string strType = OTAPI_Wrap::Instrmnt_GetType(strInstrument);

    if (!VerifyStringVal(strType)) {
        otOut << "\n\nFailure: Unable to determine instrument type. "
                 "Expected CHEQUE, VOUCHER, INVOICE, or (cash) PURSE.\n";
        return -1;
    }

    if ("CHEQUE" == strType) {
        return details_deposit_cheque(strServerID, strMyAcctID, strToNymID,
                                      strInstrument, strType);
    }
    else if ("VOUCHER" == strType) {
        return details_deposit_cheque(strServerID, strMyAcctID, strToNymID,
                                      strInstrument, strType);
    }
    else if ("INVOICE" == strType) {
        return details_deposit_cheque(strServerID, strMyAcctID, strToNymID,
                                      strInstrument, strType);
    }
    else if ("PURSE" == strType) {
        return details_deposit_purse(strServerID, strMyAcctID, strToNymID,
                                     strInstrument, "");
    }
    else {
        otOut << "\n\nFailure: Unable to determine instrument type. "
                 "Expected CHEQUE, VOUCHER, INVOICE, or (cash) PURSE.\n";
        return -1;
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainDeposit()
{
    otOut << "Usage:   deposit --myacct YOUR_ACCT_ID  \n(OT will ask you to "
             "paste the instrument.)\nOptionally:    --mynym YOUR_NYM_ID "
             "\nOptionally:    --args \"indices 4,6,9\"\nOptionally:    --args "
             "\"indices all\"  (To deposit ALL cash tokens in your "
             "purse.)\nServer and NymID are usually determined based on "
             "MyAcct.\n If you supply optional indices, they must correspond "
             "to tokens in your cash purse.\n\n";

    // A bit complicated:
    //
    // If I specify MyPurse and MyAcct, then they MUST have the same asset type.
    //
    // If I specify MyNym and MyPurse, that is where we will look for the purse.
    //
    // If I specify MyAcct, and it's owned by a different Nym than MyNym, then
    // the cash
    // tokens will be reassigned from MyNym to MyAcct's Nym, before depositing.
    // Basically ALWAYS look up MyAcct's owner, and set HIM as the recipient
    // Nym.
    // (But still use MyNym, independently, to find the purse being deposited.)
    //
    // Must ALWAYS specify MyAcct because otherwise, where are you depositing
    // to?
    //
    // If MyNym isn't available, should use MyAcct's Nym.
    //
    // Shouldn't need to specify MyPurse, since we can ONLY deposit into MyAcct
    // of
    // the same type as MyAcct. Thus we should ignore any other asset types or
    // purses
    // since they couldn't possibly be deposited into MyAcct anyway.

    if (!VerifyExists("MyAcct")) return -1;

    string strServerID = OTAPI_Wrap::GetAccountWallet_ServerID(MyAcct);
    if (!VerifyStringVal(strServerID)) {
        otOut << "Failure: Unable to find Server ID based on myacct. Use: "
                 "--myacct ACCT_ID\n";
        otOut << "The designated asset account must be yours. OT will find "
                 "the Server based on the account.\n\n";
        return -1;
    }

    if (VerifyExists("Server", false) && !(Server == strServerID)) {
        otOut << "This account is on server ( " << strServerID
              << " -- the server is deduced based on the account), but the "
                 "default server is ( " << Server
              << " ). To override it, use:  --server " << strServerID << " \n";
        return -1;
    }

    return details_deposit(strServerID, MyAcct);
}

OT_COMMANDS_OT int32_t
    OT_Command::details_import_purse(const string& strInstrument,
                                     bool bHasPassword,
                                     const string& strPurseOwner)
{
    if (!VerifyStringVal(strInstrument)) {
        return -1;
    }

    string strServerID = OTAPI_Wrap::Instrmnt_GetServerID(strInstrument);

    if (!VerifyStringVal(strServerID)) {
        otOut << "\n\nFailure: Unable to determine server ID from purse.\n";
        return -1;
    }

    string strAssetID = OTAPI_Wrap::Instrmnt_GetAssetID(strInstrument);

    if (!VerifyStringVal(strAssetID)) {
        otOut << "\n\nFailure: Unable to determine asset type ID from purse.\n";
        return -1;
    }

    bool bActuallyHasPassword =
        OTAPI_Wrap::Purse_HasPassword(strServerID, strInstrument);

    if (bHasPassword != bActuallyHasPassword) {
        otOut << "The actual purse's opinion of whether or not it has a "
                 "password, doesn't match the caller's opinion.\n";
        return -1;
    }

    bool bImported = OTAPI_Wrap::Wallet_ImportPurse(
        strServerID, strAssetID, strPurseOwner, strInstrument);

    if (bImported) {
        otOut << "\n\n Success importing purse!\nServer: " << strServerID
              << "\nAsset Type: " << strAssetID << "\nNym: " << strPurseOwner
              << "\n\n";
        return 1;
    }

    return -1;
}

OT_COMMANDS_OT int32_t
    OT_Command::details_import_cash(const string& strInstrument)
{
    if (!VerifyStringVal(strInstrument)) {
        return -1;
    }

    string strType = OTAPI_Wrap::Instrmnt_GetType(strInstrument);

    if (!VerifyStringVal(strType)) {
        otOut << "\n\nFailure: Unable to determine instrument type. Expected "
                 "(cash) PURSE.\n";
        return -1;
    }

    string strServerID = OTAPI_Wrap::Instrmnt_GetServerID(strInstrument);

    if (!VerifyStringVal(strServerID)) {
        otOut << "\n\nFailure: Unable to determine server ID from purse.\n";
        return -1;
    }

    if ("PURSE" == strType) {
    }
    // Todo: case "TOKEN"
    //
    // NOTE: This is commented out because since it is guessing the NymID as
    // MyNym,
    // then it will just create a purse for MyNym and import it into that purse,
    // and
    // then later when doing a deposit, THAT's when it tries to DECRYPT that
    // token
    // and re-encrypt it to the SERVER's nym... and that's when we might find
    // out that
    // it never was encrypted to MyNym in the first place -- we had just assumed
    // it
    // was here, when we did the import. Until I can look at that in more
    // detail, it
    // will remain commented out.
    else {
        //    // This version supports cash tokens (instead of purse...)
        //    bool bImportedToken = importCashPurse(strServerID, MyNym,
        //                                   strAssetID, userInput, isPurse);
        //
        //    if (bImportedToken)
        //    {
        //        otOut << "\n\n Success importing cash token!\nServer: "
        //              << strServerID << "\nAsset Type: " << strAssetID
        //              << "\nNym: " << MyNym << "\n\n";
        //        return 1;
        //    }

        otOut << "\n\nFailure: Unable to determine instrument type. Expected "
                 "(cash) PURSE.\n";
        return -1;
    }

    // This tells us if the purse is password-protected. (Versus being owned
    // by a Nym.)
    bool bHasPassword =
        OTAPI_Wrap::Purse_HasPassword(strServerID, strInstrument);

    // Even if the Purse is owned by a Nym, that Nym's ID may not necessarily
    // be present on the purse itself (it's optional to list it there.)
    // OTAPI_Wrap::Instrmnt_GetRecipientUserID tells us WHAT the recipient User
    // ID
    // is, IF it's on the purse. (But does NOT tell us WHETHER there is a
    // recipient. The above function is for that.)
    string strPurseOwner = "";

    if (!bHasPassword) {
        strPurseOwner = OTAPI_Wrap::Instrmnt_GetRecipientUserID(strInstrument);
    }

    // Whether the purse was password-protected (and thus had no Nym ID)
    // or whether it does have a Nym ID (but it wasn't listed on the purse)
    // Then either way, in those cases strPurseOwner will still be nullptr.
    //
    // (The third case is that the purse is Nym protected and the ID WAS
    // available,
    // in which case we'll skip this block, since we already have it.)
    //
    // But even in the case where there's no Nym at all (password protected)
    // we STILL need to pass a Signer Nym ID into
    // OTAPI_Wrap::Wallet_ImportPurse.
    // So if it's still nullptr here, then we use --mynym to make the call.
    // And also, even in the case where there IS a Nym but it's not listed,
    // we must assume the USER knows the appropriate NymID, even if it's not
    // listed on the purse itself. And in that case as well, the user can
    // simply specify the Nym using --mynym.
    //
    // Bottom line: by this point, if it's still not set, then we just use
    // MyNym, and if THAT's not set, then we return failure.
    if (!VerifyStringVal(strPurseOwner)) {
        otOut << "\n\n The NymID isn't evident from the purse itself... "
                 "(listing it is optional.)\nThe purse may have no Nym at "
                 "all--it may instead be password-protected.) Either way, a "
                 "signer nym is still necessary, even for password-protected "
                 "purses.\n\n Trying MyNym...\n";

        if (!VerifyExists("MyNym")) {
            return -1;
        }

        strPurseOwner = MyNym;
    }

    return details_import_purse(strInstrument, bHasPassword, strPurseOwner);
}

OT_COMMANDS_OT int32_t OT_Command::mainImportCash()
{
    otOut << "Usage:   importcash   (OT will ask you to paste the "
             "instrument.)\nOptionally: importcash --mynym "
             "YOUR_NYM_ID\n\nAsset (Purse) ID and ServerID are both deduced "
             "from the cash purse that you're importing.\nNymID is also "
             "deduced, if necessary. (Otherwise, specify using --mynym.)\n\n";

    otOut << "You can import a PURSE (containing cash tokens.)\n";
    otOut << "Paste your financial instrument here, followed by a ~ by itself "
             "on a blank line: \n";

    string strInstrument = OT_CLI_ReadUntilEOF();

    if (!VerifyStringVal(strInstrument)) {
        return -1;
    }

    return details_import_cash(strInstrument);
}

// MyPurse and HisPurse will call the script even when not found, giving the
// script
// the opportunity to download the appropriate asset contract from the server,
// if
// necessary.
//
// Similarly, HisAcct works without being expected to be found in the wallet
// (since
// maybe it's HIS account and thus it's not IN your wallet...)
//
// Similarly, HisNym will call the script even when not found, giving the script
// the
// opportunity to download the appropriate pubkey ("check_user" aka "checknym")
// and
// continue operating.
//
// All of the above, plus Server, ALREADY attempt a partial match search.
// Therefore,
// it's not necessary to perform ANOTHER partial match, when the value comes
// from
// --server, --mynym, --hisnym, --mypurse, --hispurse, --myacct, or --hisacct.
//
// You only need to do partial matches when you get values from ELSEWHERE, such
// as
// from custom arguments, or user-pasted input.
//
// Therefore, add a function for downloading a Nym's pubkey if not already in
// the wallet,
// and one that downloads an asset contract if not already in the wallet

OT_COMMANDS_OT string OT_Command::details_export_cash(
    const string& strServerID, const string& strFromNymID,
    const string& strAssetTypeID, string& strHisNymID, const string& strIndices,
    bool bPasswordProtected, string& strRetainedCopy)
{

    string strLocation = "\n details_export_cash";

    // The first three arguments are for loading up the purse we're exporting
    // FROM.
    //
    // Then strHisNymID is for the Nym we're exporting TO (if bPasswordProtected
    // is false.)
    //
    // Then strIndices contains the indices for the tokens to export.
    //
    // Then bPasswordProtected tells us whether to export to strHisNymID,
    // or to create a password-protected purse and export to that instead.
    // (In which case, the purse itself becomes the "owner" and can be passed
    // wherever we would normally pass a NymID as a purse owner.)
    //
    // if bPasswordProtected is false, then strHisNymID needs to contain the
    // recipient Nym.
    // This will contain MyNym by default, if --hisnym wasn't used at the
    // command line.

    string strContract = MadeEasy::load_or_retrieve_contract(
        strServerID, strFromNymID, strAssetTypeID);

    if (!VerifyStringVal(strContract)) {
        otOut << strLocation
              << ": Unable to load asset contract: " << strAssetTypeID << "\n";
        return "";
    }

    bool bLoadedPurse = false;

    string strInstrument =
        OTAPI_Wrap::LoadPurse(strServerID, strAssetTypeID, strFromNymID);

    if (!VerifyStringVal(strInstrument)) {
        otOut << strLocation << ": Unable to load purse from local storage. "
                                "Does it even exist?\n";
        return "";
    }

    bLoadedPurse = true;

    // Below this point, we know that strInstrument contains the purse as we
    // loaded it from local storage.
    // If it WAS from local storage, then there's a chance that strIndices
    // contains "4, 6, 2" etc.
    // If that's the case, then we need to iterate through the purse, and add
    // the denoted token IDs to
    // a vector (selectedTokens) and pass it into exportCashPurse.

    vector<string> vecSelectedTokenIDs;

    // the user might have wanted to export only selected indices,
    // rather than ALL the tokens in the purse.
    // So we'll loop through the purse and add any relevant IDs to the
    // "selected" list, since the actual Token IDs must be passed.
    if (bLoadedPurse) {

        // Loop through purse contents...
        int32_t nCount =
            OTAPI_Wrap::Purse_Count(strServerID, strAssetTypeID, strInstrument);
        if (nCount < 0) {
            otOut << strLocation << ": Error: Unexpected bad value returned "
                                    "from OT_API_Purse_Count.\n\n";
            return "";
        }

        if (nCount < 1) {
            otOut << strLocation
                  << ": The purse is empty, so you can't export it.\n\n";
            return "";
        }
        else {
            // Make a COPY of the purse for this loop, so we can iterate it and
            // find the
            // appropriate Token IDs...

            string strPurse = strInstrument;

            if (VerifyStringVal(strIndices)) {
                int32_t nIndex = -1;

                while (nCount > 0) {
                    --nCount;
                    ++nIndex;

                    string strToken = OTAPI_Wrap::Purse_Peek(
                        strServerID, strAssetTypeID, strFromNymID, strPurse);

                    if (!VerifyStringVal(strToken)) {
                        otOut << strLocation << ": Error: OT_API_Purse_Peek "
                                                "unexpectedly returned nullptr "
                                                "instead of token.\n";
                        return "";
                    }

                    string strNewPurse = OTAPI_Wrap::Purse_Pop(
                        strServerID, strAssetTypeID, strFromNymID, strPurse);

                    if (!VerifyStringVal(strNewPurse)) {
                        otOut << strLocation << ": Error: OT_API_Purse_Pop "
                                                "unexpectedly returned nullptr "
                                                "instead of updated purse.\n";
                        return "";
                    }

                    strPurse = strNewPurse;

                    string strTokenID = OTAPI_Wrap::Token_GetID(
                        strServerID, strAssetTypeID, strToken);

                    if (!VerifyStringVal(strTokenID)) {
                        otOut << strLocation << ": Error while exporting "
                                                "purse: bad strTokenID.\n";
                        return "";
                    }

                    // If there are no indices, then do them all. (Thus push
                    // this one,
                    // as we push every one.)
                    // OR, If there are indices, and the current index is FOUND
                    // on the
                    // vector of indices, then we push this one (since it was
                    // selected.)
                    if (!("all" == strIndices) &&
                        OTAPI_Wrap::NumList_VerifyQuery(
                            strIndices, std::to_string(nIndex))) {
                        vecSelectedTokenIDs.push_back(strTokenID);
                    }
                }
            }

            string strExportedCashPurse = MadeEasy::exportCashPurse(
                strServerID, strAssetTypeID, strFromNymID, strInstrument,
                vecSelectedTokenIDs, strHisNymID, bPasswordProtected,
                strRetainedCopy);

            return strExportedCashPurse;
        }
    }

    return "";
}

OT_COMMANDS_OT int32_t OT_Command::mainExportCash()
{
    otOut << "Usage:   exportcash --mypurse ASSET_TYPE_ID --mynym YOUR_NYM_ID "
             "--hisnym RECIPIENT_NYM_ID --server SERVER_ID\nOptionally:    "
             "--args \"indices 4,6,9\"\nOptionally:    --args \"passwd true\"  "
             "(To create a password-protected purse.)\n\n(If you create a "
             "password-protected purse, then HisNym will be ignored and can be "
             "left out.)\nIf you supply optional indices, they must correspond "
             "to tokens in your cash purse.\n\n";

    if (!VerifyExists("Server") || !VerifyExists("MyNym") ||
        !VerifyExists("MyPurse"))
        return -1;

    string strIndices = "";
    bool bPasswordProtected = false;

    if (VerifyExists("Args", false)) {
        strIndices = OT_CLI_GetValueByKey(Args, "indices");

        string strPasswordProtected = OT_CLI_GetValueByKey(Args, "passwd");

        if (VerifyStringVal(strPasswordProtected) &&
            ("true" == strPasswordProtected)) {
            bPasswordProtected = true;
        }
    }

    string strHisNym = MyNym;

    if (!bPasswordProtected && VerifyExists("HisNym", false)) {
        otOut << "HisNym was provided at the command line.\nDo you wish to "
                 "export this cash to HisNym instead of MyNym?\n\n";
        otOut << "HisNym is: " << HisNym << "\n MyNym is: " << MyNym
              << "\n\n Type yes to use HisNym, or no to use MyNym.[no]: ";

        string strAnswer = OT_CLI_ReadLine();

        if (VerifyStringVal(strAnswer) &&
            (("y" == strAnswer) || ("yes" == strAnswer))) {
            strHisNym = HisNym;
        }
    }

    string strServerID = Server;
    string strMyNymID = MyNym;
    string strMyPurse = MyPurse;
    string strRetainedCopy = "";

    string strExportedPurse =
        details_export_cash(strServerID, strMyNymID, strMyPurse, strHisNym,
                            strIndices, bPasswordProtected, strRetainedCopy);

    if (VerifyStringVal(strExportedPurse)) {
        cout << "\n" << strExportedPurse << "\n\n";
        otOut << "Success exporting cash purse!\n\n";
        return 1;
    }

    return -1;
}

// Pass in the exact indices, or an amount, and this function will return true;
// or false.
//
// If you pass the indices, this function returns true if those exact indices
// exist. In that case, this function will also set lAmount to the total.
//
// If, instead, you pass lAmount and a blank indices, this function will try to
// determine the indices that would create lAmount, if they were selected. If it
// returns true, strIndices, will contain the result, at the end of it all.
//
OT_COMMANDS_OT bool OT_Command::purse_get_indices_or_amount(
    const string& strServerID, const string& strAssetTypeID,
    const string& strMyNymID, int64_t& lAmount, string& strIndices)
{

    string strLocation = "\n purse_get_indices_or_amount";

    string strLoopIndices = "";
    int64_t lAmountRemaining = lAmount;
    int64_t lLoopAmount = 0;

    // lLoopAmount is where we will start at 0, and keep the running total of
    // the value for the selected indices, as we loop.
    // lAmountRemaining is where we will start at lAmount, and then SUBTRACT the
    // value of each selected index, as we loop.
    // If we are trying to determine the indices based on lAmount, we'll SEE if
    // the current index is less-than-or-equal-to
    // lAmountRemaining, and if so, we'll concat that index to our output and
    // subtract the denomination of the token at the
    // current index from lAmountRemaining.
    // If bFindAmountFromIndices is true then lAmount is already 0 and then
    // lAmountRemaining is already 0 as well.

    bool bFindIndicesFromAmount = false;
    bool bFindAmountFromIndices = false;

    if (VerifyStringVal(strIndices) && (0 == lAmount)) {
        bFindAmountFromIndices = true;
    }
    else if (!VerifyStringVal(strIndices) && (0 != lAmount)) {
        bFindIndicesFromAmount = true;
    }
    else {
        otOut << strLocation << ": Can only pass indices, OR amount to this "
                                "function, but not both (the other becomes "
                                "output.)\n";
        return false;
    }

    otOut << "strServerID: " << strServerID << "\n";
    otOut << "strAssetTypeID: " << strAssetTypeID << "\n";
    otOut << "strMyNymID: " << strMyNymID << "\n";

    string strLocalPurse =
        OTAPI_Wrap::LoadPurse(strServerID, strAssetTypeID, strMyNymID);

    bool bLoadedPurse = VerifyStringVal(strLocalPurse);
    if (!bLoadedPurse) {
        otOut << strLocation
              << ": Unable to load local purse. Does it even exist?\n";
        return false;
    }

    // Loop through purse contents...
    int32_t nCount =
        OTAPI_Wrap::Purse_Count(strServerID, strAssetTypeID, strLocalPurse);
    if (nCount < 0) {
        otOut << strLocation << ": Error: Unexpected bad value returned "
                                "from OT_API_Purse_Count.\n\n";
        return false;
    }

    if (nCount < 1) {
        otOut << strLocation << ": The purse is empty.\n\n";
        return false;
    }

    // Make a copy of the purse passed in, so we can iterate it and find
    // the
    // appropriate Token IDs...

    string strPurse = strLocalPurse;

    int32_t nIndex = -1;

    while (nCount > 0) {
        --nCount;
        ++nIndex;

        // NOTE: Owner can ONLY be strMyNymID in here, since

        // is only true in the case where we LOADED the purse from local
        // storage.
        // (Therefore this DEFINITELY is not a password-protected
        // purse.)
        string strToken = OTAPI_Wrap::Purse_Peek(strServerID, strAssetTypeID,
                                                 strMyNymID, strPurse);

        if (!VerifyStringVal(strToken)) {
            otOut << strLocation << ": Error: OT_API_Purse_Peek "
                                    "unexpectedly returned nullptr "
                                    "instead of token.\n";
            return false;
        }

        string strNewPurse = OTAPI_Wrap::Purse_Pop(strServerID, strAssetTypeID,
                                                   strMyNymID, strPurse);

        if (!VerifyStringVal(strNewPurse)) {
            otOut << strLocation << ": Error: OT_API_Purse_Pop "
                                    "unexpectedly returned nullptr "
                                    "instead of updated purse.\n";
            return false;
        }

        strPurse = strNewPurse;

        string strTokenID =
            OTAPI_Wrap::Token_GetID(strServerID, strAssetTypeID, strToken);
        if (!VerifyStringVal(strTokenID)) {
            otOut << strLocation << ": Error: bad strTokenID.\n";
            return false;
        }
        int64_t lDenomination = OTAPI_Wrap::Token_GetDenomination(
            strServerID, strAssetTypeID, strToken);
        if (0 > lDenomination) {
            otOut << "Error while showing purse: bad lDenomination.\n";
            return false;
        }
        int32_t nSeries =
            OTAPI_Wrap::Token_GetSeries(strServerID, strAssetTypeID, strToken);
        if (0 > nSeries) {
            otOut << "Error while showing purse: bad nSeries.\n";
            return false;
        }
        time64_t tValidFrom = OTAPI_Wrap::Token_GetValidFrom(
            strServerID, strAssetTypeID, strToken);
        if (OT_TIME_ZERO > tValidFrom) {
            otOut << "Error while showing purse: bad tValidFrom.\n";
            return false;
        }
        time64_t tValidTo =
            OTAPI_Wrap::Token_GetValidTo(strServerID, strAssetTypeID, strToken);
        if (OT_TIME_ZERO > tValidTo) {
            otOut << "Error while showing purse: bad tValidTo.\n";
            return false;
        }
        time64_t lTime = OTAPI_Wrap::GetTime();
        if (OT_TIME_ZERO > lTime) {
            otOut << "Error while showing purse: bad lTime.\n";
            return false;
        }
        string strStatus = (lTime > tValidTo) ? "expired" : "valid";

        // lLoopAmount is where we will start at 0, and keep the running
        // total of the value for the selected indices, as we loop.
        // lAmountRemaining is where we will start at lAmount, and then
        // SUBTRACT the value of each selected index, as we loop.
        // If we are trying to determine the indices based on lAmount,
        // we'll SEE if value at the current index is
        // less-than-or-equal-to
        // lAmountRemaining, and if so, we'll concat that index to our
        // output and subtract the denomination of the token at the
        // current index from lAmountRemaining.
        // If bFindAmountFromIndices is true then lAmount is already 0
        // and then lAmountRemaining is already 0 as well.

        // TODO:  This would work better if the tokens were sorted. In
        // the future, pull the tokens onto a map or something,
        // and then SORT IT based on denomination of token. THEN go
        // through the tokens from largest to smallest and do what we're
        // doing here in this loop. And then it will work perfectly
        // everytime. Todo.  This version will work okay, but there will
        // be
        // times it fails and ends up withdrawing cash when it didn't
        // really need to. TOdo.

        if (lTime > tValidTo) {
            otOut << strLocation << ": Skipping: Token is " << strStatus
                  << ".\n";
        }
        else if (bFindAmountFromIndices) {
            if (VerifyStringVal(strIndices) &&
                (("all" == strIndices) ||
                 OTAPI_Wrap::NumList_VerifyQuery(strIndices,
                                                 std::to_string(nIndex)))) {
                string strTempIndices = OTAPI_Wrap::NumList_Add(
                    strLoopIndices, std::to_string(nIndex));
                strLoopIndices = strTempIndices;
                lLoopAmount += lDenomination;
            }
        }
        else if (bFindIndicesFromAmount) {
            if (lAmountRemaining > 0 && lDenomination <= lAmountRemaining) {
                string strTempIndices = OTAPI_Wrap::NumList_Add(
                    strLoopIndices, std::to_string(nIndex));
                strLoopIndices = strTempIndices;
                lLoopAmount += lDenomination;
                lAmountRemaining -= lDenomination;

                if (0 == lAmountRemaining) {
                    break;
                }
            }
        }
    }

    if (bFindAmountFromIndices) {
        lAmount = lLoopAmount;
        return true;
    }

    if (bFindIndicesFromAmount) {
        strIndices = strLoopIndices;
        lAmount = lAmountRemaining;

        if (0 == lAmountRemaining) {
            return true;
        }

        strIndices = "";
    }

    return false;
}

OT_COMMANDS_OT bool OT_Command::withdraw_and_send_cash(
    const string& strMyAcctID, string& strHisNymID, const string& strMemo,
    const string& strAmount)
{
    string strLocation = "withdraw_and_send_cash";

    if (!VerifyStringVal(strMyAcctID) || !VerifyStringVal(strHisNymID) ||
        !VerifyStringVal(strAmount)) {
        otOut << strLocation << ": Failure: Missing one of: strMyAcctID ("
              << strMyAcctID << "), strHisNymID (" << strHisNymID
              << "), or strAmount (" << strAmount << ")\n";
        return false;
    }

    string strMyNymID = OTAPI_Wrap::GetAccountWallet_NymID(strMyAcctID);
    if (!VerifyStringVal(strMyNymID)) {
        otOut << strLocation
              << ": Failure: Unable to find NymID based on strMyAcctID ("
              << strMyAcctID << ").\n";
        return false;
    }

    string strServerID = OTAPI_Wrap::GetAccountWallet_ServerID(strMyAcctID);
    if (!VerifyStringVal(strServerID)) {
        otOut << strLocation
              << ": Failure: Unable to find Server ID based on strMyAcctID ("
              << strMyAcctID << ").\n";
        return false;
    }

    string strAssetTypeID =
        OTAPI_Wrap::GetAccountWallet_AssetTypeID(strMyAcctID);
    if (!VerifyStringVal(strAssetTypeID)) {
        otOut
            << strLocation
            << ": Failure: Unable to find Asset Type ID based on strMyAcctID ("
            << strMyAcctID << ").\n";
        return false;
    }

    int64_t lAmount = int64_t(0);

    if (VerifyStringVal(strAmount)) {
        lAmount = std::stoll(strAmount);
    }

    if (lAmount <= 0) {
        otOut << strLocation << ": Invalid value: strAmount (" << strAmount
              << ")\n";
        return false;
    }

    string strTempMemo = "";
    string strResponse = "";
    string strIndices = "";
    bool bPasswordProtected = false;

    if (VerifyStringVal(strMemo)) {
        strTempMemo = strMemo;
    }

    if (1 == OT_Command::details_send_cash(
                 strResponse, strServerID, strAssetTypeID, strMyNymID,
                 strMyAcctID, strHisNymID, strTempMemo, strAmount, strIndices,
                 bPasswordProtected)) {
        return true;
    }

    return false;
}

OT_COMMANDS_OT int32_t OT_Command::details_send_cash(
    string& strResponse, const string& strServerID,
    const string& strAssetTypeID, const string& strMyNymID,
    const string& strMyAcctID, string& strHisNymID, const string&,
    const string& strAmount, string& strIndices, bool bPasswordProtected)
{

    string strLocation = "\n details_send_cash";

    int64_t lStartAmount = int64_t(0);

    if (VerifyStringVal(strAmount)) {
        lStartAmount = std::stoll(strAmount);
    }

    int64_t lAmount = lStartAmount;

    // What we want to do from here is, see if we can send the cash purely using
    // cash we already
    // have in the local purse.
    // If so, we just package it up and send it off using send_user_payment.
    //
    // But if we do NOT have the proper cash tokens in the local purse to send,
    // then we need to
    // withdraw enough tokens until we do, and then try sending again.

    bool bAmountFromIndices = VerifyStringVal(strIndices);

    bool bGotData = purse_get_indices_or_amount(
        strServerID, strAssetTypeID, strMyNymID, lAmount, strIndices);

    if (!bGotData) // NOT ENOUGH CASH FOUND IN EXISTING PURSE TO MATCH THE
                   // AMOUNT (or indices were specified and not found.)
    {
        if (bAmountFromIndices) {
            // If we were starting out with indices, and those indices failed to
            // be found, then there's
            // nothing more we can do. They aren't there.
            otOut << strLocation
                  << ": Sorry, those indices weren't found: " << strIndices
                  << "\nPerhaps try: 'opentxs showpurse'\n";
            return -1;
        }

        // By this point, it means we started with the amount, and tried to use
        // it to find the
        // appropriate indices -- and they weren't there. This means we need to
        // withdraw more cash
        // from the server before we can send it, since we don't have enough in
        // our purse already.
        int32_t nWithdraw = details_withdraw_cash(strMyAcctID, lAmount);

        if (1 != nWithdraw) {
            otOut << strLocation << ": Tried to withdraw cash, but failed.\n";
            return -1;
        }

        // Okay we performed the withdrawal, so now let's see again if we can
        // extract the proper
        // cash tokens from the local purse. We'll load it up again...
        lAmount = lStartAmount;

        bGotData = purse_get_indices_or_amount(strServerID, strAssetTypeID,
                                               strMyNymID, lAmount, strIndices);

        if (!bGotData) {
            otOut << strLocation
                  << ": Unable to get enough cash into the local purse to "
                     "send, EVEN after performing a cash withdrawal. "
                     "(Failure.)\n strIndices: " << strIndices << " lAmount: "
                  << OTAPI_Wrap::FormatAmount(strAssetTypeID, lAmount) << "\n";
            return -1;
        }
    }

    // By this point, bGotData is definitely true, meaning there are already
    // enough cash tokens in the purse
    // to send, or there are now since performing a withdrawal. Either way, we
    // should be good to go...

    // Returns the exported cash purse, with strRetainedCopy containing the
    // sender's version of that.
    string strRetainedCopy = "";
    string strExportedCashPurse = details_export_cash(
        strServerID, strMyNymID, strAssetTypeID, strHisNymID, strIndices,
        bPasswordProtected, strRetainedCopy);
    int32_t nReturnVal = -1;

    if (VerifyStringVal(strExportedCashPurse)) {
        strResponse =
            MadeEasy::send_user_cash(strServerID, strMyNymID, strHisNymID,
                                     strExportedCashPurse, strRetainedCopy);

        nReturnVal = VerifyMessageSuccess(strResponse);

        if (1 != nReturnVal) {
            // It failed sending the cash to the recipient Nym.
            // Re-import strRetainedCopy back into the sender's cash purse.
            bool bImported = OTAPI_Wrap::Wallet_ImportPurse(
                strServerID, strAssetTypeID, strMyNymID, strRetainedCopy);

            if (bImported) {
                otOut << "\n\nFailed sending cash, but at least: success "
                         "re-importing purse.\nServer: " << strServerID
                      << "\nAsset Type: " << strAssetTypeID
                      << "\nNym: " << strMyNymID << "\n\n";
            }
            else {
                otOut << "\n\n Failed sending cash AND failed re-importing "
                         "purse.\nServer: " << strServerID
                      << "\nAsset Type: " << strAssetTypeID
                      << "\nNym: " << strMyNymID
                      << "\n\nPurse (SAVE THIS SOMEWHERE!):\n\n"
                      << strRetainedCopy << "\n";
            }
        }
    }

    return nReturnVal;
}

OT_COMMANDS_OT int32_t OT_Command::mainNewKey()
{
    string strKey = OTAPI_Wrap::CreateSymmetricKey();

    if (VerifyStringVal(strKey)) {

        otOut << "\n";

        cout << strKey << "\n";

        otOut << "\n";

        return 1;
    }

    return -1;
}

// case ("CHEQUE")
// case ("VOUCHER")
// case ("INVOICE")
// case ("PURSE")
//
// If one of the above types is passed in, then the payment will only be handled
// if the type matches.
//
// But if "ANY" is passed in, then the payment will be handled for any of them.
//

OT_COMMANDS_OT int32_t
    OT_Command::handle_payment_index(const string& strMyAcctID, int32_t nIndex,
                                     const string& strPaymentType,
                                     const string& strInbox)
{
    if (!VerifyStringVal(strMyAcctID)) {
        otOut << "Failure: strMyAcctID not a valid string.\n";
        return -1;
    }

    string strMyNymID = OTAPI_Wrap::GetAccountWallet_NymID(strMyAcctID);
    if (!VerifyStringVal(strMyNymID)) {
        otOut << "Failure: Unable to find NymID based on myacct. Use: --myacct "
                 "ACCT_ID\n";
        otOut << "The designated asset account must be yours. OT will find the "
                 "Nym based on the account.\n\n";
        return -1;
    }

    string strServerID = OTAPI_Wrap::GetAccountWallet_ServerID(strMyAcctID);
    if (!VerifyStringVal(strServerID)) {
        otOut << "Failure: Unable to find Server ID based on myacct. Use: "
                 "--myacct ACCT_ID\n";
        otOut << "The designated asset account must be yours. OT will find the "
                 "Server based on the account.\n\n";
        return -1;
    }
    string strInstrument = "";

    if (-1 == nIndex) {
        otOut << "Please paste the instrument, followed by an EOF or a ~ by "
                 "itself on a blank line:\n\n";
        strInstrument = OT_CLI_ReadUntilEOF();

        if (!VerifyStringVal(strInstrument)) {
            otOut << "\n\n Sorry, You must paste the instrument, in order to "
                     "process it. Or, specify an index in the\npayments inbox "
                     "using the option:  --args \"index "
                     "INDEX_OF_INVOICE\".\n\n";
            return -1;
        }
    }
    else {
        strInstrument = MadeEasy::get_payment_instrument(
            strServerID, strMyNymID, nIndex, strInbox);

        if (!VerifyStringVal(strInstrument)) {
            otOut << "\n\n Unable to get payment instrument based on index: "
                  << nIndex << ".\n\n";
            return -1;
        }
    }

    // By this point, strInstrument is a valid string (whether we got it from
    // the payments inbox,
    // or whether we got it from stdin.)
    string strType = OTAPI_Wrap::Instrmnt_GetType(strInstrument);

    if (!VerifyStringVal(strType)) {
        otOut << "\n\nFailure: Unable to determine strInstrument's type. "
                 "Expected CHEQUE, VOUCHER, INVOICE, or (cash) PURSE.\n";
        return -1;
    }

    string strIndexErrorMsg = "";

    if (-1 != nIndex) {
        strIndexErrorMsg = "at index " + std::to_string(nIndex) + " ";
    }

    // If there's a payment type,
    // and it's not "ANY", and it's the wrong type,
    // then skip this one.
    if (VerifyStringVal(strPaymentType) && (strPaymentType != "ANY") &&
        (strPaymentType != strType)) {
        if ((("CHEQUE" == strPaymentType) && ("VOUCHER" == strType)) ||
            (("VOUCHER" == strPaymentType) && ("CHEQUE" == strType))) {
            // in this case we allow it to drop through.
        }
        else {
            otOut << "The instrument " << strIndexErrorMsg << "is not a "
                  << strPaymentType << ". (It's a " << strType
                  << ". Skipping.)\n";
            return -1;
        }
    }

    // Note: I USED to check the ASSET TYPE ID here, but then I removed it,
    // since
    // details_deposit_cheque() already verifies that (so I don't need to do it
    // twice.)

    // By this point, we know the invoice has the right asset type for the
    // account
    // we're trying to use (to pay it from.)
    //
    // But we need to make sure the invoice is made out to strMyNymID (or to no
    // one.)
    // Because if it IS endorsed to a Nym, and strMyNymID is NOT that nym, then
    // the
    // transaction will fail. So let's check, before we bother sending it...
    string strRecipientUserID =
        OTAPI_Wrap::Instrmnt_GetRecipientUserID(strInstrument);

    // Not all instruments have a specified recipient. But if they do, let's
    // make
    // sure the Nym matches.
    if (VerifyStringVal(strRecipientUserID) &&
        (strRecipientUserID != strMyNymID)) {
        otOut << "The instrument " << strIndexErrorMsg
              << "is endorsed to a specific recipient (" << strRecipientUserID
              << ") and that doesn't match the account's owner Nym ("
              << strMyNymID << "). (Skipping.)\nTry specifying a different "
                               "account, using --myacct ACCT_ID \n";
        return -1;
    }

    // At this point I know the invoice isn't made out to anyone, or if it is,
    // it's properly
    // made out to the owner of the account which I'm trying to use to pay the
    // invoice from.
    // So let's pay it!  P.S. strRecipientUserID might be nullptr, but
    // strMyNymID is guaranteed to be good.

    string strInstrumentAssetType =
        OTAPI_Wrap::Instrmnt_GetAssetID(strInstrument);
    string strAccountAssetID =
        OTAPI_Wrap::GetAccountWallet_AssetTypeID(strMyAcctID);

    if (VerifyStringVal(strInstrumentAssetType) &&
        (strAccountAssetID != strInstrumentAssetType)) {
        otOut << "The instrument at index " << nIndex
              << " has a different asset type than the selected account. "
                 "(Skipping.)\nTry specifying a different account, using "
                 "--myacct ACCT_ID \n";
        return -1;
    }

    time64_t tFrom = OTAPI_Wrap::Instrmnt_GetValidFrom(strInstrument);
    time64_t tTo = OTAPI_Wrap::Instrmnt_GetValidTo(strInstrument);
    time64_t tTime = OTAPI_Wrap::GetTime();

    if (tTime < tFrom) {
        otOut << "The instrument at index " << nIndex
              << " is not yet within its valid date range. (Skipping.)\n";
        return -1;
    }
    if (tTo > OT_TIME_ZERO && tTime > tTo) {
        otOut << "The instrument at index " << nIndex
              << " is expired. (Moving it to the record box.)\n";

        // Since this instrument is expired, remove it from the payments inbox,
        // and move to record box.

        // Note: this harvests
        if ((nIndex >= 0) && OTAPI_Wrap::RecordPayment(strServerID, strMyNymID,
                                                       true, nIndex, true)) {
            return 0;
        }

        return -1;
    }

    // TODO, IMPORTANT: After the below deposits are completed successfully, the
    // wallet
    // will receive a "successful deposit" server reply. When that happens, OT
    // (internally)
    // needs to go and see if the deposited item was a payment in the payments
    // inbox. If so,
    // it should REMOVE it from that box and move it to the record box.
    //
    // That's why you don't see me messing with the payments inbox even when
    // these are successful.
    // They DO need to be removed from the payments inbox, but just not here in
    // the script. (Rather,
    // internally by OT itself.)
    if ("CHEQUE" == strType || "VOUCHER" == strType || "INVOICE" == strType) {
        return details_deposit_cheque(strServerID, strMyAcctID, strMyNymID,
                                      strInstrument, strType);
    }

    if ("PURSE" == strType) {
        int32_t nDepositPurse = details_deposit_purse(
            strServerID, strMyAcctID, strMyNymID, strInstrument, "");

        // if nIndex !=  -1, go ahead and call RecordPayment on the purse at
        // that index, to
        // remove it from payments inbox and move it to the recordbox.
        if ((nIndex != -1) && (1 == nDepositPurse)) {
            OTAPI_Wrap::RecordPayment(strServerID, strMyNymID, true, nIndex,
                                      true);
        }

        return nDepositPurse;
    }

    otOut << "\nSkipping this instrument: Expected CHEQUE, VOUCHER, INVOICE, "
             "or (cash) PURSE.\n";

    return -1;
}

// COULD have been named
// "details_accept_specific_instruments_of_specific_types_from_the_payment_inbox."
OT_COMMANDS_OT int32_t
    OT_Command::accept_from_paymentbox(const string& strMyAcctID,
                                       const string& strIndices,
                                       const string& strPaymentType)
{
    if (!VerifyStringVal(strMyAcctID)) {
        otOut << "Failure: strMyAcctID not a valid string.\n";
        return -1;
    }

    string strMyNymID = OTAPI_Wrap::GetAccountWallet_NymID(strMyAcctID);
    if (!VerifyStringVal(strMyNymID)) {
        otOut << "Failure: Unable to find NymID based on myacct. Use: --myacct "
                 "ACCT_ID\n";
        otOut << "The designated asset account must be yours. OT will find the "
                 "Nym based on the account.\n\n";
        return -1;
    }

    string strServerID = OTAPI_Wrap::GetAccountWallet_ServerID(strMyAcctID);
    if (!VerifyStringVal(strServerID)) {
        otOut << "Failure: Unable to find Server ID based on myacct. Use: "
                 "--myacct ACCT_ID\n";
        otOut << "The designated asset account must be yours. OT will find the "
                 "Server based on the account.\n\n";
        return -1;
    }

    string strInbox = OTAPI_Wrap::LoadPaymentInbox(strServerID, strMyNymID);
    if (!VerifyStringVal(strInbox)) {
        otOut << "\n\n accept_from_paymentbox:  OT_API_LoadPaymentInbox "
                 "Failed.\n\n";
        return -1;
    }

    int32_t nCount = OTAPI_Wrap::Ledger_GetCount(strServerID, strMyNymID,
                                                 strMyNymID, strInbox);
    if (0 > nCount) {
        otOut
            << "Unable to retrieve size of payments inbox ledger. (Failure.)\n";
        return -1;
    }

    int32_t nIndicesCount =
        VerifyStringVal(strIndices) ? OTAPI_Wrap::NumList_Count(strIndices) : 0;

    // Either we loop through all the instruments and accept them all, or
    // we loop through all the instruments and accept the specified indices.
    //
    // (But either way, we loop through all the instruments.)
    //
    for (int32_t nIndex = (nCount - 1); nIndex >= 0; --nIndex) {
        bool bContinue = false;

        // - If indices are specified, but the current index is not on
        //   that list, then continue...
        //
        // - If NO indices are specified, accept all the ones matching MyAcct's
        // asset type.
        if ((nIndicesCount > 0) &&
            !OTAPI_Wrap::NumList_VerifyQuery(strIndices,
                                             std::to_string(nIndex))) {
            bContinue = true;
        }
        else if (!bContinue) {
            handle_payment_index(strMyAcctID, nIndex, strPaymentType, strInbox);
        }
    }

    return 1;
}

OT_COMMANDS_OT int32_t
    OT_Command::details_accept_invoices(const string& strMyAcctID,
                                        const string& strIndices)
{
    return accept_from_paymentbox(strMyAcctID, strIndices, "INVOICE");
}

OT_COMMANDS_OT int32_t OT_Command::mainAcceptInvoices()
{
    otOut << "Usage:   acceptinvoices --myacct FROM_ACCT --args \"indices "
             "3,6,8\"  \n (Sample indices are shown.)\nThe invoice and myacct "
             "must both have same asset type. If indices\nare not specified "
             "for the payments inbox, then OT will pay ALL invoices in "
             "it\nthat have the same asset type as MyAcct.\n\n";

    if (VerifyExists("MyAcct")) {
        string strIndices = "";

        if (VerifyExists("Args", false)) {
            strIndices = OT_CLI_GetValueByKey(Args, "indices");
        }

        return details_accept_invoices(MyAcct, strIndices);
    }

    return -1;
}

OT_COMMANDS_OT int32_t
    OT_Command::details_accept_payments(const string& strMyAcctID,
                                        const string& strIndices)
{
    int32_t nAcceptedPurses =
        accept_from_paymentbox(strMyAcctID, strIndices, "PURSE");
    int32_t nAcceptedCheques =
        accept_from_paymentbox(strMyAcctID, strIndices, "CHEQUE");

    // Note: NOT invoices.

    // FIX: this OR should become AND so we can detect any failure
    if (nAcceptedPurses >= 0 || nAcceptedCheques >= 0) {
        return 1;
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainAcceptPayments()
{
    otOut << "Usage:   acceptpayments --myacct INTO_ACCT --args \"indices "
             "3,6,8\"  \n (Sample indices are shown.)\nThe payment instrument "
             "and myacct must both have same asset type. If indices\nare not "
             "specified for the payments inbox, then OT will accept ALL "
             "payment\ninstruments in it that have the same asset type as "
             "MyAcct.\n\n";

    if (VerifyExists("MyAcct")) {
        string strIndices = "";

        if (VerifyExists("Args", false)) {
            strIndices = OT_CLI_GetValueByKey(Args, "indices");
        }

        return details_accept_payments(MyAcct, strIndices);
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainPayInvoice()
{
    otOut << "Usage:   payinvoice --myacct FROM_ACCT --args \"index "
             "INVOICE_INDEX\" \nThe invoice and myacct must both have same "
             "asset type. If an index is not\nspecified for the payments "
             "inbox, then OT will ask you to paste an invoice.\n\n";

    if (!VerifyExists("MyAcct")) {
        return -1;
    }
    int32_t nIndex = -1;

    if (VerifyExists("Args")) {
        string strIndex = OT_CLI_GetValueByKey(Args, "index");

        if (VerifyStringVal(strIndex)) {
            int32_t nTempIndex = std::stol(strIndex);

            if (nTempIndex >= 0) {
                nIndex = nTempIndex;
            }
        }
    }
    otOut << "\n\n";
    int32_t nPaidInvoice = handle_payment_index(MyAcct, nIndex, "INVOICE", "");

    if (1 != nPaidInvoice) return -1;

    // Success!  (Therefore, remove from payments inbox,
    // and move to record box.)

    // Should I bother moving the invoice from the payments inbox to the
    // record box?
    //
    // Update: Okay for now, I'm using an API call here (RecordPayment)
    // which moves the invoice.
    // HOWEVER, in the int64_t term, we don't want to do it here. Rather, we
    // want to do it inside OT while
    // it's processesing the server reply for your cheque (invoice) deposit.
    // For example what if there's
    // a network problem and we don't process that reply here now? There'll
    // still be a copy of the reply
    // in the Nymbox and it will still get processed at a future time... and
    // THAT's when we need to move
    // the record, not here. (But this is what we'll do for now.)
    //
    // OTAPI_Wrap::RecordPayment(const OTIdentifier& SERVER_ID,
    //                              const OTIdentifier& USER_ID,
    //                              bool bIsInbox, // true == payments
    // inbox. false == payments outbox.
    //                              int32_t  nIndex)   // removes payment
    // instrument (from payments in or out box) and moves to record box.
    //        if (!OTAPI_Wrap::RecordPayment(strServerID, strMyNymID, ))
    // {; }

    // UPDATE:
    /*
    - In my Payments Inbox, there could be a cheque or invoice. Either way,
    when I deposit the cheque or
    pay the invoice, the chequeReceipt goes back to the signer's asset
    account's inbox.
    - When he accepts the chequeReceipt (during a processInbox) and WHEN HE
    GETS THE "SUCCESS" REPLY to that
    processInbox, is when the chequeReceipt should be moved from his inbox
    to his record box. It MUST be
    done then, inside OT, because the next time he downloads the inbox from
    the server, that chequeReceipt
    won't be in there anymore! It'll be too late to pass it on to the
    records.
    - Whereas I, being the recipient of his cheque, had it in my **payments
    inbox,** and thus upon receipt
    of a successful server-reply to my deposit transaction, need to move it
    from my payments inbox to my
    record box. (The record box will eventually be a callback so that client
    software can take over that
    functionality, which is outside the scope of OT. The actual CALL to
    store in the record box, however
    should occur inside OT.)
    - For now, I'm using the below API call, so it's available inside the
    scripts. This is "good enough"
    for now, just to get the payments inbox/outbox working for the scripts.
    But in the int64_t term, I'll need
    to add the hooks directly into OT as described just above. (It'll be
    necessary in order to get the record
    box working.)
    - Since I'm only worried about Payments Inbox for now, and since I'll be
    calling the below function
    directly from inside the scripts, how will this work? Incoming cheque or
    invoice will be in the payments
    inbox, and will need to be moved to recordBox (below call) when the
    script receives a success reply to
    the depositing/paying of that cheque/invoice.
    - Whereas outoing cheque/invoice is in the Outpayments box,
    (fundamentally more similar to the outmail
    box than to the payments inbox.) If the cheque/invoice is cashed/paid by
    the endorsee, **I** will receive
    the chequeReceipt, in MY asset account inbox, and when I accept it
    during a processInbox transaction,
    the SUCCESS REPLY from the server for that processInbox is where I
    should actually process that chequeReceipt
    and, if it appears in the outpayments box, move it at that time to the
    record box. The problem is, I can NOT
    do this much inside the script. To do this part, I thus HAVE to go into
    OT itself as I just described.
    - Fuck!
    - Therefore I might as well comment this out, since this simply isn't
    going to work.

    - Updated plan:
    1. DONE: Inside OT, when processing successful server reply to
    processInbox request, if a chequeReceipt
    was processed out successfully, and if that chequeReceipt is found
    inside the outpayments, then
    move it at that time to the record box.
    2. DONE: Inside OT, when processing successful server reply to
    depositCheque request, if that cheque is
    found inside the Payments Inbox, move it to the record box.
    3. As for cash:
    If I SENT cash, it will be in my outpayments box. But that's wrong.
    Because I can
    never see if the guy cashed it or not. Therefore it should go straight
    to the record box, when
    sent. AND it needs to be encrypted to MY key, not his -- so need to
    generate BOTH versions, when
    exporting the purse to him in the FIRST PLACE. Then my version goes
    straight into my record box and
    I can delete it at my leisure. (If he comes running the next day saying
    "I lost it!!" I can still
    recover it. But once he deposits it, then the cash will be no good and I
    might as well archive it
    or destroy it, or whatever I choose to do with my personal records.)
    If I RECEIVED cash, it will be in my payments inbox, and then when I
    deposit it, and when I process
    the SUCCESSFUL server REPLY to my depositCash request, it should be
    moved to my record Box.
    4. How about vouchers? If I deposit a voucher, then the "original
    sender" should get some sort of
    notice. This means attaching his ID to the voucher--which should be
    optional--and then dropping an
    "FYI" notice to him when it gets deposited. It can't be a normal
    chequeReceipt because that's used
    to verify the balance agreement against a balance change, whereas a
    "voucher receipt" wouldn't represent
    a balance change at all, since the balance was already changed when you
    originally bought the voucher.
    Instead it would probably be send to your Nymbox but it COULD NOT BE
    PROVEN that it was, since OT currently
    can't prove NOTICE!!

    All of the above needs to happen inside OT, since there are many plances
    where it's the only appropriate
    place to take the necessary action. (Script cannot.)
    */

    cout << "\n Success paying invoice!\n\n\n";

    return 1;
}

vector<string> OT_Command::tokenize(const string& str, const string& delimiters,
                                    bool trimEmpty)
{
    int32_t lastPos = 0;
    vector<string> tokens;

    while (true) {
        int32_t pos = str.find_first_of(delimiters, lastPos);

        if (-1 == pos) {
            pos = str.size();

            if (pos != lastPos || !trimEmpty) {
                tokens.push_back(str.substr(lastPos, pos - lastPos));
            }
            break;
        }
        else {
            if (pos != lastPos || !trimEmpty) {
                tokens.push_back(str.substr(lastPos, pos - lastPos));
            }
        }

        lastPos = pos + 1;
    }
    return tokens;
}

// Show the active cron item IDs, or the details of one by ID.
//
OT_COMMANDS_OT int32_t OT_Command::mainShowActive()
{
    otOut << "Usage:   showactive --server SERVER_ID --mynym NYM_ID   (To "
             "display a list of IDs.)\nOR:     showactive --server SERVER_ID "
             "--args \"id TRANSACTION_ID\"\n\nLists the transaction numbers of "
             "currently active payment plans\nand smart contracts. Also useful "
             "for displaying contents by ID.\n\n";

    string strError = "\nPlease provide either:  --mynym NYM_ID   (to view a "
                      "list of active transactions)\nor, to view details for a "
                      "specific transaction, use --args \"id TRANSACTION_ID\" "
                      "\n\n";

    if (!VerifyExists("Server")) return -1;

    int64_t lTransNum = 0;
    bool bDetailMode = false;

    if (VerifyExists("Args", false)) {
        string strTransNum = OT_CLI_GetValueByKey(Args, "id");
        if (VerifyStringVal(strTransNum)) {
            lTransNum = std::stoll(strTransNum);

            if (lTransNum > 0) {
                bDetailMode = true;
            }
            else {
                otOut << "Missing argument (1): " << strError;
                return -1;
            }
        }
    }
    if (!bDetailMode && !VerifyExists("MyNym")) {
        otOut << "Missing argument (2): " << strError;
        return -1;
    }
    // By this point, we know for a fact that either MyNym was supplied, and
    // thus we need to display a list
    // of active transactions for the given nym/server, or otherwise,
    // bDetailMode will be true, and lTransNum
    // contains a greater-than-zero value in int64_t form, in which case, we
    // need to display the details
    // of that cron item.

    if (bDetailMode) {
        string strCronItem = OTAPI_Wrap::GetActiveCronItem(Server, lTransNum);

        if (VerifyStringVal(strCronItem)) {
            string strType = OTAPI_Wrap::Instrmnt_GetType(strCronItem);

            if (!VerifyStringVal(strType)) {
                strType = "UNKNOWN";
            }

            otOut << "Found an active transaction!\n";
            otOut << "ID: " << lTransNum << "  Type: " << strType
                  << "\n\nContents:\n\n";

            cout << strCronItem << "\n";

            otOut << "\n";
        }
    }
    else {
        string strIDs = OTAPI_Wrap::GetNym_ActiveCronItemIDs(MyNym, Server);

        if (VerifyStringVal(strIDs) && OTAPI_Wrap::NumList_Count(strIDs) > 0) {
            vector<string> vecIDs = tokenize(strIDs, ",", true);

            for (size_t nIndex = 0; nIndex < vecIDs.size(); ++nIndex) {
                string strTransNum = vecIDs[nIndex];
                if (!VerifyStringVal(strTransNum)) continue;

                lTransNum = std::stoll(strTransNum);
                if (lTransNum <= 0) continue;

                string strCronItem =
                    OTAPI_Wrap::GetActiveCronItem(Server, lTransNum);
                if (!VerifyStringVal(strCronItem)) continue;

                string strType = OTAPI_Wrap::Instrmnt_GetType(strCronItem);
                if (!VerifyStringVal(strType)) {
                    strType = "UNKNOWN";
                }
                if (0 == nIndex) {
                    otOut << "\n Found " << vecIDs.size()
                          << " active transactions:\n\n";
                }
                cout << "ID: " << strTransNum << "  Type: " << strType
                     << "\n\n";
            }
            otOut << "\n";
        }
        else {
            otOut << "\nFound no active transactions. Perhaps try 'opentxs "
                     "refresh' first?\n";
        }
    }
    return 1;
}

// Show an individual payment in detail.
//
OT_COMMANDS_OT int32_t OT_Command::mainShowPayment()
{
    otOut << "Usage:   showpayment --args \"index PAYMENT_INDEX showmemo "
             "true|false\"\n Default index is 0. Default showmemo is false.\n";

    // SHOW a payment from the PAYMENTS INBOX
    //
    // Load an asset account's payments inbox from local storage and display it
    // on the screen.

    if (!VerifyExists("Server") || !VerifyExists("MyNym") ||
        !VerifyExists("Args"))
        return -1;

    string strIndex = OT_CLI_GetValueByKey(Args, "index");
    string strShowLargeMemo = OT_CLI_GetValueByKey(Args, "showmemo");
    int32_t nIndex = 0;
    bool bShowLargeMemo = false;

    // Set the values based on the custom arguments, for those found.
    if (VerifyStringVal(strShowLargeMemo) && ("true" == strShowLargeMemo)) {
        bShowLargeMemo = true;
    }

    if (VerifyStringVal(strIndex)) {
        int32_t nTempIndex = std::stol(strIndex);

        if (nTempIndex >= 0) {
            nIndex = nTempIndex;
        }
    }

    string strInbox = OTAPI_Wrap::LoadPaymentInbox(Server, MyNym);

    if (!VerifyStringVal(strInbox)) {
        otOut << "\n\n OT_API_LoadPaymentInbox: Failed.\n\n";
        return -1;
    }

    otOut << "\n\n";

    int32_t nCount =
        OTAPI_Wrap::Ledger_GetCount(Server, MyNym, MyNym, strInbox);
    if (0 <= nCount) {
        if (nIndex > (nCount - 1)) {
            otOut << "Index out of bounds. (There are " << nCount
                  << " indices, starting at 0.\n";
            return -1;
        }

        otOut << "Ind  Amt   Type       Txn#   Memo\n";
        otOut << "---------------------------------\n";

        string strInstrument = OTAPI_Wrap::Ledger_GetInstrument(
            Server, MyNym, MyNym, strInbox, nIndex);

        if (!VerifyStringVal(strInstrument)) {
            otOut << "Failed trying to get payment instrument from "
                     "payments box.\n";
            return -1;
        }

        string strTrans = OTAPI_Wrap::Ledger_GetTransactionByIndex(
            Server, MyNym, MyNym, strInbox, nIndex);
        int64_t lTransNumber = OTAPI_Wrap::Ledger_GetTransactionIDByIndex(
            Server, MyNym, MyNym, strInbox, nIndex);

        OTAPI_Wrap::Transaction_GetDisplayReferenceToNum(Server, MyNym, MyNym,
                                                         strTrans);

        int64_t lAmount = OTAPI_Wrap::Instrmnt_GetAmount(strInstrument);
        string strType = OTAPI_Wrap::Instrmnt_GetType(strInstrument);
        string strAssetType = OTAPI_Wrap::Instrmnt_GetAssetID(
            strInstrument); // todo: output this.
        string strMemo = OTAPI_Wrap::Instrmnt_GetMemo(strInstrument);
        string strSenderUserID =
            OTAPI_Wrap::Instrmnt_GetSenderUserID(strInstrument);
        string strSenderAcctID =
            OTAPI_Wrap::Instrmnt_GetSenderAcctID(strInstrument);
        string strRemitterUserID =
            OTAPI_Wrap::Instrmnt_GetRemitterUserID(strInstrument);
        string strRemitterAcctID =
            OTAPI_Wrap::Instrmnt_GetRemitterAcctID(strInstrument);
        string strRecipientUserID =
            OTAPI_Wrap::Instrmnt_GetRecipientUserID(strInstrument);
        string strRecipientAcctID =
            OTAPI_Wrap::Instrmnt_GetRecipientAcctID(strInstrument);

        string strUserID = VerifyStringVal(strRemitterUserID)
                               ? strRemitterUserID
                               : strSenderUserID;
        string strAcctID = VerifyStringVal(strRemitterAcctID)
                               ? strRemitterAcctID
                               : strSenderAcctID;

        bool bUserIDExists = VerifyStringVal(strUserID);
        bool bAcctIDExists = VerifyStringVal(strAcctID);
        bool bAssetIDExists = VerifyStringVal(strAssetType);
        bool bMemoExists = VerifyStringVal(strMemo);

        bool bHasAmount = OT_ERROR_AMOUNT != lAmount;

        if (bMemoExists && (strMemo.find("\n") != string::npos) &&
            !bShowLargeMemo) {
            strMemo = "<too large to display here>";
        }
        else if (bMemoExists) {
            string strTempMemo = "\"" + strMemo + "\"";
            strMemo = strTempMemo;
        }

        string strNewlineSeparator = "";

        if (bUserIDExists || bAcctIDExists) {
            strNewlineSeparator = "\n";
        }

        string strSeparator =
            (!bUserIDExists && !bAcctIDExists && !bAssetIDExists)
                ? ""
                : strNewlineSeparator;

        // Set up some symbols to appear or not,
        // based on whether there is something to show.
        string strUserDenoter = (bUserIDExists ? "U:" : "");
        string strUserDenoter2 = (bUserIDExists ? " - " : "");
        string strAcctDenoter = (bAcctIDExists ? "A:" : "");
        string strAcctDenoter2 = (bAcctIDExists ? " - " : "");
        string strAssetDenoter = (bAssetIDExists ? "T:" : "");
        string strAssetDenoter2 = (bAssetIDExists ? " - " : "");

        // If we have the user/acct/asset ID, then get the associated
        // name (if that exists.)
        string strUserName =
            (bUserIDExists ? ("\"" + OTAPI_Wrap::GetNym_Name(strUserID) + "\"")
                           : "");
        string strAcctName =
            (bAcctIDExists
                 ? ("\"" + OTAPI_Wrap::GetAccountWallet_Name(strAcctID) + "\"")
                 : "");
        string strAssetName =
            (bAssetIDExists
                 ? ("\"" + OTAPI_Wrap::GetAssetType_Name(strAssetType) + "\"")
                 : "");

        // Just making sure here that if the string wasn't there, that
        // it's set to
        // a proper empty string, instead of a null or a "not a value"
        // value.
        if (!VerifyStringVal(strUserName)) {
            strUserName = "";
            strUserDenoter2 = "";
        }
        if (!VerifyStringVal(strAcctName)) {
            strAcctName = "";
            strAcctDenoter2 = "";
        }
        if (!VerifyStringVal(strAssetName)) {
            strAssetName = "";
            strAssetDenoter2 = "";
        }

        if ("\"\"" == strUserName) {
            strUserName = "";
            strUserDenoter2 = "";
        }
        if ("\"\"" == strAcctName) {
            strAcctName = "";
            strAcctDenoter2 = "";
        }
        if ("\"\"" == strAssetName) {
            strAssetName = "";
            strAssetDenoter2 = "";
        }

        cout << nIndex << "    ";

        if (bAssetIDExists && bHasAmount) {
            string strAmount = OTAPI_Wrap::FormatAmount(strAssetType, lAmount);
            cout << strAmount << (strAmount.size() < 3 ? "    " : "   ");
        }
        else {
            cout << "      ";
        }

        cout << strType << (strType.size() > 10 ? " " : "    ");
        cout << lTransNumber
             << (std::to_string(lTransNumber).size() < 2 ? "    " : "   ");
        cout << (bMemoExists ? strMemo : "") << "\n";
        cout << strUserDenoter << strUserID << strUserDenoter2 << strUserName
             << strSeparator;
        cout << strAcctDenoter << strAcctID << strAcctDenoter2 << strAcctName
             << strSeparator;
        cout << strAssetDenoter << strAssetType << strAssetDenoter2
             << strAssetName << "\n";
        cout << "Instrument:\n" << strInstrument << "\n";
    }
    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainShowIncoming()
{
    int32_t nShowPayments = -1;
    int32_t nShowInbox = -1;

    if (VerifyExists("MyAcct", false)) {
        nShowInbox = OT_Command::mainInbox();
    }
    else {
        otOut << "Try adding --myacct ASSET_ACCT_ID   (to see the asset "
                 "account's inbox.)\n";
    }

    if (VerifyExists("Server", false) && VerifyExists("MyNym", false)) {
        nShowPayments = OT_Command::mainInpayments();
    }
    else {
        otOut << "Try adding --mynym NYM_ID  and  --server SERVER_ID\n(to see "
                 "the payments inbox for that Server and Nym.)\n";
    }

    if ((nShowInbox > -1) || (nShowPayments > -1)) {
        return 1;
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainShowOutgoing()
{
    int32_t nShowPayments = -1;
    int32_t nShowOutbox = -1;

    if (VerifyExists("MyAcct", false)) {
        nShowOutbox = OT_Command::mainOutbox();
    }
    else {
        otOut << "Try adding --myacct ASSET_ACCT_ID   (to see the asset "
                 "account's outbox.)\n";
    }

    if (VerifyExists("MyNym", false)) {
        nShowPayments = OT_Command::mainOutpayment();
    }
    else {
        otOut << "Try adding --mynym NYM_ID (to see the outpayment box for "
                 "that Nym.)\n";
    }

    if ((nShowOutbox > -1) || (nShowPayments > -1)) {
        return 1;
    }

    return -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainInpayments()
{
    // SHOW PAYMENTS INBOX
    //
    // Load an asset account's payments inbox from local storage and display it
    // on the screen.

    if (!VerifyExists("Server") || !VerifyExists("MyNym")) return -1;

    string strInbox = OTAPI_Wrap::LoadPaymentInbox(Server, MyNym);

    if (!VerifyStringVal(strInbox)) {
        otOut << "\n Unable to load the payments inbox (probably doesn't "
                 "exist yet.)\n(Nym/Server: " << MyNym << " / " << Server
              << " )\n\n";
        return -1;
    }

    otOut << "\n";

    int32_t nCount =
        OTAPI_Wrap::Ledger_GetCount(Server, MyNym, MyNym, strInbox);
    if (0 > nCount) {
        return -1;
    }

    if (nCount > 0) {
        otOut << "Show payments inbox (Nym/Server)\n( " << MyNym << " / "
              << Server << " )\n";
        otOut << "Idx  Amt   Type      Txn#  Asset_Type\n";
        otOut << "---------------------------------------\n";

        for (int32_t nIndex = 0; nIndex < nCount; ++nIndex) {
            string strInstrument = OTAPI_Wrap::Ledger_GetInstrument(
                Server, MyNym, MyNym, strInbox, nIndex);

            if (!VerifyStringVal(strInstrument)) {
                otOut << "Failed trying to get payment instrument from "
                         "payments box.\n";
                return -1;
            }

            string strTrans = OTAPI_Wrap::Ledger_GetTransactionByIndex(
                Server, MyNym, MyNym, strInbox, nIndex);
            int64_t lTransNumber = OTAPI_Wrap::Ledger_GetTransactionIDByIndex(
                Server, MyNym, MyNym, strInbox, nIndex);

            string strTransID = 0 <= lTransNumber ? std::to_string(lTransNumber)
                                                  : "UNKNOWN_TRANS_NUM";

            OTAPI_Wrap::Transaction_GetDisplayReferenceToNum(Server, MyNym,
                                                             MyNym, strTrans);
            int64_t lAmount = OTAPI_Wrap::Instrmnt_GetAmount(strInstrument);
            string strType = OTAPI_Wrap::Instrmnt_GetType(strInstrument);
            string strAssetType = OTAPI_Wrap::Instrmnt_GetAssetID(
                strInstrument); // todo: output this.
            string strSenderUserID =
                OTAPI_Wrap::Instrmnt_GetSenderUserID(strInstrument);
            string strSenderAcctID =
                OTAPI_Wrap::Instrmnt_GetSenderAcctID(strInstrument);
            string strRecipientUserID =
                OTAPI_Wrap::Instrmnt_GetRecipientUserID(strInstrument);
            string strRecipientAcctID =
                OTAPI_Wrap::Instrmnt_GetRecipientAcctID(strInstrument);

            string strUserID = VerifyStringVal(strSenderUserID)
                                   ? strSenderUserID
                                   : strRecipientUserID;
            string strAcctID = VerifyStringVal(strSenderAcctID)
                                   ? strSenderAcctID
                                   : strRecipientAcctID;

            bool bHasAmount = OT_ERROR_AMOUNT != lAmount;
            bool bHasAsset = VerifyStringVal(strAssetType);

            string strAmount =
                (bHasAmount && bHasAsset)
                    ? OTAPI_Wrap::FormatAmount(strAssetType, lAmount)
                    : "UNKNOWN_AMOUNT";

            VerifyStringVal(strUserID);
            VerifyStringVal(strAcctID);
            bool bAssetIDExists = VerifyStringVal(strAssetType);

            string strAssetDenoter = (bAssetIDExists ? " - " : "");

            string strAssetName =
                (bAssetIDExists
                     ? ("\"" + OTAPI_Wrap::GetAssetType_Name(strAssetType) +
                        "\"")
                     : "");
            if (!VerifyStringVal(strAssetName)) {
                strAssetName = "";
                strAssetDenoter = "";
            }

            cout << nIndex << "    ";
            cout << strAmount << (strAmount.size() < 3 ? "    " : "   ");
            cout << strType << (strType.size() > 10 ? " " : "    ");
            cout << strTransID << (strTransID.size() < 2 ? "    " : "   ");
            cout << strAssetType << strAssetDenoter << strAssetName << "\n";
        }

        otOut << "\n For the above, try: acceptpayments, "
                 "acceptinvoices, acceptmoney, or acceptall.\n EXCEPT "
                 "for smart contracts and payment plans -- for those, "
                 "use: opentxs confirm\n\n";
    }
    else {
        otOut << "\n Payments inbox is empty (Nym/Server: " << MyNym << " / "
              << Server << " )\n";
    }

    return 1;
}

OT_COMMANDS_OT int32_t
    OT_Command::details_show_record(const string& strServerID,
                                    const string& strMyNymID,
                                    const string& strMyAcctID, int32_t nIndex,
                                    const string& strRecordBox)
{
    if (!VerifyStringVal(strRecordBox)) {
        otOut << "\n\n details_show_record: strRecordBox is empty.\n(Need to "
                 "load it first before calling this function.)\n\n";
        return false;
    }

    string strTrans = OTAPI_Wrap::Ledger_GetTransactionByIndex(
        strServerID, strMyNymID, strMyAcctID, strRecordBox, nIndex);
    int64_t lTransID = OTAPI_Wrap::Ledger_GetTransactionIDByIndex(
        strServerID, strMyNymID, strMyAcctID, strRecordBox, nIndex);
    int64_t lRefNum = OTAPI_Wrap::Transaction_GetDisplayReferenceToNum(
        strServerID, strMyNymID, strMyAcctID, strTrans);
    int64_t lAmount = OTAPI_Wrap::Transaction_GetAmount(strServerID, strMyNymID,
                                                        strMyAcctID, strTrans);
    string strType = OTAPI_Wrap::Transaction_GetType(strServerID, strMyNymID,
                                                     strMyAcctID, strTrans);
    string strSenderUserID = OTAPI_Wrap::Transaction_GetSenderUserID(
        strServerID, strMyNymID, strMyAcctID, strTrans);
    string strSenderAcctID = OTAPI_Wrap::Transaction_GetSenderAcctID(
        strServerID, strMyNymID, strMyAcctID, strTrans);
    string strRecipientUserID = OTAPI_Wrap::Transaction_GetRecipientUserID(
        strServerID, strMyNymID, strMyAcctID, strTrans);
    string strRecipientAcctID = OTAPI_Wrap::Transaction_GetRecipientAcctID(
        strServerID, strMyNymID, strMyAcctID, strTrans);

    string strUserID =
        VerifyStringVal(strSenderUserID) ? strSenderUserID : strRecipientUserID;
    string strAcctID =
        VerifyStringVal(strSenderAcctID) ? strSenderAcctID : strRecipientAcctID;

    bool bUserIDExists = VerifyStringVal(strUserID);
    bool bAcctIDExists = VerifyStringVal(strAcctID);

    string strNewlineSeparator = "";

    if (bUserIDExists || bAcctIDExists) {
        strNewlineSeparator = "\n                                 |";
    }

    string strSeparator =
        (!bUserIDExists && !bAcctIDExists) ? "" : strNewlineSeparator;

    string strUserDenoter = (bUserIDExists ? "U:" : "");
    string strAcctDenoter = (bAcctIDExists ? "A:" : "");

    string strAssetTypeID =
        bAcctIDExists ? OTAPI_Wrap::GetAccountWallet_AssetTypeID(strAcctID)
                      : "";

    bool bHasAmount = OT_ERROR_AMOUNT != lAmount;
    bool bHasAsset = VerifyStringVal(strAssetTypeID);

    string strAmount =
        bHasAmount
            ? (bHasAsset ? OTAPI_Wrap::FormatAmount(strAssetTypeID, lAmount)
                         : std::to_string(lAmount))
            : "UNKNOWN_AMOUNT";

    otOut << nIndex << "    ";
    otOut << strAmount << (strAmount.size() < 3 ? "    " : "   ");
    otOut << strType << (strType.size() > 10 ? " " : "    ");
    otOut << lTransID << (std::to_string(lTransID).size() < 2 ? "    " : "   ");
    otOut << lRefNum << (std::to_string(lRefNum).size() > 2 ? "  " : " ")
          << "|";
    otOut << strUserDenoter << strUserID << strSeparator << strAcctDenoter
          << strAcctID << "\n";

    return true;
}

OT_COMMANDS_OT int32_t
    OT_Command::details_show_records(const string& strServerID,
                                     const string& strMyNymID,
                                     const string& strMyAcctID)
{
    string strRecordBox =
        OTAPI_Wrap::LoadRecordBox(strServerID, strMyNymID, strMyAcctID);

    if (!VerifyStringVal(strRecordBox)) {
        otOut << "\n\n details_show_records: OT_API_LoadRecordBox: Failed.\n\n";
        return -1;
    }

    otOut << "\n\n";

    int32_t nCount = OTAPI_Wrap::Ledger_GetCount(strServerID, strMyNymID,
                                                 strMyAcctID, strRecordBox);
    if (0 > nCount) {
        return -1;
    }

    otOut << "SHOW RECORDS: \n\n";
    otOut << "Idx  Amt  Type        Txn# InRef#|User / Acct\n";
    otOut << "---------------------------------|(from or to)\n";

    for (int32_t nIndex = 0; nIndex < nCount; ++nIndex) {
        if (!details_show_record(strServerID, strMyNymID, strMyAcctID, nIndex,
                                 strRecordBox)) {
            otOut
                << "details_show_records: Error calling details_show_record.\n";
            return -1;
        }
    }

    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainShowRecords()
{
    string strLocation = "mainShowRecords";

    // SHOW RECORD BOX
    //
    // Load an asset account's record box from local storage and display it on
    // the screen.
    bool bNymExists = VerifyExists("MyNym", false);
    bool bAcctExists = VerifyExists("MyAcct", false);
    bool bBothExist = (bNymExists && bAcctExists);
    bool bShowNymOnly = (bNymExists && !bAcctExists);

    string strMyNymID = "";

    if (bShowNymOnly) {
        strMyNymID = MyNym;
    }
    else if (bAcctExists) {
        strMyNymID = OTAPI_Wrap::GetAccountWallet_NymID(MyAcct);
    }
    else {
        otOut << strLocation << ": This should never happen. (1)\n";
        return -1;
    }

    if (!VerifyStringVal(strMyNymID)) {
        // if bShowNymOnly is true, then we KNOW MyNym is the Nym, and we
        // ALREADY know he's
        // good, since we called VerifyExists at the top of this function.
        // (MyNym, unlike HisNym,
        // is verified against the wallet, so we KNOW it's there.)
        // Therefore, if strMyNymID doesn't contain a proper string, then we
        // KNOW bShowNymOnly
        // must have been false, and that
        // OTAPI_Wrap::GetAccountWallet_NymID(MyAcct) was the call that
        // failed. Thus, the error message is appropriate to the latter case and
        // not the former.
        otOut << strLocation << ": Unable to find NymID based on myacct. Try a "
                                "different account or nym, using --myacct "
                                "ACCT_ID or --mynym NYM_ID\n";
        return -1;
    }
    if (bBothExist && !(strMyNymID == MyNym)) {
        otOut << strLocation << ": MyNym (" << MyNym
              << ") is not the same nym who owns MyAcct.\nTo override using "
                 "MyAcct's owner nym, add:  --mynym (" << strMyNymID << ")\n";
        return -1;
    }

    string strServerID = "";
    if (bAcctExists) {
        strServerID = OTAPI_Wrap::GetAccountWallet_ServerID(MyAcct);
    }
    else if (VerifyExists("Server", false)) {
        strServerID = Server;
    }
    else {
        otOut << strLocation << ": Server was not specified, and either MyAcct "
                                "wasn't specified either, or if it was, I was "
                                "unable to find any ServerID associated with "
                                "MyAcct.\nTry a different account or different "
                                "server, using --myacct ACCT_ID or --server "
                                "SERVER_ID \n\n";
        return -1;
    }

    if (!VerifyStringVal(strServerID)) {
        otOut << strLocation << ": Unable to find Server ID.\nTry: --server "
                                "SERVER_ID\nYou might also try: --myacct "
                                "ACCT_ID  (It will find the server ID based on "
                                "the account.)\n";
        return -1;
    }
    //
    // REMEMBER, recordbox for MyAcct contains the old inbox receipts.
    // Whereas recordbox for MyNym contains the old payments (in and out.)
    // In the case of the latter, the NYM must be passed as the ACCT...
    //
    // Meaning: there are TWO record boxes. So must there thus be two commands
    // for viewing them? Or do we combine them somehow?
    ///
    // ===> I say combine them, since they are for viewing only (nothing is
    // actually done
    // with the records -- they're just there for the actual client to take and
    // store
    // however it wishes.)
    otOut << "\n\nNote: OT is completely done with these records. A proper GUI "
             "will sweep them out\nperiodically and archive them somewhere, or "
             "just erase them. All you can do at the\ncommand line (using this "
             "tool) is view them, or erase them using: opentxs clearrecords "
             "\n\n";
    otOut << "\nArchived Nym-related records (" << strMyNymID << "): \n";
    int32_t nNymRecords =
        details_show_records(strServerID, strMyNymID, strMyNymID);
    otOut << "--------------------------------------------------------------\n";

    int32_t nAcctRecords = 1;

    if (bAcctExists) {
        otOut << "\nArchived Account-related records (" << MyAcct << "): \n";
        nAcctRecords = details_show_records(strServerID, strMyNymID, MyAcct);
    }

    if (2 == (nNymRecords + nAcctRecords)) {
        return 1;
    }

    return -1;
}

OT_COMMANDS_OT int32_t
    OT_Command::details_clear_records(const string& strServerID,
                                      const string& strMyNymID,
                                      const string& strMyAcctID)
{
    bool bTrue = true;
    bool bCleared =
        OTAPI_Wrap::ClearRecord(strServerID, strMyNymID, strMyAcctID, 0, bTrue);

    return bCleared ? 1 : -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainClearRecords()
{
    string strLocation = "mainClearRecords";

    otOut << " Usage:   clearrecords\n\n";

    // CLEAR RECORD BOX
    //
    // Load an asset account's record box from local storage and display it on
    // the screen.
    bool bNymExists = VerifyExists("MyNym", false);
    bool bAcctExists = VerifyExists("MyAcct", false);
    bool bBothExist = (bNymExists && bAcctExists);
    bool bShowNymOnly = (bNymExists && !bAcctExists);

    string strMyNymID = "";

    if (bShowNymOnly) {
        strMyNymID = MyNym;
    }
    else if (bAcctExists) {
        strMyNymID = OTAPI_Wrap::GetAccountWallet_NymID(MyAcct);
    }
    else {
        otOut << strLocation << ": This should never happen. (1)\n";
        return -1;
    }

    if (!VerifyStringVal(strMyNymID)) {
        // if bShowNymOnly is true, then we KNOW MyNym is the Nym, and we
        // ALREADY know he's
        // good, since we called VerifyExists at the top of this function.
        // (MyNym, unlike HisNym,
        // is verified against the wallet, so we KNOW it's there.)
        // Therefore, if strMyNymID doesn't contain a proper string, then we
        // KNOW bShowNymOnly
        // must have been false, and that
        // OTAPI_Wrap::GetAccountWallet_NymID(MyAcct) was the call that
        // failed. Thus, the error message is appropriate to the latter case and
        // not the former.
        otOut << strLocation << ": Unable to find NymID based on myacct. Try a "
                                "different account or nym, using --myacct "
                                "ACCT_ID or --mynym NYM_ID\n";
        return -1;
    }
    if (bBothExist && !(strMyNymID == MyNym)) {
        otOut << strLocation << ": MyNym (" << MyNym
              << ") is not the same nym who owns MyAcct.\nTo override using "
                 "MyAcct's owner nym, add:  --mynym (" << strMyNymID << ")\n";
        return -1;
    }

    string strServerID = "";
    if (bAcctExists) {
        strServerID = OTAPI_Wrap::GetAccountWallet_ServerID(MyAcct);
    }
    else if (VerifyExists("Server", false)) {
        strServerID = Server;
    }
    else {
        otOut << strLocation << ": Server was not specified, and either MyAcct "
                                "wasn't specified either, or if it was, I was "
                                "unable to find any ServerID associated with "
                                "MyAcct.\nTry a different account or different "
                                "server, using --myacct ACCT_ID or --server "
                                "SERVER_ID \n\n";
        return -1;
    }

    if (!VerifyStringVal(strServerID)) {
        otOut << strLocation << ": Unable to find Server ID.\nTry: --server "
                                "SERVER_ID\nYou might also try: --myacct "
                                "ACCT_ID  (It will find the server ID based on "
                                "the account.)\n";
        return -1;
    }

    // REMEMBER, recordbox for MyAcct contains the old inbox receipts.
    // Whereas recordbox for MyNym contains the old payments (in and out.)
    // In the case of the latter, the NYM must be passed as the ACCT...
    //
    // Meaning: there are TWO record boxes.
    otOut << "\n Clearing archived Nym-related records (" << strMyNymID
          << ")... \n";
    int32_t nNymRecords =
        details_clear_records(strServerID, strMyNymID, strMyNymID);
    otOut << "--------------------------------------------------------------\n";
    otOut << "\nClearing archived Account-related records (" << MyAcct
          << ")... \n";
    int32_t nAcctRecords =
        details_clear_records(strServerID, strMyNymID, MyAcct);

    if (2 == (nNymRecords + nAcctRecords)) {
        return 1;
    }

    return -1;
}

OT_COMMANDS_OT int32_t
    OT_Command::details_clear_expired(const string& strServerID,
                                      const string& strMyNymID)
{
    bool bTrue = true;
    bool bCleared = OTAPI_Wrap::ClearExpired(strServerID, strMyNymID, 0, bTrue);

    return bCleared ? 1 : -1;
}

OT_COMMANDS_OT int32_t OT_Command::mainClearExpired()
{
    otOut << " Usage:   clearexpired\n\n";

    // CLEAR EXPIRED BOX
    if (!VerifyExists("MyNym") || !VerifyExists("Server")) return -1;

    string strMyNymID = MyNym;
    string strServerID = Server;

    // expiredBox for MyNym contains the old payments (in and out.)
    otOut << "\n Clearing archived Nym-related expired records (" << strMyNymID
          << ")... \n";

    return details_clear_expired(strServerID, strMyNymID);
}

// TODO: This is based on details_show_record which assumes it can contain asset
// inbox records as well
// as payment inbox records. But in fact, the expired box can ONLY contain
// payment records, not asset
// account records. So at some point, we will want to update this function to
// use the Instrument calls
// instead of the typical transaction calls, since that is more appropriate in
// this case. In fact we
// will also want to update details_show_record to do the same thing, in cases
// where the nymID and
// the account ID are the same.
//
OT_COMMANDS_OT int32_t
    OT_Command::details_show_expired(const string& strServerID,
                                     const string& strMyNymID, int32_t nIndex,
                                     const string& strExpiredBox)
{
    if (!VerifyStringVal(strExpiredBox)) {
        otOut << "\n\n details_show_expired: strExpiredBox is empty.\n(Need to "
                 "load it first before calling this function.)\n\n";
        return false;
    }

    string strTrans = OTAPI_Wrap::Ledger_GetTransactionByIndex(
        strServerID, strMyNymID, strMyNymID, strExpiredBox, nIndex);
    int64_t lTransID = OTAPI_Wrap::Ledger_GetTransactionIDByIndex(
        strServerID, strMyNymID, strMyNymID, strExpiredBox, nIndex);
    int64_t lRefNum = OTAPI_Wrap::Transaction_GetDisplayReferenceToNum(
        strServerID, strMyNymID, strMyNymID, strTrans);
    int64_t lAmount = OTAPI_Wrap::Transaction_GetAmount(strServerID, strMyNymID,
                                                        strMyNymID, strTrans);
    string strType = OTAPI_Wrap::Transaction_GetType(strServerID, strMyNymID,
                                                     strMyNymID, strTrans);
    string strSenderUserID = OTAPI_Wrap::Transaction_GetSenderUserID(
        strServerID, strMyNymID, strMyNymID, strTrans);
    string strSenderAcctID = OTAPI_Wrap::Transaction_GetSenderAcctID(
        strServerID, strMyNymID, strMyNymID, strTrans);
    string strRecipientUserID = OTAPI_Wrap::Transaction_GetRecipientUserID(
        strServerID, strMyNymID, strMyNymID, strTrans);
    string strRecipientAcctID = OTAPI_Wrap::Transaction_GetRecipientAcctID(
        strServerID, strMyNymID, strMyNymID, strTrans);

    string strUserID =
        VerifyStringVal(strSenderUserID) ? strSenderUserID : strRecipientUserID;
    string strAcctID =
        VerifyStringVal(strSenderAcctID) ? strSenderAcctID : strRecipientAcctID;

    bool bUserIDExists = VerifyStringVal(strUserID);
    bool bAcctIDExists = VerifyStringVal(strAcctID);

    string strNewlineSeparator = "";

    if (bUserIDExists || bAcctIDExists) {
        strNewlineSeparator = "\n                                 |";
    }

    string strSeparator =
        (!bUserIDExists && !bAcctIDExists) ? "" : strNewlineSeparator;

    string strUserDenoter = (bUserIDExists ? "U:" : "");
    string strAcctDenoter = (bAcctIDExists ? "A:" : "");

    string strAssetTypeID =
        bAcctIDExists ? OTAPI_Wrap::GetAccountWallet_AssetTypeID(strAcctID)
                      : "";

    string strAmount =
        OT_ERROR_AMOUNT != lAmount
            ? (VerifyStringVal(strAssetTypeID)
                   ? OTAPI_Wrap::FormatAmount(strAssetTypeID, lAmount)
                   : std::to_string(lAmount))
            : "UNKNOWN_AMOUNT";

    otOut << nIndex << "    ";
    otOut << strAmount << (strAmount.size() < 3 ? "    " : "   ");
    otOut << strType << (strType.size() > 10 ? " " : "    ");
    otOut << lTransID << (std::to_string(lTransID).size() < 2 ? "    " : "   ");
    otOut << lRefNum << (std::to_string(lRefNum).size() > 2 ? "  " : " ")
          << "|";
    otOut << strUserDenoter << strUserID << strSeparator << strAcctDenoter
          << strAcctID << "\n";

    return true;
}

OT_COMMANDS_OT int32_t
    OT_Command::details_show_expired_records(const string& strServerID,
                                             const string& strMyNymID)
{
    string strExpiredBox = OTAPI_Wrap::LoadExpiredBox(strServerID, strMyNymID);

    if (!VerifyStringVal(strExpiredBox)) {
        otOut << "\n\n details_show_expired_records: OT_API_LoadExpiredBox: "
                 "Failed.\n\n";
        return -1;
    }

    otOut << "\n\n";

    int32_t nCount = OTAPI_Wrap::Ledger_GetCount(strServerID, strMyNymID,
                                                 strMyNymID, strExpiredBox);
    if (0 > nCount) {
        return -1;
    }

    otOut << "SHOW EXPIRED RECORDS: \n\n";
    otOut << "Idx  Amt  Type        Txn# InRef#|User / Acct\n";
    otOut << "---------------------------------|(from or to)\n";

    for (int32_t nIndex = 0; nIndex < nCount; ++nIndex) {
        if (!details_show_expired(strServerID, strMyNymID, nIndex,
                                  strExpiredBox)) {
            otOut << "details_show_expired_records: Error calling "
                     "details_show_expired.\n";
            return -1;
        }
    }

    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainShowExpired()
{
    string strLocation = "mainShowExpired";

    // SHOW EXPIRED RECORD BOX
    if (!VerifyExists("MyNym") || !VerifyExists("Server")) return -1;

    string strMyNymID = MyNym;
    string strServerID = Server;

    // expiredbox for MyNym contains the old payments (in and out.)

    otOut << "\n\nNote: OT is completely done with these expired records. "
             "A proper GUI will sweep them out\nperiodically and archive "
             "them somewhere, or just erase them. All you can do at "
             "the\ncommand line (using this tool) is view them, or erase "
             "them using: opentxs clearexpired \n\n";
    otOut << "\nArchived Nym-related expired records (" << strMyNymID
          << "): \n";

    return details_show_expired_records(strServerID, strMyNymID);
}

OT_COMMANDS_OT int32_t OT_Command::mainInbox()
{

    // SHOW INBOX
    //
    // Load an asset account's inbox from local storage and display it on the
    // screen.

    if (!VerifyExists("MyAcct")) return -1;

    string strMyNymID = OTAPI_Wrap::GetAccountWallet_NymID(MyAcct);
    if (!VerifyStringVal(strMyNymID)) {
        otOut << "Failure: Unable to find NymID based on myacct. Use: "
                 "--myacct ACCT_ID\n";
        otOut << "The designated asset account must be yours. OT will find "
                 "the Nym based on the account.\n\n";
        return -1;
    }

    string strServerID = OTAPI_Wrap::GetAccountWallet_ServerID(MyAcct);
    if (!VerifyStringVal(strServerID)) {
        otOut << "Failure: Unable to find Server ID based on myacct. Use: "
                 "--myacct ACCT_ID\n";
        otOut << "The designated asset account must be yours. OT will find "
                 "the Server based on the account.\n\n";
        return -1;
    }

    string strInbox = OTAPI_Wrap::LoadInbox(strServerID, strMyNymID, MyAcct);
    if (!VerifyStringVal(strInbox)) {
        otOut << "\n Unable to load asset account inbox. ( " << MyAcct
              << " )\n Perhaps it doesn't exist yet?\n\n";
        return -1;
    }

    otOut << "\n";

    int32_t nCount =
        OTAPI_Wrap::Ledger_GetCount(strServerID, strMyNymID, MyAcct, strInbox);
    if (0 > nCount) {
        return -1;
    }

    if (nCount > 0) {
        otOut << "Show inbox for an asset account  (" << MyAcct << "): \n";
        otOut << "Idx  Amt  Type        Txn# InRef#|User / Acct\n";
        otOut << "---------------------------------|(from or to)\n";

        for (int32_t nIndex = 0; nIndex < nCount; ++nIndex) {
            string strTrans = OTAPI_Wrap::Ledger_GetTransactionByIndex(
                strServerID, strMyNymID, MyAcct, strInbox, nIndex);
            int64_t lTransID = OTAPI_Wrap::Ledger_GetTransactionIDByIndex(
                strServerID, strMyNymID, MyAcct, strInbox, nIndex);
            int64_t lRefNum = OTAPI_Wrap::Transaction_GetDisplayReferenceToNum(
                strServerID, strMyNymID, MyAcct, strTrans);
            int64_t lAmount = OTAPI_Wrap::Transaction_GetAmount(
                strServerID, strMyNymID, MyAcct, strTrans);
            string strType = OTAPI_Wrap::Transaction_GetType(
                strServerID, strMyNymID, MyAcct, strTrans);
            string strSenderUserID = OTAPI_Wrap::Transaction_GetSenderUserID(
                strServerID, strMyNymID, MyAcct, strTrans);
            string strSenderAcctID = OTAPI_Wrap::Transaction_GetSenderAcctID(
                strServerID, strMyNymID, MyAcct, strTrans);
            string strRecipientUserID =
                OTAPI_Wrap::Transaction_GetRecipientUserID(
                    strServerID, strMyNymID, MyAcct, strTrans);
            string strRecipientAcctID =
                OTAPI_Wrap::Transaction_GetRecipientAcctID(
                    strServerID, strMyNymID, MyAcct, strTrans);

            string strUserID = VerifyStringVal(strSenderUserID)
                                   ? strSenderUserID
                                   : strRecipientUserID;
            string strAcctID = VerifyStringVal(strSenderAcctID)
                                   ? strSenderAcctID
                                   : strRecipientAcctID;

            bool bUserIDExists = VerifyStringVal(strUserID);
            bool bAcctIDExists = VerifyStringVal(strAcctID);

            string strNewlineSeparator = "";

            if (bUserIDExists || bAcctIDExists) {
                strNewlineSeparator = "\n                                 |";
            }

            string strSeparator =
                (!bUserIDExists && !bAcctIDExists) ? "" : strNewlineSeparator;

            string strUserDenoter = (bUserIDExists ? "U:" : "");
            string strAcctDenoter = (bAcctIDExists ? "A:" : "");

            string strAssetTypeID =
                bAcctIDExists
                    ? OTAPI_Wrap::GetAccountWallet_AssetTypeID(strAcctID)
                    : "";

            string strAmount =
                OT_ERROR_AMOUNT != lAmount
                    ? (VerifyStringVal(strAssetTypeID)
                           ? OTAPI_Wrap::FormatAmount(strAssetTypeID, lAmount)
                           : std::to_string(lAmount))
                    : "UNKNOWN_AMOUNT";

            otOut << nIndex << "    ";
            otOut << strAmount << (strAmount.size() < 3 ? "    " : "   ");
            otOut << strType;
            otOut << (strType.size() > 10 ? " " : "    ");
            otOut << lTransID
                  << (std::to_string(lTransID).size() < 2 ? "    " : "   ");
            otOut << lRefNum
                  << (std::to_string(lRefNum).size() > 2 ? "  " : " ") << "|";
            otOut << strUserDenoter << strUserID << strSeparator
                  << strAcctDenoter << strAcctID << "\n";
        }

        otOut << "\n For the above, try: accepttransfers, "
                 "acceptreceipts, acceptinbox, acceptmoney, or "
                 "acceptall.\n";
    }
    else {
        otOut << "\n Asset account inbox ( " << MyAcct << " ) is empty.\n";
    }

    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainOutbox()
{

    // SHOW OUTPUT
    //
    // Load an asset account's outbox from local storage and display it on the
    // screen.
    otOut << "-----------------------------------------------\nUSAGE:   outbox "
             "--myacct MY_ACCT_ID\n";

    if (!VerifyExists("MyAcct")) return -1;

    string strMyNymID = OTAPI_Wrap::GetAccountWallet_NymID(MyAcct);
    if (!VerifyStringVal(strMyNymID)) {
        otOut << "Failure: Unable to find NymID based on myacct. Use: "
                 "--myacct ACCT_ID\n";
        otOut << "The designated asset account must be yours. OT will find "
                 "the Nym based on the account.\n\n";
        return -1;
    }

    string strServerID = OTAPI_Wrap::GetAccountWallet_ServerID(MyAcct);
    if (!VerifyStringVal(strServerID)) {
        otOut << "Failure: Unable to find Server ID based on myacct. Use: "
                 "--myacct ACCT_ID\n";
        otOut << "The designated asset account must be yours. OT will find "
                 "the Server based on the account.\n\n";
        return -1;
    }

    string strOutbox = OTAPI_Wrap::LoadOutbox(strServerID, strMyNymID, MyAcct);
    if (!VerifyStringVal(strOutbox)) {
        otOut << "\n\n OT_API_LoadOutbox: Failed.\n\n";
        return -1;
    }

    otOut << "\n";

    int32_t nCount =
        OTAPI_Wrap::Ledger_GetCount(strServerID, strMyNymID, MyAcct, strOutbox);
    if (0 > nCount) {
        return -1;
    }

    otOut << "===> SHOW OUTBOX: \n";

    if (nCount <= 0) {
        otOut << "\nAsset account outbox is empty for account: " << MyAcct
              << "\n\n--------------------------------------------\n";
    }
    else {
        otOut << "\nIdx  Amt  Type        Txn# InRef#|User / Acct\n";
        otOut << "---------------------------------|(from or to)\n";

        for (int32_t nIndex = 0; nIndex < nCount; ++nIndex) {
            string strTrans = OTAPI_Wrap::Ledger_GetTransactionByIndex(
                strServerID, strMyNymID, MyAcct, strOutbox, nIndex);
            int64_t lTransID = OTAPI_Wrap::Ledger_GetTransactionIDByIndex(
                strServerID, strMyNymID, MyAcct, strOutbox, nIndex);
            int64_t lRefNum = OTAPI_Wrap::Transaction_GetDisplayReferenceToNum(
                strServerID, strMyNymID, MyAcct, strTrans);
            int64_t lAmount = OTAPI_Wrap::Transaction_GetAmount(
                strServerID, strMyNymID, MyAcct, strTrans);
            string strType = OTAPI_Wrap::Transaction_GetType(
                strServerID, strMyNymID, MyAcct, strTrans);
            string strSenderUserID = OTAPI_Wrap::Transaction_GetSenderUserID(
                strServerID, strMyNymID, MyAcct, strTrans);
            string strSenderAcctID = OTAPI_Wrap::Transaction_GetSenderAcctID(
                strServerID, strMyNymID, MyAcct, strTrans);
            string strRecipientUserID =
                OTAPI_Wrap::Transaction_GetRecipientUserID(
                    strServerID, strMyNymID, MyAcct, strTrans);
            string strRecipientAcctID =
                OTAPI_Wrap::Transaction_GetRecipientAcctID(
                    strServerID, strMyNymID, MyAcct, strTrans);

            string strUserID = strRecipientUserID;
            string strAcctID = strRecipientAcctID;

            bool bUserIDExists = VerifyStringVal(strUserID);
            bool bAcctIDExists = VerifyStringVal(strAcctID);

            string strNewlineSeparator = "";

            if (bUserIDExists || bAcctIDExists) {
                strNewlineSeparator = "\n                                 |";
            }

            string strSeparator =
                (!bUserIDExists && !bAcctIDExists) ? "" : strNewlineSeparator;

            string strUserDenoter = (bUserIDExists ? "U:" : "");
            string strAcctDenoter = (bAcctIDExists ? "A:" : "");

            string strAssetTypeID =
                bAcctIDExists
                    ? OTAPI_Wrap::GetAccountWallet_AssetTypeID(strAcctID)
                    : "";

            string strAmount =
                OT_ERROR_AMOUNT != lAmount
                    ? (VerifyStringVal(strAssetTypeID)
                           ? OTAPI_Wrap::FormatAmount(strAssetTypeID, lAmount)
                           : std::to_string(lAmount))
                    : "UNKNOWN_AMOUNT";

            otOut << nIndex << "    ";
            otOut << strAmount << (strAmount.size() < 3 ? "    " : "   ");
            otOut << strType << (strType.size() > 10 ? " " : "    ");
            otOut << lTransID
                  << (std::to_string(lTransID).size() < 2 ? "    " : "   ");
            otOut << lRefNum
                  << (std::to_string(lRefNum).size() > 2 ? "  " : " ") << "|";
            otOut << strUserDenoter << strUserID << strSeparator
                  << strAcctDenoter << strAcctID << "\n";
        }
    }

    return 1;
}

OT_COMMANDS_OT bool OT_Command::show_mail_message(const string& strMyNymID,
                                                  int32_t nIndex,
                                                  bool bShowContents)
{
    bool bMailVerified = OTAPI_Wrap::Nym_VerifyMailByIndex(strMyNymID, nIndex);

    if (!bMailVerified) {
        cout << "UNVERIFIED mail! At index: " << nIndex << "\n\n";
        //      return false;
    }
    else {
        cout << "------------------------------\nVerified mail at index: "
             << nIndex << "\n";
    }

    string strMailServerID =
        OTAPI_Wrap::GetNym_MailServerIDByIndex(strMyNymID, nIndex);
    string strMailSenderID =
        OTAPI_Wrap::GetNym_MailSenderIDByIndex(strMyNymID, nIndex);
    string strMailContents =
        OTAPI_Wrap::GetNym_MailContentsByIndex(strMyNymID, nIndex);

    if (VerifyStringVal(strMailSenderID)) {
        string strName = OTAPI_Wrap::GetNym_Name(strMailSenderID);
        if (!VerifyStringVal(strName)) {
            strName = "";
        }
        cout << "Mail from: " << strMailSenderID << " \"" << strName << "\" \n";
    }
    if (VerifyStringVal(strMailServerID)) {
        string strName = OTAPI_Wrap::GetServer_Name(strMailServerID);
        if (!VerifyStringVal(strName)) {
            strName = "";
        }
        cout << "Server ID: " << strMailServerID << " \"" << strName << "\" \n";
    }
    if (bShowContents && VerifyStringVal(strMailContents)) {
        cout << "Contents:\n" << strMailContents << "\n\n";
    }

    return true;
}

OT_COMMANDS_OT int32_t OT_Command::details_del_mail(const string& strMyNymID,
                                                    const string& strIndices)
{

    int32_t nCount = OTAPI_Wrap::GetNym_MailCount(strMyNymID);
    if (0 > nCount) {
        otOut << "Error: cannot retrieve mail for Nym: " << strMyNymID << "\n";
        return -1;
    }

    if (0 == nCount) {
        otOut << "\n(Mail box is empty)\n\n";
        return 0;
    }

    bool bIndices = VerifyStringVal(strIndices);
    bool bDeleteAll = false;

    if (bIndices && ("all" == strIndices)) {
        bDeleteAll = true;
    }

    int32_t nIndicesCount =
        (bIndices && !bDeleteAll) ? OTAPI_Wrap::NumList_Count(strIndices) : 0;

    // If we're not deleting ALL, then we must be deleting SPECIFC indices.
    // But no specific indices were supplied!
    // Therefore, error.
    if (!bDeleteAll && (nIndicesCount < 1)) {
        otOut << "Problem: You're apparently not deleting \"all\" indices, but "
                 "neither have you selected SPECIFIC indices.\n Try adding: "
                 "--args \"indices 4,6,7\" (for deleting the messages at "
                 "indices 4, 6, and 7, for example.)\n\n";
        return -1;
    }

    int32_t nReturnVal = 1;

    for (int32_t nIndex = (nCount - 1); nIndex >= 0; --nIndex) {

        // If we're not deleting "all" (i.e. we're deleting specific indices)...
        // and the current index doesn't appear on that specified list, then
        // continue...
        if (!bDeleteAll &&
            !OTAPI_Wrap::NumList_VerifyQuery(strIndices,
                                             std::to_string(nIndex))) {
            // We skip any indices that the user isn't trying to delete.
            //          continue   // apparently not supported by the language.
        }

        // Let's try to remove it...
        else if (OTAPI_Wrap::Nym_RemoveMailByIndex(strMyNymID, nIndex)) {
            otOut << "Deleted mail at index " << nIndex
                  << " for nym: " << strMyNymID << " \n";
        }
        else {
            otOut << "Error while trying to delete mail at index " << nIndex
                  << " for nym: " << strMyNymID << " \n";
            nReturnVal = -1;
        }
    }

    return nReturnVal;
}

OT_COMMANDS_OT int32_t OT_Command::mainDeleteInmail()
{
    otOut << "Usage:   deleteinmail --mynym MY_NYM_ID --args \"index 5\"  (To "
             "delete message 5.)\n Also, try: --args \"indices all\" (for all "
             "messages)\n As well as: --args \"indices 3,5,6\" (for messages "
             "3, 5, and 6)\n\n";

    if (!VerifyExists("MyNym")) return -1;

    string strIndices = "";

    if (VerifyExists("Args", false)) {
        string strIndex = OT_CLI_GetValueByKey(Args, "index");

        if (VerifyStringVal(strIndex)) {
            strIndices = strIndex;
        }

        string strTempIndices = OT_CLI_GetValueByKey(Args, "indices");

        if (VerifyStringVal(strTempIndices)) {
            strIndices = strTempIndices;
        }
    }

    int32_t nDetails = details_del_mail(MyNym, strIndices);
    return nDetails;
}

OT_COMMANDS_OT int32_t OT_Command::mainInmail()
{
    otOut << "Usage:   mail --mynym MY_NYM_ID   (To list all the mail messages "
             "for mynym.)\n Also:   mail --args \"index 5\"  (To examine a "
             "specific message.)\n\n";

    if (!VerifyExists("MyNym")) {
        return -1;
    }

    int32_t nCount = OTAPI_Wrap::GetNym_MailCount(MyNym);
    if (0 > nCount) {
        otOut << "Error: cannot retrieve mail for Nym: " << MyNym << "\n";
        return -1;
    }

    if (0 == nCount) {
        otOut << "\n(Mail box is empty)\n\n";
        return 0;
    }

    int32_t nIndex = -1;
    if (VerifyExists("Args", false)) {
        string strIndex = OT_CLI_GetValueByKey(Args, "index");
        if (VerifyStringVal(strIndex)) {
            nIndex = std::stol(strIndex);
            if (nIndex >= nCount) {
                otOut << "Error: invalid message index: " << strIndex << "\n";
                return -1;
            }
        }
    }

    if (nIndex > -1) {
        if (!show_mail_message(MyNym, nIndex, true)) {
            otOut << "Error: cannot retrieve mail message.\n";
            return -1;
        }
        return 1;
    }

    otOut << "\n Mail contents:\n\n";

    int32_t nReturnVal = 1;
    for (nIndex = 0; nIndex < nCount; ++nIndex) {
        if (!show_mail_message(MyNym, nIndex, false)) {
            otOut << "Error: cannot retrieve mail message.\n";
            nReturnVal = -1;
        }
    }

    return nReturnVal;
}

OT_COMMANDS_OT bool OT_Command::show_outmail_message(const string& strMyNymID,
                                                     int32_t nIndex,
                                                     bool bShowContents)
{
    bool bMailVerified =
        OTAPI_Wrap::Nym_VerifyOutmailByIndex(strMyNymID, nIndex);

    if (!bMailVerified) {
        cout << "UNVERIFIED outmail! At index: " << nIndex << "\n\n";
        //      return false;
    }
    else {
        cout << "--------------------------------------------------------"
                "\nVerified outmail at index: " << nIndex << "\n";
    }

    string strMailServerID =
        OTAPI_Wrap::GetNym_OutmailServerIDByIndex(strMyNymID, nIndex);
    string strMailRecipientID =
        OTAPI_Wrap::GetNym_OutmailRecipientIDByIndex(strMyNymID, nIndex);
    string strMailContents =
        OTAPI_Wrap::GetNym_OutmailContentsByIndex(strMyNymID, nIndex);

    if (VerifyStringVal(strMailRecipientID)) {
        cout << "Mail was sent to: " << strMailRecipientID << "\n";
    }
    if (VerifyStringVal(strMailServerID)) {
        cout << "At server ID: " << strMailServerID << "\n";
    }
    if (bShowContents && VerifyStringVal(strMailContents)) {
        cout << "Contents:\n" << strMailContents << "\n\n";
    }
    return true;
}

OT_COMMANDS_OT int32_t OT_Command::mainOutmail()
{
    otOut << "Usage:   outmail --mynym MY_NYM_ID   (To list all the sent mail "
             "messages for mynym.)\n Also:   outmail --args \"index 5\"  (To "
             "examine a specific message.)\n\n";

    if (!VerifyExists("MyNym")) {
        return -1;
    }

    int32_t nCount = OTAPI_Wrap::GetNym_OutmailCount(MyNym);
    if (0 > nCount) {
        otOut << "Error: cannot retrieve outmail for Nym: " << MyNym << "\n";
        return -1;
    }

    if (0 == nCount) {
        otOut << "\n(Outmail box is empty)\n\n";
        return 0;
    }

    int32_t nIndex = -1;
    if (VerifyExists("Args", false)) {
        string strIndex = OT_CLI_GetValueByKey(Args, "index");
        if (VerifyStringVal(strIndex)) {
            nIndex = std::stol(strIndex);
            if (nIndex >= nCount) {
                otOut << "Error: invalid message index: " << strIndex << "\n";
                return -1;
            }
        }
    }

    if (nIndex > -1) {
        if (!show_outmail_message(MyNym, nIndex, true)) {
            otOut << "Error: cannot retrieve outmail message.\n";
            return -1;
        }
        return 1;
    }

    otOut << "\n Outmail contents:\n\n";

    int32_t nReturnVal = 1;
    for (nIndex = 0; nIndex < nCount; ++nIndex) {
        if (!show_outmail_message(MyNym, nIndex, false)) {
            otOut << "Error: cannot retrieve outmail message.\n";
            nReturnVal = -1;
        }
    }

    return nReturnVal;
}

OT_COMMANDS_OT int32_t OT_Command::details_del_outmail(const string& strMyNymID,
                                                       const string& strIndices)
{
    int32_t nCount = OTAPI_Wrap::GetNym_OutmailCount(strMyNymID);
    if (0 > nCount) {
        otOut << "Error: cannot retrieve outmail for Nym: " << strMyNymID
              << "\n";
        return -1;
    }

    if (0 == nCount) {
        otOut << "\n(Outmail box is empty)\n\n";
        return 0;
    }

    bool bIndices = VerifyStringVal(strIndices);
    bool bDeleteAll = false;

    if (bIndices && ("all" == strIndices)) {
        bDeleteAll = true;
    }

    int32_t nIndicesCount =
        (bIndices && !bDeleteAll) ? OTAPI_Wrap::NumList_Count(strIndices) : 0;

    // If we're not deleting ALL, then we must be deleting SPECIFC indices.
    // But no specific indices were supplied!
    // Therefore, error.
    if (!bDeleteAll && (nIndicesCount < 1)) {
        otOut << "Problem: You're apparently not deleting \"all\" indices, but "
                 "neither have you selected SPECIFIC indices.\n Try adding: "
                 "--args \"indices 4,6,7\" (for deleting the outgoing messages "
                 "at indices 4, 6, and 7, for example.)\n\n";
        return -1;
    }

    int32_t nReturnVal = 1;

    for (int32_t nIndex = (nCount - 1); nIndex >= 0; --nIndex) {

        // If we're not deleting "all" (i.e. we're deleting specific indices)...
        // and the current index doesn't appear on that specified list, then
        // continue...
        if (!bDeleteAll &&
            !OTAPI_Wrap::NumList_VerifyQuery(strIndices,
                                             std::to_string(nIndex))) {
            // We skip any indices that the user isn't trying to delete.
            //          continue // apparently not supported by the language.
        }

        // Let's try to remove it...
        else if (OTAPI_Wrap::Nym_RemoveOutmailByIndex(strMyNymID, nIndex)) {
            otOut << "Deleted outgoing mail at index " << nIndex
                  << " for nym: " << strMyNymID << " \n";
        }
        else {
            otOut << "Error while trying to delete outgoing mail at index "
                  << nIndex << " for nym: " << strMyNymID << " \n";
            nReturnVal = -1;
        }
    }

    return nReturnVal;
}

OT_COMMANDS_OT int32_t OT_Command::mainDeleteOutmail()
{
    otOut << "Usage:   deleteoutmail --mynym MY_NYM_ID --args \"index 5\"  (To "
             "delete outmail message 5.)\n Also, try: --args \"indices all\" "
             "(for all outgoing messages)\n As well as: --args \"indices "
             "3,5,6\" (for outgoing messages 3, 5, and 6)\n\n";

    if (!VerifyExists("MyNym")) return -1;

    string strIndices = "";

    if (VerifyExists("Args", false)) {
        string strIndex = OT_CLI_GetValueByKey(Args, "index");

        if (VerifyStringVal(strIndex)) {
            strIndices = strIndex;
        }

        string strTempIndices = OT_CLI_GetValueByKey(Args, "indices");

        if (VerifyStringVal(strTempIndices)) {
            strIndices = strTempIndices;
        }
    }

    int32_t nDetails = details_del_outmail(MyNym, strIndices);
    return nDetails;
}

OT_COMMANDS_OT bool OT_Command::show_outpayment(const string& strMyNym,
                                                int32_t nIndex,
                                                bool bShowInFull)
{
    bool bMailVerified =
        OTAPI_Wrap::Nym_VerifyOutpaymentsByIndex(strMyNym, nIndex);

    if (!bMailVerified) {
        cout << "UNVERIFIED sent (outgoing) payment! At index: " << nIndex
             << "\n";
        //      return false;
    }
    else {
        cout << "\n----------------------------------------------\n(index "
             << nIndex << ")\n";
    }

    string strMailServerID =
        OTAPI_Wrap::GetNym_OutpaymentsServerIDByIndex(strMyNym, nIndex);
    string strMailRecipientID =
        OTAPI_Wrap::GetNym_OutpaymentsRecipientIDByIndex(strMyNym, nIndex);
    string strMailContents =
        OTAPI_Wrap::GetNym_OutpaymentsContentsByIndex(strMyNym, nIndex);

    if (!VerifyStringVal(strMailContents)) {
        otOut << "Error: bad result from "
                 "OT_API_GetNym_OutpaymentsContentsByIndex at Index: " << nIndex
              << "\n";
        return false;
    }

    int64_t lPaymentAmount = OTAPI_Wrap::Instrmnt_GetAmount(strMailContents);
    string strPaymentAmount = OT_ERROR_AMOUNT != lPaymentAmount
                                  ? std::to_string(lPaymentAmount)
                                  : "UNKNOWN_PAYMENT_AMOUNT";

    string strPaymentAssetID = OTAPI_Wrap::Instrmnt_GetAssetID(strMailContents);
    string strPaymentType = OTAPI_Wrap::Instrmnt_GetType(strMailContents);

    string strFormatted = "";
    string strAssetTypeName = "";

    string strRecipientString = "";
    string strServerString = "";

    if (VerifyStringVal(strMailRecipientID)) {
        string strName = OTAPI_Wrap::GetNym_Name(strMailRecipientID);
        if (!VerifyStringVal(strName)) {
            strName = "";
        }
        strRecipientString =
            "Payment sent to: " + strMailRecipientID + " \"" + strName + "\" ";
    }
    if (VerifyStringVal(strMailServerID)) {
        string strName = OTAPI_Wrap::GetServer_Name(strMailServerID);
        if (!VerifyStringVal(strName)) {
            strName = "";
        }
        strServerString =
            "   At server ID: " + strMailServerID + " \"" + strName + "\" ";
    }

    if (VerifyStringVal(strPaymentAssetID)) {
        strAssetTypeName = OTAPI_Wrap::GetAssetType_Name(strPaymentAssetID);
        if (!VerifyStringVal(strAssetTypeName)) {
            strAssetTypeName = "";
        }
    }
    else {
        strPaymentAssetID = "UNKNOWN_ASSET_ID";
    }

    if (OT_ERROR_AMOUNT != lPaymentAmount) {
        if (!VerifyStringVal(strPaymentType)) {
            strPaymentType = "UNKNOWN_PAYMENT_TYPE";
        }
        string strTempFormat;
        if (lPaymentAmount >= 0) {
            strTempFormat =
                OTAPI_Wrap::FormatAmount(strPaymentAssetID, lPaymentAmount);
        }
        if (!VerifyStringVal(strTempFormat)) {
            strTempFormat = strPaymentAmount;
        }

        strFormatted = "( " + strPaymentType + ": " + strTempFormat + " )";
    }

    cout << "         Amount: " << strPaymentAmount << "  " << strFormatted
         << "\n";
    cout << strRecipientString << "\n";
    cout << "  Of asset type: " << strPaymentAssetID << " \""
         << strAssetTypeName << "\"\n";
    cout << strServerString << "\n";

    if (bShowInFull) {
        cout << "     Instrument: \n" << strMailContents << "\n\n";
    }

    return true;
}

OT_COMMANDS_OT int32_t OT_Command::mainOutpayment()
{
    otOut << "Usage:   outpayment --mynym MY_NYM_ID --args \"index 5\"   (for "
             "example)\nIf no index is specified, all outgoing payments are "
             "listed.\n";

    if (!VerifyExists("MyNym")) {
        return -1;
    }

    int32_t nCount = OTAPI_Wrap::GetNym_OutpaymentsCount(MyNym);
    if (0 > nCount) {
        otOut << "Error: cannot retrieve outpayments for Nym: " << MyNym
              << "\n";
        return -1;
    }

    if (0 == nCount) {
        otOut << "\n(Outpayment box is empty)\n\n";
        return 0;
    }

    int32_t nIndex = -1;
    if (VerifyExists("Args", false)) {
        string strIndex = OT_CLI_GetValueByKey(Args, "index");

        if (VerifyStringVal(strIndex)) {
            nIndex = std::stol(strIndex);
            if (nIndex >= nCount) {
                otOut << "Error: invalid message index: " << strIndex << "\n";
                return 0;
            }
        }
    }

    if (nIndex > -1) {
        if (!show_outpayment(MyNym, nIndex, true)) {
            otOut << "Error: cannot retrieve outpayment.\n";
            return -1;
        }
    }

    otOut << "\n\n===> SHOW OUTGOING PAYMENTS:\n";

    int32_t nReturnVal = 1;
    for (nIndex = 0; nIndex < nCount; ++nIndex) {
        if (!show_outpayment(MyNym, nIndex, false)) {
            otOut << "Error: cannot retrieve outpayment.\n";
            nReturnVal = -1;
        }
    }
    return nReturnVal;
}

OT_COMMANDS_OT int32_t OT_Command::mainAddServer()
{
    otOut << "Please paste a server contract, followed by an EOF or a ~ by "
             "itself on a blank line:\n\n";
    string strContract = OT_CLI_ReadUntilEOF();

    if (!VerifyStringVal(strContract)) {
        otOut << "\n\n Sorry, You must input a server contract, in order to "
                 "add it to your wallet.\n\n";
        return -1;
    }

    int32_t nAdded = OTAPI_Wrap::AddServerContract(strContract);

    if (1 != nAdded) {
        otOut << "\n\n Sorry, failed. Are you sure that was a server "
                 "contract?\n\n";
        return -1;
    }

    otOut << "\n\n Success adding server contract to your wallet.\n\n";
    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainAddAsset()
{
    otOut << "Please paste a currency contract, followed by an EOF or a ~ by "
             "itself on a blank line:\n\n";
    string strContract = OT_CLI_ReadUntilEOF();

    if (!VerifyStringVal(strContract)) {
        otOut << "\n\n Sorry, You must input a currency contract, in order to "
                 "add it to your wallet.\n\n";
        return -1;
    }

    int32_t nAdded = OTAPI_Wrap::AddAssetContract(strContract);

    if (1 != nAdded) {
        otOut << "\n\n Sorry, failed. Are you sure that was an asset "
                 "contract?\n\n";
        return -1;
    }

    otOut << "\n\n Success adding asset contract to your wallet.\n\n";
    return 1;
}

OT_COMMANDS_OT int32_t OT_Command::mainIssueAsset()
{
    otOut << "Usage:   issueasset --server SERVER_ID --mynym NYM_ID\n\n       "
             "(NYM_ID must already be the 'contract' key on the new "
             "contract.)\n       See 'opentxs newasset' before running this "
             "script.\n\n";

    if (!VerifyExists("Server") || !VerifyExists("MyNym")) return -1;

    otOut << "Please paste a currency contract, followed by an EOF or a ~ "
             "by itself on a blank line:\n\n";
    string strContract = OT_CLI_ReadUntilEOF();

    if (!VerifyStringVal(strContract)) {
        otOut << "\n\n Sorry, You must input a currency contract, in order "
                 "to issue it on an OT server.\n\n";
        return -1;
    }

    if (!OTAPI_Wrap::IsNym_RegisteredAtServer(MyNym, Server)) {
        // If the Nym's not registered at the server, then register him
        // first.
        OT_Command::mainRegisterNym();
    }

    string strResponse = MadeEasy::issue_asset_type(Server, MyNym, strContract);
    int32_t nStatus = VerifyMessageSuccess(strResponse);
    switch (nStatus) {
    case 1:
        otOut << "\n\n SUCCESS in issue_asset! Server response:\n\n";
        cout << strResponse << "\n";
        break;
    case 0:
        otOut << "\n\n FAILURE in issue_asset! Server response:\n\n";
        cout << strResponse << "\n";
        break;
    default:
        otOut << "\n\nError in issue_asset! nStatus is: " << nStatus << "\n";

        if (VerifyStringVal(strResponse)) {
            otOut << "Server response:\n\n";
            cout << strResponse << "\n";
        }
        break;
    }
    otOut << "\n\n";

    return (0 == nStatus) ? -1 : nStatus;
}

} // namespace opentxs

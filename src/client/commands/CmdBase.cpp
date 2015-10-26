/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "CmdBase.hpp"

#include "../ot_made_easy_ot.hpp"
#include <opentxs/client/ot_otapi_ot.hpp>
#include "../ot_utility_ot.hpp"

#include <opentxs/client/OpenTransactions.hpp>
#include <opentxs/client/OTAPI.hpp>
#include <opentxs/client/OTWallet.hpp>
#include <opentxs/ext/Helpers.hpp>
#include <opentxs/core/Account.hpp>
#include <opentxs/core/AssetContract.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/Nym.hpp>
#include <opentxs/core/OTServerContract.hpp>

#include <map>
#include <sstream>

using namespace opentxs;
using namespace std;

CmdBase::CmdBase()
    : category(catError)
    , command(nullptr)
    , help(nullptr)
    , usage(nullptr)
{
    for (int i = 0; i < MAX_ARGS; i++) {
        args[i] = nullptr;
    }
}

CmdBase::~CmdBase()
{
}

bool CmdBase::checkAccount(const char* name, string& account) const
{
    if (!checkMandatory(name, account)) {
        return false;
    }

    OTWallet* wallet = getWallet();
    Account* theAccount = wallet->GetAccount(account);
    if (theAccount == nullptr) {
        theAccount = wallet->GetAccountPartialMatch(account);
        if (theAccount == nullptr) {
            otOut << "Error: " << name << ": unknown account: " << account
                  << "\n";
            return false;
        }
    }

    String tmp;
    theAccount->GetPurportedAccountID().GetString(tmp);
    account = tmp.Get();
    otOut << "Using " << name << ": " << account << "\n";
    return true;
}

int64_t CmdBase::checkAmount(const char* name, const string& amount,
                             const string& myacct) const
{
    if (!checkMandatory(name, amount)) {
        return OT_ERROR_AMOUNT;
    }

    string assetType = getAccountAssetType(myacct);
    if ("" == assetType) {
        return OT_ERROR_AMOUNT;
    }

    int64_t value = OTAPI_Wrap::StringToAmount(assetType, amount);
    if (OT_ERROR_AMOUNT == value) {
        otOut << "Error: " << name << ": invalid amount: " << amount << "\n";
        return OT_ERROR_AMOUNT;
    }

    return value;
}

bool CmdBase::checkFlag(const char* name, const string& value) const
{
    if (!checkMandatory(name, value)) {
        return false;
    }

    if (value != "false" && value != "true") {
        otOut << "Error: " << name << ": expected 'false' or 'true'.\n";
        return false;
    }

    return true;
}

int32_t CmdBase::checkIndex(const char* name, const string& index,
                            int32_t items) const
{
    if (!checkValue(name, index)) {
        return -1;
    }

    if (!checkIndicesRange(name, index, items)) {
        return -1;
    }

    return stoi(index);
}

bool CmdBase::checkIndices(const char* name, const string& indices) const
{
    if (!checkMandatory(name, indices)) {
        return false;
    }

    if ("all" == indices) {
        return true;
    }

    for (string::size_type i = 0; i < indices.length(); i++) {
        if (!isdigit(indices[i])) {
            otOut << "Error: " << name << ": not a value: " << indices << "\n";
            return false;
        }
        for (i++; i < indices.length() && isdigit(indices[i]); i++) {
        }
        if (i < indices.length() && ',' != indices[i]) {
            otOut << "Error: " << name << ": not a value: " << indices << "\n";
            return false;
        }
    }

    return true;
}

bool CmdBase::checkIndicesRange(const char* name, const string& indices,
                                int32_t items) const
{
    if ("all" == indices) {
        return true;
    }

    for (string::size_type i = 0; i < indices.length(); i++) {
        int32_t value = 0;
        for (; isdigit(indices[i]); i++) {
            value = value * 10 + indices[i] - '0';
        }
        if (0 > value || value >= items) {
            otOut << "Error: " << name << ": value (" << value
                  << ") out of range (must be < " << items << ")\n";
            return false;
        }
    }

    return true;
}

bool CmdBase::checkMandatory(const char* name, const string& value) const
{
    if ("" == value) {
        otOut << "Error: " << name << ": mandatory parameter not specified.\n";
        return false;
    }

    return true;
}

bool CmdBase::checkNym(const char* name, string& nym, bool checkExistance) const
{
    if (!checkMandatory(name, nym)) {
        return false;
    }

    OTWallet* wallet = getWallet();

    otOut << "Checking nym: " << nym << "\n";

    const Identifier nymID(nym);
    Nym* theNym = wallet->GetOrLoadNym(nymID);
    if (theNym == nullptr) {
        theNym = wallet->GetNymByIDPartialMatch(nym);
        if (theNym == nullptr && checkExistance) {
            otOut << "Error: " << name << ": unknown nym: " << nym << "\n";
            return false;
        }
    }

    if (theNym) {
        String tmp;
        theNym->GetIdentifier(tmp);
        nym = tmp.Get();
    }
    otOut << "Using " << name << ": " << nym << "\n";
    return true;
}

bool CmdBase::checkPurse(const char* name, string& purse) const
{
    if (!checkMandatory(name, purse)) {
        return false;
    }

    OTWallet* wallet = getWallet();
    AssetContract* thePurse = wallet->GetAssetContract(purse);
    if (thePurse == nullptr) {
        thePurse = wallet->GetAssetContractPartialMatch(purse);
        if (thePurse == nullptr) {
            otOut << "Error: " << name << ": unknown purse: " << purse << "\n";
            return false;
        }
    }

    String tmp;
    thePurse->GetIdentifier(tmp);
    purse = tmp.Get();
    otOut << "Using " << name << ": " << purse << "\n";
    return true;
}

bool CmdBase::checkServer(const char* name, string& server) const
{
    if (!checkMandatory(name, server)) {
        return false;
    }

    OTWallet* wallet = getWallet();
    OTServerContract* theServer = wallet->GetServerContract(server);
    if (theServer == nullptr) {
        theServer = wallet->GetServerContractPartialMatch(server);
        if (theServer == nullptr) {
            otOut << "Error: " << name << ": unknown server: " << server
                  << "\n";
            return false;
        }
    }

    String tmp;
    theServer->GetIdentifier(tmp);
    server = tmp.Get();
    otOut << "Using " << name << ": " << server << "\n";
    return true;
}

int64_t CmdBase::checkTransNum(const char* name, const string& id) const
{
    if (!checkMandatory(name, id)) {
        return -1;
    }

    for (string::size_type i = 0; i < id.length(); i++) {
        if (!isdigit(id[i])) {
            otOut << "Error: " << name << ": not a value: " << id << "\n";
            return -1;
        }
    }

    int64_t value = stoll(id);
    if (0 >= value) {
        otOut << "Error: " << name << ": invalid value: " << id << "\n";
        return -1;
    }

    return value;
}

bool CmdBase::checkValue(const char* name, const string& value) const
{
    if (!checkMandatory(name, value)) {
        return false;
    }

    for (string::size_type i = 0; i < value.length(); i++) {
        if (!isdigit(value[i])) {
            otOut << "Error: " << name << ": not a value: " << value << "\n";
            return false;
        }
    }

    return true;
}

void CmdBase::dashLine() const
{
    // 76 dashes :-)
    cout << "--------------------------------------"
            "--------------------------------------\n";
}

const vector<string>& CmdBase::extractArgumentNames()
{
    // only do this once
    if (0 != argNames.size()) {
        return argNames;
    }

    // extract argument names from usage help text
    for (int i = 0; i < MAX_ARGS && args[i] != nullptr; i++) {
        const char* arg = args[i];
        while ('[' == *arg || '-' == *arg) {
            arg++;
        }
        string argName = "";
        for (; isalpha(*arg); arg++) {
            argName += *arg;
        }
        argNames.push_back(argName);
    }

    return argNames;
}

string CmdBase::formatAmount(const string& assetType, int64_t amount) const
{
    if (OT_ERROR_AMOUNT == amount) {
        // this probably should not happen
        return "UNKNOWN_AMOUNT";
    }

    if ("" == assetType) {
        // just return unformatted
        return to_string(amount);
    }

    return OTAPI_Wrap::FormatAmount(assetType, amount);
}

Category CmdBase::getCategory() const
{
    return category;
}

const char* CmdBase::getCommand() const
{
    return command;
}

const char* CmdBase::getHelp() const
{
    return help;
}

string CmdBase::getAccountAssetType(const string& myacct) const
{
    string assetType =
        OTAPI_Wrap::GetAccountWallet_InstrumentDefinitionID(myacct);
    if ("" == assetType) {
        otOut << "Error: cannot load instrument definition from myacct.\n";
    }
    return assetType;
}

string CmdBase::getOption(string optionName) const
{
    auto result = options.find(optionName);
    if (result == options.end()) {
        otWarn << "Option " << optionName << " not found.\n";
        return "";
    }

    otInfo << "Option  " << result->first << ": " << result->second << "\n";
    return result->second;
}

string CmdBase::getUsage() const
{
    stringstream ss;

    // construct usage string
    ss << "Usage:   " << command;
    for (int i = 0; i < MAX_ARGS && args[i] != nullptr; i++) {
        ss << " " << args[i];
    }
    ss << "\n\n" << help << "\n\n";
    if (usage != nullptr) {
        ss << usage << "\n\n";
    }

    return ss.str();
}

OTWallet* CmdBase::getWallet() const
{
    OTWallet* wallet = OTAPI_Wrap::OTAPI()->GetWallet();
    OT_ASSERT_MSG(wallet != nullptr, "Cannot load wallet->\n");
    return wallet;
}

int32_t CmdBase::harvestTxNumbers(const string& contract, const string& mynym)
{
    OTAPI_Wrap::Msg_HarvestTransactionNumbers(contract, mynym, false, false,
                                              false, false, false);
    return -1;
}

string CmdBase::inputLine()
{
    return OT_CLI_ReadLine();
}

string CmdBase::inputText(const char* what)
{
    cout << "Please paste " << what << ",\n"
         << "followed by an EOF or a ~ on a line by itself:\n";

    string input = OT_CLI_ReadUntilEOF();
    if ("" == input) {
        otOut << "Error: you did not paste " << what << ".\n";
    }
    return input;
}

int32_t CmdBase::processResponse(const string& response, const char* what) const
{
    switch (responseStatus(response)) {
    case 1:
        break;

    case 0:
        otOut << "Error: failed to " << what << ".\n";
        return -1;

    default:
        otOut << "Error: cannot " << what << ".\n";
        return -1;
    }

    cout << response << "\n";
    return 1;
}

int32_t CmdBase::processTxResponse(const string& server, const string& mynym,
                                   const string& myacct, const string& response,
                                   const char* what) const
{
    if (1 != responseStatus(response)) {
        otOut << "Error: cannot " << what << ".\n";
        return -1;
    }

    if (1 != VerifyMsgBalanceAgrmntSuccess(server, mynym, myacct, response)) {
        otOut << "Error: " << what << " balance agreement failed.\n";
        return -1;
    }

    if (1 != VerifyMsgTrnxSuccess(server, mynym, myacct, response)) {
        otOut << "Error: " << what << " transaction failed.\n";
        return -1;
    }

    cout << response << "\n";

    return 1;
}

int32_t CmdBase::responseReply(const string& response, const string& server,
                               const string& mynym, const string& myacct,
                               const char* function) const
{
    return InterpretTransactionMsgReply(server, mynym, myacct, function,
                                        response);
}

int32_t CmdBase::responseStatus(const string& response) const
{
    return VerifyMessageSuccess(response);
}

bool CmdBase::run(const map<string, string>& _options)
{
    options = _options;
    int32_t returnValue = runWithOptions();
    options.clear();

    switch (returnValue) {
    case 0: // no action performed, return success
        return true;
    case 1: // success
        return true;
    case -1: // failed
        return false;
    default:
        otOut << "Error: undefined error code: " << returnValue << ".\n";
        break;
    }

    return false;
}

vector<string> CmdBase::tokenize(const string& str, char delim,
                                 bool noEmpty) const
{
    vector<string> tokens;

    const char* p = str.c_str();
    int begin = 0;
    while (true) {
        int next = begin;
        while (p[next] != delim && '\0' != p[next]) {
            next++;
        }
        if (next != begin || !noEmpty) {
            tokens.push_back(str.substr(begin, next - begin));
        }
        if ('\0' == p[next]) {
            break;
        }
        begin = next + 1;
    }

    return tokens;
}

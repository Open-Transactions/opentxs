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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/client/OTME_too.hpp"

#include "opentxs/api/Api.hpp"
#include "opentxs/api/Identity.hpp"
#include "opentxs/api/OT.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/client/MadeEasy.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/OTAPI_Wrap.hpp"
#include "opentxs/client/OT_ME.hpp"
#include "opentxs/core/crypto/CryptoEncodingEngine.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#ifdef ANDROID
#include "opentxs/core/util/android_string.hpp"
#endif  // ANDROID
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"

#include <chrono>
#include <functional>
#include <sstream>

#define MASTER_SECTION "Master"
#define PAIRED_NODES_KEY "paired_nodes"
#define INTRODUCTION_SERVER_KEY "introduction_server_id"
#define PAIRED_SECTION_PREFIX "paired_node_"
#define BRIDGE_NYM_KEY "bridge_nym_id"
#define ADMIN_PASSWORD_KEY "admin_password"
#define OWNER_NYM_KEY "owner_nym_id"
#define NOTARY_ID_KEY "notary_id"
#define BACKUP_KEY "backup"
#define CONNECTED_KEY "connected"
#define DONE_KEY "done"
#define ISSUED_UNITS_KEY "issued_units"
#define ISSUED_UNIT_PREFIX_KEY "issued_unit_"
#define ASSET_ID_PREFIX_KEY "unit_definition_"
#define ACCOUNT_ID_PREFIX_KEY "account_id_"
#define NYM_REVISION_SECTION_PREFIX "nym_revision_"
#define RENAME_KEY "rename_started"
#define CONTACT_COUNT_KEY "contacts"
#define CONTACT_SECTION_PREFIX "contact_"
#define CONTACT_NYMID_KEY "nymid"
#define CONTACT_PAYMENTCODE_KEY "payment_code"
#define CONTACT_UPDATED_KEY "updated"
#define CONTACT_REVISION_KEY "revision"
#define CONTACT_LABEL_KEY "label"
#define CONTACT_REFRESH_DAYS 7
#define ALL_SERVERS "all"

#define OT_METHOD "OTME_too::"

namespace opentxs
{

const std::string OTME_too::DEFAULT_INTRODUCTION_SERVER =
    R"(-----BEGIN OT ARMORED SERVER CONTRACT-----
Version: Open Transactions 0.99.1-113-g2b3acf5
Comment: http://opentransactions.org

CAESI290b20xcHFmREJLTmJLR3RiN0NBa0ZodFRXVFVOTHFIRzIxGiNvdHVkd3p4cWF0UHh4
bmh4VFV3RUo3am5HenE2RkhGYTRraiIMU3Rhc2ggQ3J5cHRvKr8NCAESI290dWR3enhxYXRQ
eHhuaHhUVXdFSjdqbkd6cTZGSEZhNGtqGAIoATJTCAEQAiJNCAESIQI9MywLxxKfOtai26pj
JbxKtCCPhM/DbvX08iwbW2qYqhoga6Ccvp6CABGAFj/RdWNjtg5uzIRHT5Dn+fUzdAM9SUSA
AQCIAQA6vAwIARIjb3R1ZHd6eHFhdFB4eG5oeFRVd0VKN2puR3pxNkZIRmE0a2oaI290dXdo
ZzNwb2kxTXRRdVkzR3hwYWpOaXp5bmo0NjJ4Z2RIIAIymgQIARIjb3R1d2hnM3BvaTFNdFF1
WTNHeHBhak5penluajQ2MnhnZEgYAiABKAIyI290dWR3enhxYXRQeHhuaHhUVXdFSjdqbkd6
cTZGSEZhNGtqQl0IARJTCAEQAiJNCAESIQI9MywLxxKfOtai26pjJbxKtCCPhM/DbvX08iwb
W2qYqhoga6Ccvp6CABGAFj/RdWNjtg5uzIRHT5Dn+fUzdAM9SUSAAQCIAQAaBAgBEAJKiAEI
ARACGioIARAEGAIgASogZ6MtTp4aTEDLxFfhnsGo+Esp5B4hkgjWEejNPt5J6C0aKggBEAQY
AiACKiAhqJjWf2Ugqbg6z6ps59crAx9lHwTuT6Eq4x6JmkBlGBoqCAEQBBgCIAMqII2Vps1F
C2YUMbB4kE9XsHt1jrVY6pMPV6KWc5sH3VvTem0IARIjb3R1d2hnM3BvaTFNdFF1WTNHeHBh
ak5penluajQ2MnhnZEgYASAFKkDQLsszAol/Ih56MomuBKV8zpKaw5+ry7Kse1+5nPwJlP8f
72OAgTegBlmv31K4JgLVs52EKJTBpjnV+v0pxzUOem0IARIjb3R1ZHd6eHFhdFB4eG5oeFRV
d0VKN2puR3pxNkZIRmE0a2oYAyAFKkAJZ0LTVM+XBrGbRdiZsEQSbvwqg+mqGwHD5MQ+D4h0
fPQaUrdB6Pp/HM5veox02LBKg05hVNQ64tcU+LAxK+VHQuQDCAESI290clA2dDJXY2hYMjYz
ZVpiclRuVzZyY2FCZVNQb2VqSzJnGAIgAigCMiNvdHVkd3p4cWF0UHh4bmh4VFV3RUo3am5H
enE2RkhGYTRrajonCAESI290dXdoZzNwb2kxTXRRdVkzR3hwYWpOaXp5bmo0NjJ4Z2RISogB
CAEQAhoqCAEQBBgCIAEqIDpwlCrxHNWvvtFt6k8ocB5NBo7vjkGO/mRuSOQ/j/9WGioIARAE
GAIgAiog6Dw0+AWok4dENWWc/3qhykA7NNybWecqMGs5fL8KLLYaKggBEAQYAiADKiD+s/iq
37NrYI4/xdHOYtO/ocR0YqDVz09IaDNGVEdBtnptCAESI290clA2dDJXY2hYMjYzZVpiclRu
VzZyY2FCZVNQb2VqSzJnGAEgBSpATbHtakma53Na35Be+rGvW+z1H6EtkHlljv9Mo8wfies3
in9el1Ejb4BDbGCN5ABl3lQpfedZnR+VYv2X6Y1yBnptCAESI290dXdoZzNwb2kxTXRRdVkz
R3hwYWpOaXp5bmo0NjJ4Z2RIGAEgBSpAeptEmgdqgkGUcOJCqG0MsiChEREUdDzH/hRj877u
WDIHoRHsf/k5dCOHfDct4TDszasVhGFhRdNunpgQJcp0DULnAwgBEiNvdHd6ZWd1dTY3cENI
RnZhYjZyS2JYaEpXelNvdlNDTGl5URgCIAIoAjIjb3R1ZHd6eHFhdFB4eG5oeFRVd0VKN2pu
R3pxNkZIRmE0a2o6JwgBEiNvdHV3aGczcG9pMU10UXVZM0d4cGFqTml6eW5qNDYyeGdkSEqL
AQgBEAIaKwgBEAMYAiABKiEC5p36Ivxs4Wb6CjKTnDA1MFtX3Mx2UBlrmloSt+ffXz0aKwgB
EAMYAiACKiECtMkEo4xsefeevzrBb62ll98VYZy8PipgbrPWqGUNxQMaKwgBEAMYAiADKiED
W1j2DzOZemB9OOZ/pPrFroKDfgILYu2IOtiRFfi0vDB6bQgBEiNvdHd6ZWd1dTY3cENIRnZh
YjZyS2JYaEpXelNvdlNDTGl5URgBIAUqQJYd860/Ybh13GtW+grxWtWjjmzPifHE7bTlgUWl
3bX+ZuWNeEotA4yXQvFNog4PTAOF6dbvCr++BPGepBEUEEx6bQgBEiNvdHV3aGczcG9pMU10
UXVZM0d4cGFqTml6eW5qNDYyeGdkSBgBIAUqQH6GXnKCCDDgDvcSt8dLWuVMlr75zVkHy85t
tccoy2oLHNevDvKrLfUk/zuICyaSIvDy0Kb2ytOuh/O17yabxQ8yHQgBEAEYASISb3Quc3Rh
c2hjcnlwdG8ubmV0KK03MiEIARADGAEiFnQ1NGxreTJxM2w1ZGt3bnQub25pb24orTcyRwgB
EAQYASI8b3ZpcDZrNWVycXMzYm52cjU2cmgzZm5pZ2JuZjJrZWd1cm5tNWZpYnE1NWtqenNv
YW54YS5iMzIuaTJwKK03Op8BTWVzc2FnaW5nLW9ubHkgc2VydmVyIHByb3ZpZGVkIGZvciB0
aGUgY29udmllbmllbmNlIG9mIFN0YXNoIENyeXB0byB1c2Vycy4gU2VydmljZSBpcyBwcm92
aWRlZCBhcyBpcyB3aXRob3V0IHdhcnJhbnR5IG9mIGFueSBraW5kLCBlaXRoZXIgZXhwcmVz
c2VkIG9yIGltcGxpZWQuQiCK4L5cnecfUFz/DQyvAklKC2pTmWQtxt9olQS5/0hUHUptCAES
I290clA2dDJXY2hYMjYzZVpiclRuVzZyY2FCZVNQb2VqSzJnGAUgBSpA1/bep0NTbisZqYns
MCL/PCUJ6FIMhej+ROPk41604x1jeswkkRmXRNjzLlVdiJ/pQMxG4tJ0UQwpxHxrr0IaBA==

-----END OT ARMORED SERVER CONTRACT-----)";

OTME_too::Cleanup::Cleanup(std::atomic<bool>& run)
    : run_(run)
{
    run_.store(true);
}

OTME_too::Cleanup::~Cleanup() { run_.store(false); }

OTME_too::OTME_too(
    std::recursive_mutex& lock,
    Settings& config,
    OT_API& otapi,
    OTAPI_Exec& exec,
    const MadeEasy& madeEasy,
    const OT_ME& otme,
    Wallet& wallet,
    CryptoEncodingEngine& encoding,
    Identity& identity)
    : api_lock_(lock)
    , config_(config)
    , ot_api_(otapi)
    , exec_(exec)
    , made_easy_(madeEasy)
    , otme_(otme)
    , wallet_(wallet)
    , encoding_(encoding)
    , identity_(identity)
{
    pairing_.store(false);
    refreshing_.store(false);
    shutdown_.store(false);
    refresh_count_.store(0);
    scan_pairing();
    scan_contacts();
}

bool OTME_too::AcceptIncoming(
    const Identifier& nymID,
    const Identifier& accountID,
    const Identifier& serverID)
{
    rLock apiLock(api_lock_);
    const std::string account = String(accountID).Get();
    auto context = wallet_.mutable_ServerContext(nymID, serverID);
    auto processInbox = ot_api_.CreateProcessInbox(accountID, context.It());
    auto& response = std::get<0>(processInbox);
    auto& inbox = std::get<1>(processInbox);

    if (false == bool(response)) {
        if (nullptr == inbox) {
            // This is a new account which has never instantiated an inbox.

            return true;
        }

        otErr << OT_METHOD << __FUNCTION__
              << ": Error instantiating processInbox for account: " << account
              << std::endl;

        return false;
    }

    const auto items = inbox->GetTransactionCount();

    if (0 == items) {
        otInfo << OT_METHOD << __FUNCTION__
               << ": No items to accept in this account." << std::endl;

        return true;
    }

    for (std::int32_t i = 0; i < items; i++) {
        auto transaction = inbox->GetTransactionByIndex(i);

        OT_ASSERT(nullptr != transaction);

        const TransactionNumber number = transaction->GetTransactionNum();

        if (transaction->IsAbbreviated()) {
            inbox->LoadBoxReceipt(number);
            transaction = inbox->GetTransaction(number);

            if (nullptr == transaction) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Unable to load item: " << number << std::endl;

                continue;
            }
        }

        const bool accepted = ot_api_.IncludeResponse(
            accountID, true, context.It(), *transaction, *response);

        if (!accepted) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed to accept item: " << number << std::endl;

            return false;
        }
    }

    const bool finalized = ot_api_.FinalizeProcessInbox(
        accountID, context.It(), *response, *inbox);

    if (false == finalized) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to finalize response."
              << std::endl;

        return false;
    }

    const auto result = otme_.process_inbox(
        String(serverID).Get(),
        String(nymID).Get(),
        String(accountID).Get(),
        String(*response).Get());

    return (
        1 ==
        otme_.InterpretTransactionMsgReply(
            String(serverID).Get(),
            String(nymID).Get(),
            String(accountID).Get(),
            "process_inbox",
            result));
}

Identifier OTME_too::add_background_thread(BackgroundThread thread)
{
    Identifier output;

    if (shutdown_.load()) {
        return output;
    }

    std::unique_lock<std::mutex> lock(thread_lock_);

    Data nonce;
    encoding_.Nonce(32, nonce);
    output.CalculateDigest(nonce);
    auto& item = threads_[output];
    auto& running = std::get<0>(item);
    auto& handle = std::get<1>(item);
    auto& exitStatus = std::get<2>(item);

    OT_ASSERT(!handle);

    running.store(false);
    handle.reset(new std::thread(thread, &running, &exitStatus));

    return output;
}

void OTME_too::add_checknym_tasks(
    const nymAccountMap nyms,
    serverTaskMap& tasks)
{
    for (const auto nym : nyms) {
        const auto& serverID = nym.first;
        const auto& nymList = nym.second;
        const bool all = (ALL_SERVERS == serverID);

        if (all) {
            for (auto& it : tasks) {
                auto& server = it.second;
                server.second = nymList;
            }
        } else {
            tasks[serverID].second = nymList;
        }
    }
}

bool OTME_too::add_update_contact(
    const Lock& lock,
    const std::string& nymID,
    const std::string& paymentCode,
    const std::string& label)
{
    OT_ASSERT(verify_lock(lock, contact_lock_));

    if (nymID.empty()) {
        otErr << __FUNCTION__ << ": Empty contact nym id." << std::endl;

        return false;
    }

    std::string bip47 = paymentCode;
    std::uint64_t version = 0;
    auto nym = wallet_.Nym(Identifier(nymID));

    if (nym) {
        version = nym->Revision();

        if (bip47.empty()) {
            bip47 = nym->PaymentCode();
        }
    }

    const auto now = std::time(nullptr);
    // Negative number means "not found"
    auto index = find_contact(nymID, lock);

    if (0 > index) {
        // This is the next index value
        index = contact_map_.size();
    }

    auto& meta = contact_map_[index];
    auto& mapNymID = std::get<0>(meta);
    auto& mapRevision = std::get<1>(meta);
    auto& checked = std::get<2>(meta);
    auto& task = std::get<3>(meta);
    auto& name = std::get<4>(meta);
    auto& code = std::get<5>(meta);
    mapNymID = nymID;
    mapRevision = version;
    checked = now;

    if (!label.empty()) {
        name = label;
    }

    if (!bip47.empty()) {
        code = bip47;
    }

    if (!nym) {
        find_nym_if_necessary(nymID, task);
    }

    return write_contact_data(lock);
}

bool OTME_too::AddContact(const std::string& id, const std::string label)
{
    Lock lock(contact_lock_);

    std::string nymID;
    std::string paymentCode;
    PaymentCode code(id);

    const bool isNymID = Identifier::validateID(id);
    const bool isPaymentCode = code.VerifyInternally();

    if (isNymID) {
        nymID = id;
        paymentCode = "";
    } else if (isPaymentCode) {
        nymID = String(code.ID()).Get();
        paymentCode = code.asBase58();
    } else {
        otErr << __FUNCTION__ << ": Invalid contact id." << std::endl;

        return false;
    }

    return add_update_contact(lock, nymID, paymentCode, label);
}

void OTME_too::build_account_list(serverTaskMap& output) const
{
    // Make sure no nyms, servers, or accounts are added or removed while
    // creating the list
    std::unique_lock<std::recursive_mutex> apiLock(api_lock_);
    const auto serverList = wallet_.ServerList();
    const auto nymCount = exec_.GetNymCount();
    const auto accountCount = exec_.GetAccountCount();

    for (const auto server : serverList) {
        const auto& serverID = server.first;

        for (std::int32_t n = 0; n < nymCount; n++) {
            const auto nymID = exec_.GetNym_ID(n);

            if (exec_.IsNym_RegisteredAtServer(nymID, serverID)) {
                auto& tasks = output[serverID];
                auto& accounts = tasks.first;
                accounts.insert({nymID, {}});
            }
        }
    }

    if (!yield()) {
        return;
    }

    for (std::int32_t n = 0; n < accountCount; n++) {
        const auto accountID = exec_.GetAccountWallet_ID(n);
        const auto serverID = exec_.GetAccountWallet_NotaryID(accountID);
        const auto nymID = exec_.GetAccountWallet_NymID(accountID);

        auto& tasks = output[serverID];
        auto& accounts = tasks.first;
        auto& nym = accounts[nymID];
        nym.push_back(accountID);
    }

    apiLock.unlock();
    yield();
}

void OTME_too::build_nym_list(std::list<std::string>& output) const
{
    output.clear();

    // Make sure no nyms are added or removed while creating the list
    std::unique_lock<std::recursive_mutex> apiLock(api_lock_);
    const auto nymCount = exec_.GetNymCount();

    for (std::int32_t n = 0; n < nymCount; n++) {
        output.push_back(exec_.GetNym_ID(n));
    }

    apiLock.unlock();
}

Messagability OTME_too::can_message(
    const std::string& sender,
    const std::string& recipient,
    std::string& server)
{
    auto senderNym = wallet_.Nym(Identifier(sender));

    if (!senderNym) {
        return Messagability::MISSING_SENDER;
    }

    const bool canSign = senderNym->hasCapability(NymCapability::SIGN_MESSAGE);

    if (!canSign) {
        return Messagability::INVALID_SENDER;
    }

    auto recipientNym = wallet_.Nym(Identifier(recipient));

    if (!recipientNym) {
        mailability(sender, recipient);

        return Messagability::MISSING_RECIPIENT;
    }

    const auto claims = recipientNym->ContactData();

    if (!claims) {
        return Messagability::NO_SERVER_CLAIM;
    }

    server = extract_server(*claims);

    if (server.empty()) {
        return Messagability::NO_SERVER_CLAIM;
    }

    const bool registered = exec_.IsNym_RegisteredAtServer(sender, server);

    if (!registered) {
        mailability(sender, recipient);

        return Messagability::UNREGISTERED;
    }

    return Messagability::READY;
}

Messagability OTME_too::CanMessage(
    const std::string& sender,
    const std::string& recipient)
{
    std::string notUsed;

    return can_message(sender, recipient, notUsed);
}

bool OTME_too::check_accounts(PairedNode& node)
{
    const auto& owner = std::get<1>(node);
    const auto& notaryID = std::get<3>(node);

    if (!check_server_registration(owner, notaryID, false, true)) {

        return false;
    }

    auto& unitMap = std::get<4>(node);
    auto& accountMap = std::get<5>(node);
    const auto needed = unitMap.size();
    std::uint64_t have = 0;
    unitTypeMap neededAccounts;
    typeUnitMap neededUnitTypes;

    for (const auto unit : unitMap) {
        const auto& type = unit.first;
        const auto& unitID = unit.second;

        if ((accountMap.find(type) == accountMap.end()) ||
            (accountMap[type].empty())) {
            neededAccounts[type] = unitID;
            neededUnitTypes[unitID] = type;
        } else {
            have++;
        }
    }

    OT_ASSERT(neededAccounts.size() == neededUnitTypes.size());

    // Just in case these were created but failed to be added to config file
    fill_existing_accounts(owner, have, neededUnitTypes, neededAccounts, node);

    if (!yield()) {
        return false;
    }

    OT_ASSERT(neededAccounts.size() == neededUnitTypes.size());

    for (const auto& currency : neededAccounts) {
        if (obtain_asset_contract(owner, currency.second, notaryID)) {
            const auto accountID =
                obtain_account(owner, currency.second, notaryID);

            if (!accountID.empty()) {
                std::unique_lock<std::mutex> lock(pair_lock_);
                accountMap[currency.first] = accountID;
                lock.unlock();
                have++;
            }
        }
    }

    return (needed == have);
}

bool OTME_too::check_backup(const std::string& bridgeNymID, PairedNode& node)
{
    auto& backup = std::get<6>(node);

    if (backup) {
        return true;
    }

    if (!send_backup(bridgeNymID, node)) {
        return false;
    }

    bool dontCare = false;
    String section = PAIRED_SECTION_PREFIX;
    String sectionKey = std::to_string(std::get<0>(node)).c_str();
    section.Concatenate(sectionKey);
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);

    if (!config_.Set_bool(section, BACKUP_KEY, true, dontCare)) {

        return false;
    }

    return config_.Save();
}

bool OTME_too::check_bridge_nym(const std::string& bridgeNym, PairedNode& node)
{
    const auto& ownerNym = std::get<1>(node);
    auto bridge = obtain_nym(ownerNym, bridgeNym, "");

    if (!bridge) {
        return false;
    }

    auto claims = obtain_contact_data(ownerNym, *bridge, "");

    if (!claims) {
        return false;
    }

    auto server = obtain_server_id(ownerNym, bridgeNym, *claims);

    if (server.empty()) {
        otErr << __FUNCTION__ << ": Bridge nym does not advertise a notary."
              << std::endl;

        return false;
    }

    if (!update_notary(server, node)) {
        return false;
    }

    if (!yield()) {
        return false;
    }

    if (!obtain_server_contract(ownerNym, server, true)) {
        return false;
    }

    if (!obtain_assets(bridgeNym, *claims, node)) {
        return false;
    }

    if (!update_assets(node)) {
        return false;
    }

    if (!yield()) {
        return false;
    }

    return true;
}

bool OTME_too::check_introduction_server(const std::string& withNym) const
{
    if (withNym.empty()) {
        otErr << __FUNCTION__ << ": Invalid nym." << std::endl;

        return false;
    }

    std::string serverID = get_introduction_server();

    if (!yield()) {
        return false;
    }

    if (serverID.empty()) {
        otErr << __FUNCTION__ << ": No introduction server configured."
              << std::endl;

        return false;
    }

    return check_server_registration(withNym, serverID, false, true);
}

bool OTME_too::check_nym_revision(
    const std::string& nymID,
    const std::string& server) const
{
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);

    auto nym = wallet_.Nym(Identifier(nymID));

    if (!nym) {
        return false;
    }

    bool dontCare = false;
    String section = NYM_REVISION_SECTION_PREFIX;
    section.Concatenate(String(nymID));
    const String key(server);
    const std::int64_t local(nym->Revision());
    std::int64_t remote = 0;

    if (!config_.Check_long(section, key, remote, dontCare)) {
        return false;
    }

    return (local == remote);
}

bool OTME_too::check_pairing(
    const std::string& bridgeNym,
    std::string& password)
{
    bool output = false;

    std::lock_guard<std::mutex> lock(pair_lock_);

    auto it = paired_nodes_.find(bridgeNym);

    if (paired_nodes_.end() != it) {
        auto& node = it->second;
        const auto& nodeIndex = std::get<0>(node);
        const auto& owner = std::get<1>(node);
        auto& existingPassword = std::get<2>(node);

        output = insert_at_index(
            nodeIndex, PairedNodeCount(), owner, bridgeNym, password);

        if (output) {
            existingPassword = password;
        }
    }

    return output;
}

void OTME_too::check_server_names()
{
    ServerNameData serverIDs;
    std::unique_lock<std::mutex> lock(pair_lock_);

    for (const auto& it : paired_nodes_) {
        const auto& bridgeNymID = it.first;
        const auto& node = it.second;
        const auto& done = std::get<9>(node);

        if (!done) {
            const auto& owner = std::get<1>(node);
            const auto& password = std::get<2>(node);
            const auto& notary = std::get<3>(node);
            serverIDs.emplace(
                notary,
                std::tuple<std::string, std::string, std::string>{
                    owner, bridgeNymID, password});
        }
    }

    lock.unlock();

    if (!yield()) {
        return;
    }

    set_server_names(serverIDs);
}

bool OTME_too::check_server_registration(
    const std::string& nym,
    const std::string& server,
    const bool force,
    const bool publish) const
{
    if (!force) {
        const bool registered = exec_.IsNym_RegisteredAtServer(nym, server);

        if (!yield()) {
            return false;
        };

        if (registered) {
            return true;
        }
    }

    const bool updated = RegisterNym(nym, server, publish);

    if (!updated) {
        return false;
    }

    return update_nym_revision(nym, server);
}

void OTME_too::clean_background_threads()
{
    shutdown_.store(true);

    while (0 < threads_.size()) {
        for (auto it = threads_.begin(); it != threads_.end();) {
            auto& item = it->second;
            auto& running = std::get<0>(item);
            auto& handle = std::get<1>(item);

            if (!running.load()) {
                if (handle) {
                    if (handle->joinable()) {
                        handle->join();
                    }

                    handle.reset();
                }

                it = threads_.erase(it);
            } else {
                it++;
            }
        }
    }
}

std::uint64_t OTME_too::ContactCount() const
{
    std::int64_t result = 0;
    bool notUsed = false;
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
    config_.Check_long(MASTER_SECTION, CONTACT_COUNT_KEY, result, notUsed);

    if (1 > result) {
        return 0;
    }

    return result;
}

std::list<std::string> OTME_too::ContactList() const
{
    Lock lock(contact_lock_);

    std::list<std::string> output;

    for (const auto& contact : contact_map_) {
        const auto& nymID = std::get<0>(contact.second);
        output.emplace_back(nymID);
    }

    return output;
}

std::string OTME_too::ContactName(const std::string& nymID)
{
    Lock lock(contact_lock_);
    std::string output;
    std::string code;

    const auto index = find_contact(nymID, lock);
    const bool haveContact = (-1 != index);

    if (haveContact) {
        output = std::get<4>(contact_map_.at(index));

        if (output.empty()) {
            const auto nym = wallet_.Nym(Identifier(nymID));

            if (nym) {
                output = nym->Alias();
                code = nym->PaymentCode();
            }

            if (!output.empty()) {
                add_update_contact(lock, nymID, code, output);
            }
        }
    } else {
        const auto nym = wallet_.Nym(Identifier(nymID));

        if (nym) {
            output = nym->Alias();
            code = nym->PaymentCode();
            add_update_contact(lock, nymID, code, output);
        }
    }

    return output;
}

std::string OTME_too::ContactPaymentCode(const std::string& nymID)
{
    Lock lock(contact_lock_);
    std::string output;

    const auto index = find_contact(nymID, lock);
    const bool haveContact = (-1 != index);

    if (haveContact) {
        output = std::get<5>(contact_map_.at(index));

        if (output.empty()) {
            const auto nym = wallet_.Nym(Identifier(nymID));

            if (nym) {
                output = nym->PaymentCode();
            }

            if (!output.empty()) {
                add_update_contact(lock, nymID, output, "");
            }
        }
    } else {
        const auto nym = wallet_.Nym(Identifier(nymID));

        if (nym) {
            output = nym->PaymentCode();
            add_update_contact(lock, nymID, output, "");
        }
    }

    return output;
}

bool OTME_too::download_nym(
    const std::string& localNym,
    const std::string& remoteNym,
    const std::string& server) const
{
    std::string serverID;

    if (server.empty()) {
        serverID = get_introduction_server();
    } else {
        serverID = server;
    }

    const auto result = otme_.check_nym(serverID, localNym, remoteNym);

    if (!yield()) {
        return false;
    };

    return (1 == otme_.VerifyMessageSuccess(result));
}

void OTME_too::establish_mailability(
    const std::string& sender,
    const std::string& recipient,
    std::atomic<bool>* running,
    std::atomic<bool>* exitStatus)
{
    OT_ASSERT(nullptr != running)
    OT_ASSERT(nullptr != exitStatus)

    Cleanup threadStatus(*running);
    exitStatus->store(false);

    std::unique_lock<std::mutex> lock(messagability_lock_);
    const std::pair<std::string, std::string> key{sender, recipient};
    auto& runningMessagability = messagability_map_[key];
    bool previous = runningMessagability.load();

    if (previous) {
        otInfo << OT_METHOD << __FUNCTION__ << ": Thread already running."
               << std::endl;

        return;
    }

    Cleanup messagabilityStatus(runningMessagability);
    lock.unlock();
    auto recipientNym = wallet_.Nym(Identifier(recipient));

    if (!recipientNym) {
        otInfo << OT_METHOD << __FUNCTION__ << ": Searching for recipient nym."
               << std::endl;

        FindNym(recipient, "");

        return;
    }

    const auto claims = recipientNym->ContactData();

    if (!claims) {
        otInfo << OT_METHOD << __FUNCTION__ << ": Recipient nym has no claims."
               << std::endl;

        return;
    }

    auto server = extract_server(*claims);

    if (server.empty()) {
        otInfo << OT_METHOD << __FUNCTION__
               << ": Searching for server contract." << std::endl;

        FindServer(server);

        return;
    }

    const bool registered = exec_.IsNym_RegisteredAtServer(sender, server);

    bool exit = false;

    if (!registered) {
        otInfo << OT_METHOD << __FUNCTION__ << ": Registering on target server."
               << std::endl;

        exit = RegisterNym(sender, server, true);
    }

    exitStatus->store(exit);
}

std::uint64_t OTME_too::extract_assets(
    const proto::ContactData& claims,
    PairedNode& node)
{
    std::uint64_t output = 0;
    auto& unitMap = std::get<4>(node);

    for (const auto& section : claims.section()) {
        if (proto::CONTACTSECTION_CONTRACT == section.name()) {
            for (const auto& item : section.item()) {
                bool primary = false;
                bool active = false;

                for (const auto& attr : item.attribute()) {
                    if (proto::CITEMATTR_PRIMARY == attr) {
                        primary = true;
                    }

                    if (proto::CITEMATTR_ACTIVE == attr) {
                        active = true;
                    }
                }

                if (primary && active) {
                    std::unique_lock<std::mutex> lock(pair_lock_);
                    unitMap[item.type()] = item.value();
                    lock.unlock();
                    output++;
                } else {
                    OT_FAIL;
                }
            }
        }
    }

    return output;
}

std::string OTME_too::extract_server(const std::string& nymID) const
{
    const auto nym = wallet_.Nym(Identifier(nymID));

    if (!nym) {
        return "";
    }

    const auto claims = nym->ContactData();

    if (!claims) {
        return "";
    }

    return extract_server(*claims);
}

std::string OTME_too::extract_server(const proto::ContactData& claims) const
{
    std::string output;

    std::list<std::string> servers;
    const bool found = identity_.ExtractClaims(
        claims,
        proto::CONTACTSECTION_COMMUNICATION,
        proto::CITEMTYPE_OPENTXS,
        servers);

    if (found) {
        output = servers.front();
    }

    return output;
}

std::string OTME_too::extract_server_name(const std::string& serverNymID) const
{
    std::string output;

    const auto serverNym = wallet_.Nym(Identifier(serverNymID));

    if (!serverNym) {
        return output;
    }

    std::list<std::string> names;
    const bool found = identity_.ExtractClaims(
        *serverNym,
        proto::CONTACTSECTION_SCOPE,
        proto::CITEMTYPE_SERVER,
        names,
        true);

    if (found) {
        output = names.front();
    }

    return output;
}

std::set<std::string> OTME_too::extract_message_servers(
    const std::string& nymID) const
{
    std::list<std::string> output;

    const auto nym = wallet_.Nym(Identifier(nymID));

    if (!nym) {
        return {};
    }

    identity_.ExtractClaims(
        *nym,
        proto::CONTACTSECTION_COMMUNICATION,
        proto::CITEMTYPE_OPENTXS,
        output,
        false);

    return unique_servers(output);
}

void OTME_too::fill_existing_accounts(
    const std::string& nym,
    std::uint64_t& have,
    typeUnitMap& neededUnits,
    unitTypeMap& neededAccounts,
    PairedNode& node)
{
    const auto& notaryID = std::get<3>(node);
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
    const auto count = exec_.GetAccountCount();

    for (std::int32_t n = 0; n < count; n++) {
        const auto id = exec_.GetAccountWallet_ID(n);

        if (nym != exec_.GetAccountWallet_NymID(id)) {
            continue;
        }
        if (notaryID != exec_.GetAccountWallet_NotaryID(id)) {
            continue;
        }

        const auto unitDefinitionID =
            exec_.GetAccountWallet_InstrumentDefinitionID(id);
        auto it1 = neededUnits.find(unitDefinitionID);

        if (neededUnits.end() != it1) {
            auto it2 = neededAccounts.find(it1->second);
            auto& accountMap = std::get<5>(node);
            std::unique_lock<std::mutex> lock(pair_lock_);
            accountMap[it2->first] = id;
            lock.unlock();
            neededUnits.erase(it1);
            neededAccounts.erase(it2);
            have++;
        }
    }
}

void OTME_too::fill_paired_servers(
    std::set<std::string>& serverList,
    std::list<std::pair<std::string, std::string>>& serverNymList) const
{
    std::lock_guard<std::mutex> pairLock(pair_lock_);

    for (const auto& it : paired_nodes_) {
        const auto& node = it.second;
        const auto& localNymID = std::get<1>(node);
        const auto& server = std::get<3>(node);
        const auto& connected = std::get<7>(node);

        if (connected) {
            if (0 == serverList.count(server)) {
                serverNymList.push_back({server, localNymID});
                serverList.insert(server);
            }
        }
    }
}

void OTME_too::fill_registered_servers(
    std::string& introductionNym,
    std::set<std::string>& serverList,
    std::list<std::pair<std::string, std::string>>& serverNymList) const
{
    serverTaskMap accounts;
    build_account_list(accounts);
    const auto introductionServer = get_introduction_server();

    for (const auto server : accounts) {
        const auto& serverID = server.first;
        const auto& tasks = server.second;
        const auto& accountMap = tasks.first;
        const auto it = accountMap.begin();

        if (0 == accountMap.size()) {
            continue;
        }

        const auto& nymID = it->first;

        if (nymID.empty()) {
            continue;
        }

        if (serverID == introductionServer) {
            introductionNym = nymID;

            continue;
        }

        if (0 == serverList.count(serverID)) {
            serverNymList.push_back({serverID, nymID});
            serverList.insert(serverID);
        }
    }
}

void OTME_too::fill_viable_servers(
    std::list<std::pair<std::string, std::string>>& serverList) const
{
    std::set<std::string> servers;
    std::string introductionNym;
    const auto introductionServer = get_introduction_server();

    fill_paired_servers(servers, serverList);
    fill_registered_servers(introductionNym, servers, serverList);

    if ((!introductionServer.empty()) && (!introductionNym.empty())) {
        serverList.push_back({introductionServer, introductionNym});
    }
}

std::int64_t OTME_too::find_contact(const std::string& nymID, const Lock& lock)
    const
{
    OT_ASSERT(verify_lock(lock, contact_lock_));

    std::uint64_t output = -1;

    for (auto it : contact_map_) {
        const auto& index = it.first;
        const auto& meta = it.second;
        const auto& id = std::get<0>(meta);

        if (nymID == id) {
            output = index;

            return output;
        }
    }

    return output;
}

std::unique_ptr<OTME_too::PairedNode> OTME_too::find_node(
    const std::string& identifier) const
{
    std::string notUsed;

    return find_node(identifier, notUsed);
}

std::unique_ptr<OTME_too::PairedNode> OTME_too::find_node(
    const std::string& identifier,
    std::string& bridgeNymId) const
{
    std::unique_ptr<OTME_too::PairedNode> output;

    std::lock_guard<std::mutex> lock(pair_lock_);
    const auto it = paired_nodes_.find(identifier);

    if (paired_nodes_.end() != it) {
        // identifier was bridge nym ID
        output.reset(new OTME_too::PairedNode(it->second));
        bridgeNymId = identifier;

        return output;
    }

    for (const auto& it : paired_nodes_) {
        const auto& bridge = it.first;
        const auto& node = it.second;
        const auto& index = std::get<0>(node);
        const auto& server = std::get<3>(node);

        if ((server == identifier) || (std::to_string(index) == identifier)) {
            output.reset(new OTME_too::PairedNode(node));
            bridgeNymId = bridge;

            return output;
        }
    }

    bridgeNymId.clear();

    return output;
}

void OTME_too::find_nym(
    const std::string& remoteNymID,
    const std::string& serverIDhint,
    std::atomic<bool>* running,
    std::atomic<bool>* exitStatus) const
{
    OT_ASSERT(nullptr != running)
    OT_ASSERT(nullptr != exitStatus)

    Cleanup threadStatus(*running);
    exitStatus->store(false);
    std::list<std::pair<std::string, std::string>> serverList;
    fill_viable_servers(serverList);

    if (!serverIDhint.empty()) {
        for (auto it = serverList.begin(); it != serverList.end();) {
            const auto& serverID = it->first;
            const auto& nymID = it->second;

            if (serverIDhint == serverID) {

            } else {
                serverList.push_front({serverID, nymID});
                it = serverList.erase(it);
            }
        }
    }

    for (const auto& it : serverList) {
        const auto& serverID = it.first;
        const auto& nymID = it.second;

        const auto response = otme_.check_nym(serverID, nymID, remoteNymID);
        const bool found = (1 == otme_.VerifyMessageSuccess(response));

        if (found) {
            otErr << __FUNCTION__ << ": nym " << remoteNymID << " found on "
                  << serverID << "." << std::endl;
            exitStatus->store(true);

            break;
        }

        if (shutdown_.load()) {
            break;
        }
    }
}

void OTME_too::find_nym_if_necessary(const std::string& nymID, Identifier& task)
{
    auto nym = wallet_.Nym(Identifier(nymID));

    if (nym) {
        return;
    }

    const auto status = Status(task);

    switch (status) {
        case ThreadStatus::ERROR:
        case ThreadStatus::FINISHED_SUCCESS:
        case ThreadStatus::FINISHED_FAILED: {
            task = FindNym(nymID, "");
        } break;
        default: {
        }
    }
}

void OTME_too::find_server(
    const std::string& remoteServerID,
    std::atomic<bool>* running,
    std::atomic<bool>* exitStatus) const
{
    OT_ASSERT(nullptr != running)
    OT_ASSERT(nullptr != exitStatus)

    Cleanup threadStatus(*running);
    exitStatus->store(false);
    std::list<std::pair<std::string, std::string>> serverList;
    fill_viable_servers(serverList);

    for (const auto& it : serverList) {
        const auto& serverID = it.first;
        const auto& nymID = it.second;

        const auto response =
            otme_.retrieve_contract(serverID, nymID, remoteServerID);
        const bool found = (1 == otme_.VerifyMessageSuccess(response));

        if (found) {
            otErr << __FUNCTION__ << ": server " << remoteServerID
                  << " found on " << serverID << "." << std::endl;
            exitStatus->store(true);

            break;
        }

        if (shutdown_.load()) {
            break;
        }
    }
}

Identifier OTME_too::FindNym(
    const std::string& nymID,
    const std::string& serverHint)
{
    OTME_too::BackgroundThread thread =
        [=](std::atomic<bool>* running, std::atomic<bool>* exit) -> void {
        find_nym(nymID, serverHint, running, exit);
    };

    return add_background_thread(thread);
}

Identifier OTME_too::FindServer(const std::string& serverID)
{
    OTME_too::BackgroundThread thread =
        [=](std::atomic<bool>* running, std::atomic<bool>* exit) -> void {
        find_server(serverID, running, exit);
    };

    return add_background_thread(thread);
}

std::string OTME_too::get_introduction_server() const
{
    bool keyFound = false;
    String serverID;
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
    const bool config = config_.Check_str(
        MASTER_SECTION, INTRODUCTION_SERVER_KEY, serverID, keyFound);

    if (!config || !keyFound || !serverID.Exists()) {

        return import_default_introduction_server();
    }

    return serverID.Get();
}

std::time_t OTME_too::get_time(const std::string& alias) const
{
    std::time_t output = 0;

    try {
        output = std::stoi(alias);
    } catch (std::invalid_argument) {
        output = 0;
    } catch (std::out_of_range) {
        output = 0;
    }

    return output;
}

bool OTME_too::HaveContact(const std::string& nymID) const
{
    Lock lock(contact_lock_);

    return (-1 != find_contact(nymID, lock));
}

void OTME_too::import_contacts(const Lock& lock)
{
    OT_ASSERT(verify_lock(lock, contact_lock_));

    auto nyms = wallet_.NymList();

    for (const auto& it : nyms) {
        const auto& nymid = it.first;
        const auto nym = wallet_.Nym(Identifier(nymid));

        OT_ASSERT(nym);

        const auto nymType = identity_.NymType(*nym);

        switch (nymType) {
            case proto::CITEMTYPE_INDIVIDUAL:
            case proto::CITEMTYPE_ORGANIZATION:
            case proto::CITEMTYPE_BUSINESS:
            case proto::CITEMTYPE_GOVERNMENT: {
                add_update_contact(
                    lock, nymid, nym->PaymentCode(), nym->Alias());
            } break;
            case proto::CITEMTYPE_SERVER:
            default: {
            }
        }

        if (!yield()) {
            return;
        };
    }
}

std::string OTME_too::import_default_introduction_server() const
{
    return SetIntroductionServer(OTME_too::DEFAULT_INTRODUCTION_SERVER);
}

bool OTME_too::insert_at_index(
    const std::int64_t index,
    const std::int64_t total,
    const std::string& myNym,
    const std::string& bridgeNym,
    std::string& password) const
{
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
    bool dontCare = false;

    if (!config_.Set_long(MASTER_SECTION, PAIRED_NODES_KEY, total, dontCare)) {

        return false;
    }

    String section = PAIRED_SECTION_PREFIX;
    String key = std::to_string(index).c_str();
    section.Concatenate(key);

    if (!config_.Set_str(
            section, BRIDGE_NYM_KEY, String(bridgeNym), dontCare)) {

        return false;
    }

    if (!config_.Set_str(
            section, ADMIN_PASSWORD_KEY, String(password), dontCare)) {

        return false;
    }

    String pw;

    if (!config_.Check_str(section, ADMIN_PASSWORD_KEY, pw, dontCare)) {

        return false;
    }

    password = pw.Get();

    if (!config_.Set_str(section, OWNER_NYM_KEY, String(myNym), dontCare)) {

        return false;
    }

    return config_.Save();
}

std::string OTME_too::GetPairedServer(const std::string& identifier) const
{
    std::string output;

    const auto node = find_node(identifier);

    if (node) {
        const auto& notaryID = std::get<3>(*node);
        output = notaryID;
    }

    return output;
}

void OTME_too::mailability(
    const std::string& sender,
    const std::string& recipient)
{
    OTME_too::BackgroundThread thread =
        [=](std::atomic<bool>* running, std::atomic<bool>* exit) -> void {
        establish_mailability(sender, recipient, running, exit);
    };

    add_background_thread(thread);
}

void OTME_too::mark_connected(PairedNode& node)
{
    auto& connected = std::get<7>(node);
    bool dontCare = false;
    String section = PAIRED_SECTION_PREFIX;
    String sectionKey = std::to_string(std::get<0>(node)).c_str();
    section.Concatenate(sectionKey);
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
    connected = true;
    config_.Set_bool(section, CONNECTED_KEY, connected, dontCare);
    config_.Save();
}

void OTME_too::mark_finished(const std::string& bridgeNymID)
{
    std::unique_lock<std::mutex> lock(pair_lock_);
    auto& node = paired_nodes_[bridgeNymID];
    lock.unlock();
    auto& done = std::get<9>(node);
    bool dontCare = false;
    String section = PAIRED_SECTION_PREFIX;
    String sectionKey = std::to_string(std::get<0>(node)).c_str();
    section.Concatenate(sectionKey);
    done = true;
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
    config_.Set_bool(section, DONE_KEY, done, dontCare);
    config_.Save();
}

void OTME_too::mark_renamed(const std::string& bridgeNymID)
{
    std::unique_lock<std::mutex> lock(pair_lock_);
    auto& node = paired_nodes_[bridgeNymID];
    lock.unlock();
    auto& renamed = std::get<8>(node);
    bool dontCare = false;
    String section = PAIRED_SECTION_PREFIX;
    String sectionKey = std::to_string(std::get<0>(node)).c_str();
    section.Concatenate(sectionKey);
    renamed = true;
    std::unique_lock<std::recursive_mutex> apiLock(api_lock_);
    config_.Set_bool(section, RENAME_KEY, renamed, dontCare);
    config_.Save();
    apiLock.unlock();
    yield();
}

void OTME_too::message_contact(
    const std::string& server,
    const std::string& sender,
    const std::string& contact,
    const std::string& message,
    std::atomic<bool>* running,
    std::atomic<bool>* exitStatus)
{
    OT_ASSERT(nullptr != running)
    OT_ASSERT(nullptr != exitStatus)

    Cleanup threadStatus(*running);
    exitStatus->store(false);
    const auto result = otme_.send_user_msg(server, sender, contact, message);
    const bool success = (1 == otme_.VerifyMessageSuccess(result));
    exitStatus->store(success);
}

Identifier OTME_too::MessageContact(
    const std::string& sender,
    const std::string& contact,
    const std::string& message)
{
    std::string server;
    const auto messagability = can_message(sender, contact, server);

    if (Messagability::READY != messagability) {
        otWarn << OT_METHOD << __FUNCTION__ << ": Unable to message ("
               << std::to_string(static_cast<std::int8_t>(messagability)) << ")"
               << std::endl;

        return {};
    }

    OTME_too::BackgroundThread thread =
        [=](std::atomic<bool>* running, std::atomic<bool>* exit) -> void {
        message_contact(server, sender, contact, message, running, exit);
    };

    return add_background_thread(thread);
}

bool OTME_too::need_to_refresh(const std::string& serverID)
{
    Lock lock(refresh_interval_lock_);

    auto it = refresh_interval_.find(serverID);

    if (refresh_interval_.end() == it) {
        // This server has not been configured for reduced updates

        return true;
    }

    auto& interval = it->second;

    if (0 == interval) {
        // A value of zero means update every loop

        return true;
    }

    return (0 == (refresh_count_.load() % interval));
}

bool OTME_too::NodeRenamed(const std::string& identifier) const
{
    const auto node = find_node(identifier);

    if (node) {
        const auto& renamed = std::get<8>(*node);

        return renamed;
    }

    return false;
}

std::string OTME_too::obtain_account(
    const std::string& nym,
    const std::string& id,
    const std::string& server) const
{
    const std::string result = otme_.create_asset_acct(server, nym, id);

    if (1 != OTAPI_Wrap::Message_GetSuccess(result)) {
        return "";
    }

    return OTAPI_Wrap::Message_GetNewAcctID(result);
}

bool OTME_too::obtain_asset_contract(
    const std::string& nym,
    const std::string& id,
    const std::string& server) const
{
    std::string contract;
    bool retry = false;

    while (true) {
        contract = exec_.GetAssetType_Contract(id);

        if (!yield()) {
            return false;
        };

        if (!contract.empty() || retry) {
            break;
        }

        retry = true;

        otme_.retrieve_contract(server, nym, id);

        if (!yield()) {
            return false;
        }
    }

    return !contract.empty();
}

bool OTME_too::obtain_assets(
    const std::string& bridgeNym,
    const proto::ContactData& claims,
    PairedNode& node)
{
    const auto& owner = std::get<1>(node);
    const auto& notaryID = std::get<3>(node);

    std::uint64_t assets = 0;
    bool retry = false;

    while (true) {
        assets = extract_assets(claims, node);

        if (0 < assets) {
            break;
        }

        if (retry) {
            otErr << __FUNCTION__
                  << ": Bridge nym does not advertise instrument definitions."
                  << std::endl;
            break;
        }

        retry = true;

        if (!download_nym(owner, bridgeNym, notaryID)) {
            otErr << __FUNCTION__ << ": Unable to download nym." << std::endl;
            break;
        }
    }

    return 0 < assets;
}

std::unique_ptr<proto::ContactData> OTME_too::obtain_contact_data(
    const std::string& localNym,
    const Nym& remoteNym,
    const std::string& server) const
{
    std::unique_ptr<proto::ContactData> output;
    bool retry = false;

    while (true) {
        output.reset(identity_.Claims(remoteNym).release());

        if (output) {
            break;
        }

        if (retry) {
            otErr << __FUNCTION__ << ": Nym has no contact data." << std::endl;
            break;
        }

        retry = true;

        if (!download_nym(localNym, String(remoteNym.ID()).Get(), server)) {
            otErr << __FUNCTION__ << ": Unable to download nym." << std::endl;
            break;
        }
    }

    return output;
}

std::shared_ptr<const Nym> OTME_too::obtain_nym(
    const std::string& localNym,
    const std::string& remoteNym,
    const std::string& server) const
{
    std::shared_ptr<const Nym> output;
    bool retry = false;

    while (true) {
        output = wallet_.Nym(Identifier(remoteNym));

        if (output || retry) {
            break;
        }

        retry = true;

        if (!download_nym(localNym, remoteNym, server)) {
            otErr << __FUNCTION__ << ": Unable to download nym." << std::endl;

            break;
        }
    }

    return output;
}

bool OTME_too::obtain_server_contract(
    const std::string& nym,
    const std::string& server,
    const bool publish) const
{
    std::string contract;
    bool retry = false;

    while (true) {
        contract = exec_.GetServer_Contract(server);

        if (!yield()) {
            return false;
        };

        if (!contract.empty() || retry) {
            break;
        }

        retry = true;

        otme_.retrieve_contract(get_introduction_server(), nym, server);

        if (!yield()) {
            return false;
        }
    }

    if (!contract.empty()) {

        return RegisterNym(nym, server, publish);
    }

    return false;
}

std::string OTME_too::obtain_server_id(
    const std::string& ownerNym,
    const std::string& bridgeNym,
    const proto::ContactData& claims) const
{
    std::string output;
    output = extract_server(claims);

    if (output.empty()) {
        download_nym(ownerNym, bridgeNym, "");
    }

    return output;
}

std::string OTME_too::obtain_server_id(const std::string& nymID) const
{
    std::string output;

    auto nym = wallet_.Nym(Identifier(nymID));

    if (!nym) {
        return output;
    }

    auto data = nym->ContactData();

    if (!data) {
        return output;
    }

    output = extract_server(*data);

    return output;
}

void OTME_too::pair(const std::string& bridgeNymID)
{
    std::unique_lock<std::mutex> lock(pair_lock_);
    auto& node = paired_nodes_[bridgeNymID];
    lock.unlock();
    const auto& ownerNym = std::get<1>(node);

    if (!check_introduction_server(ownerNym)) {
        return;
    }

    if (!check_bridge_nym(bridgeNymID, node)) {
        return;
    }

    const bool backup = check_backup(bridgeNymID, node);

    if (!yield()) {
        return;
    }

    const bool accounts = check_accounts(node);
    const bool saved = update_accounts(node);

    if (!yield()) {
        return;
    }

    if (backup && accounts && saved) {
        const auto& notary = std::get<3>(node);
        publish_server_registration(ownerNym, notary, true);
        request_connection(
            ownerNym, notary, bridgeNymID, proto::CONNECTIONINFO_BTCRPC);
        mark_connected(node);

        if (!yield()) {
            return;
        }
    }

    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
    config_.Save();
}

std::uint64_t OTME_too::PairedNodeCount() const
{
    std::int64_t result = 0;
    bool notUsed = false;
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
    config_.Check_long(MASTER_SECTION, PAIRED_NODES_KEY, result, notUsed);

    if (1 > result) {
        return 0;
    }

    return result;
}

void OTME_too::pairing_thread()
{
    std::unique_lock<std::mutex> lock(pair_lock_);
    std::list<std::string> unfinished;

    Cleanup cleanup(pairing_);

    for (const auto& it : paired_nodes_) {
        const auto& node = it.second;
        const auto& connected = std::get<7>(node);

        if (!connected) {
            unfinished.push_back(it.first);
        }
    }

    lock.unlock();

    if (!yield()) {
        return;
    };

    for (const auto& bridgeNymID : unfinished) {
        pair(bridgeNymID);

        if (!yield()) {
            return;
        }
    }

    unfinished.clear();
    check_server_names();
}

bool OTME_too::PairingComplete(const std::string& identifier) const
{
    const auto node = find_node(identifier);

    if (node) {
        const auto& done = std::get<9>(*node);

        return done;
    }

    return false;
}

bool OTME_too::PairingStarted(const std::string& identifier) const
{
    const auto node = find_node(identifier);

    return bool(node);
}

std::string OTME_too::PairingStatus(const std::string& identifier) const
{
    std::stringstream output{};
    std::string bridgeNymID;
    const auto node = find_node(identifier, bridgeNymID);

    if (false == bool(node)) {
        output << "Pairing to " << identifier << " has not started."
               << std::endl;

        return output.str();
    }

    const auto& myNym = std::get<1>(*node);
    const auto& password = std::get<2>(*node);
    const auto& notaryID = std::get<3>(*node);
    const auto& unitmap = std::get<4>(*node);
    const auto& accountmap = std::get<5>(*node);
    const auto& backupStarted = std::get<6>(*node);
    const auto& connected = std::get<7>(*node);
    const auto& renameStarted = std::get<8>(*node);
    const auto& done = std::get<9>(*node);

    output << "Stash Node\n"
           << "Issuer nym ID: " << bridgeNymID << "\n"
           << "Server password: " << password << "\n";

    const auto intro = get_introduction_server();

    if (intro.empty()) {
        output << "The wallet does not yet have an introduction server set."
               << "\n";

        return output.str();
    }

    if (false == exec_.IsNym_RegisteredAtServer(myNym, intro)) {
        output << "The wallet is not yet registered on the introduction server."
               << "\n";

        return output.str();
    }

    const auto nym = wallet_.Nym(Identifier(bridgeNymID));

    if (false == bool(nym)) {
        output << "The credentials for the issuer nym are not yet downloaded."
               << "\n";

        return output.str();
    }

    output << "Notary ID: " << notaryID << "\n"
           << "Contracts issued on this node:\n";

    for (const auto& it : unitmap) {
        const auto& type = it.first;
        const auto& unitID = it.second;
        const auto typeName = identity_.ContactTypeName(type);
        output << "* " << typeName << " unit defininition ID: " << unitID
               << "\n";
    }

    if (false == exec_.IsNym_RegisteredAtServer(myNym, notaryID)) {
        output << "The wallet is not yet registerd on the Stash Node notary."
               << "\n";

        return output.str();
    }

    output << "Local accounts registered on this node:\n";

    for (const auto& it : accountmap) {
        const auto& type = it.first;
        const auto& accountID = it.second;
        const auto typeName = identity_.ContactTypeName(type);
        output << "* " << typeName << " account ID: " << accountID << "\n";
    }

    output << "Pairing is ";

    if (connected) {
        output << " successful.\n";
    } else {
        output << " in-progress.\n";

        return output.str();
    }

    output << "Wallet seed backup process has ";

    if (false == backupStarted) {
        output << "not ";
    }

    output << "started.\n";
    output << "Server rename process has ";

    if (false == renameStarted) {
        output << "not ";
    }

    output << "started.\n";

    if (done) {
        output << "Pairing is complete.\n";
    }

    return output.str();
}

bool OTME_too::PairingSuccessful(const std::string& identifier) const
{
    const auto node = find_node(identifier);

    if (node) {
        const auto& connected = std::get<7>(*node);

        return connected;
    }

    return false;
}

bool OTME_too::PairNode(
    const std::string& myNym,
    const std::string& bridgeNym,
    const std::string& password)
{
    if (myNym.empty()) {
        otErr << __FUNCTION__ << ": missing nym." << std::endl;

        return false;
    }

    if (bridgeNym.empty()) {
        otErr << __FUNCTION__ << ": missing bridge nym." << std::endl;

        return false;
    }

    if (password.empty()) {
        otErr << __FUNCTION__ << ": missing password." << std::endl;

        return false;
    }

    auto pw = CryptoEncodingEngine::SanatizeBase58(password);

    std::unique_lock<std::mutex> startLock(pair_initiate_lock_);
    const bool alreadyPairing = check_pairing(bridgeNym, pw);

    if (alreadyPairing) {
        return true;
    }

    startLock.unlock();
    std::unique_lock<std::mutex> lock(pair_lock_);
    auto total = PairedNodeCount();

    if (!yield()) {
        return false;
    }

    auto index = scan_incomplete_pairing(bridgeNym);

    if (0 > index) {
        index = total;
        total++;
    }

    const bool saved = insert_at_index(index, total, myNym, bridgeNym, pw);

    if (!yield()) {
        return false;
    }

    if (!saved) {
        otErr << __FUNCTION__ << ": Failed to update config file." << std::endl;

        return false;
    }

    auto& node = paired_nodes_[bridgeNym];
    auto& nodeIndex = std::get<0>(node);
    auto& owner = std::get<1>(node);
    auto& serverPassword = std::get<2>(node);

    otErr << OT_METHOD << __FUNCTION__ << ": Pairing started for " << bridgeNym
          << std::endl;

    nodeIndex = index;
    owner = myNym;
    serverPassword = pw;

    lock.unlock();
    UpdatePairing();

    return true;
}

void OTME_too::parse_contact_section(
    const Lock& lock,
    const std::uint64_t index)
{
    verify_lock(lock, contact_lock_);
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
    bool notUsed = false;
    String nymID;
    String name;
    String paymentCode;
    std::int64_t checked = 0;
    std::int64_t version = 0;
    String section = CONTACT_SECTION_PREFIX;
    String key = std::to_string(index).c_str();
    section.Concatenate(key);
    config_.Check_str(section, CONTACT_NYMID_KEY, nymID, notUsed);
    config_.Check_long(section, CONTACT_UPDATED_KEY, checked, notUsed);
    config_.Check_long(section, CONTACT_REVISION_KEY, version, notUsed);
    config_.Check_str(section, CONTACT_LABEL_KEY, name, notUsed);
    config_.Check_str(section, CONTACT_PAYMENTCODE_KEY, paymentCode, notUsed);
    const bool ready = nymID.Exists();

    if (!ready) {
        return;
    }

    if (0 > version) {
        version = 0;
    }

    auto& contactMetadata = contact_map_[index];
    auto& mapNymID = std::get<0>(contactMetadata);
    auto& revision = std::get<1>(contactMetadata);
    auto& checkedTime = std::get<2>(contactMetadata);
    auto& label = std::get<4>(contactMetadata);
    auto& code = std::get<5>(contactMetadata);

    mapNymID = nymID.Get();
    revision = version;
    checkedTime = static_cast<std::time_t>(checked);
    label = name.Get();
    code = paymentCode.Get();
}

void OTME_too::parse_pairing_section(std::uint64_t index)
{
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
    bool notUsed = false;
    String bridgeNym, adminPassword, ownerNym;
    String section = PAIRED_SECTION_PREFIX;
    String key = std::to_string(index).c_str();
    section.Concatenate(key);
    config_.Check_str(section, BRIDGE_NYM_KEY, bridgeNym, notUsed);
    config_.Check_str(section, ADMIN_PASSWORD_KEY, adminPassword, notUsed);
    config_.Check_str(section, OWNER_NYM_KEY, ownerNym, notUsed);
    const bool ready =
        (bridgeNym.Exists() && adminPassword.Exists() && ownerNym.Exists());

    if (!ready) {
        return;
    }

    auto& node = paired_nodes_[bridgeNym.Get()];
    auto& nodeIndex = std::get<0>(node);
    auto& owner = std::get<1>(node);
    auto& password = std::get<2>(node);
    auto& notaryID = std::get<3>(node);
    auto& unitMap = std::get<4>(node);
    auto& accountMap = std::get<5>(node);
    auto& backup = std::get<6>(node);
    auto& connected = std::get<7>(node);
    auto& rename = std::get<8>(node);
    auto& done = std::get<9>(node);

    nodeIndex = index;
    owner = ownerNym.Get();
    password = adminPassword.Get();

    config_.Check_bool(section, BACKUP_KEY, backup, notUsed);
    config_.Check_bool(section, CONNECTED_KEY, connected, notUsed);
    config_.Check_bool(section, RENAME_KEY, rename, notUsed);
    config_.Check_bool(section, DONE_KEY, done, notUsed);

    String notary;
    config_.Check_str(section, NOTARY_ID_KEY, notary, notUsed);
    notaryID = notary.Get();

    std::int64_t issued = 0;
    config_.Check_long(section, ISSUED_UNITS_KEY, issued, notUsed);

    for (std::int64_t n = 0; n < issued; n++) {
        bool exists = false;
        std::int64_t type;
        String key = ISSUED_UNIT_PREFIX_KEY;
        String index = std::to_string(n).c_str();
        key.Concatenate(index);
        const bool checked = config_.Check_long(section, key, type, exists);
        const auto unit = validate_unit(type);
        const bool valid =
            checked && exists && (proto::CITEMTYPE_ERROR != unit);

        if (valid) {
            String contract, account;
            String unitKey = ASSET_ID_PREFIX_KEY;
            String accountKey = ACCOUNT_ID_PREFIX_KEY;
            String unitIndex = std::to_string(type).c_str();
            unitKey.Concatenate(unitIndex);
            accountKey.Concatenate(unitIndex);
            config_.Check_str(section, unitKey, contract, notUsed);
            config_.Check_str(section, accountKey, account, notUsed);
            unitMap[unit] = contract.Get();
            accountMap[unit] = account.Get();
        }
    }
}

bool OTME_too::publish_server_registration(
    const std::string& nymID,
    const std::string& server,
    const bool forcePrimary) const
{
    std::unique_lock<std::recursive_mutex> apiLock(api_lock_);
    auto nym = ot_api_.GetOrLoadPrivateNym(Identifier(nymID), false);

    OT_ASSERT(nullptr != nym);

    std::string claimID;
    const bool alreadyExists = identity_.ClaimExists(
        *nym,
        proto::CONTACTSECTION_COMMUNICATION,
        proto::CITEMTYPE_OPENTXS,
        server,
        claimID);

    if (alreadyExists) {
        return true;
    }

    bool setPrimary = false;

    if (forcePrimary) {
        setPrimary = true;
    } else {
        std::string primary;
        const bool hasPrimary = identity_.HasPrimary(
            *nym,
            proto::CONTACTSECTION_COMMUNICATION,
            proto::CITEMTYPE_OPENTXS,
            primary);
        setPrimary = !hasPrimary;
    }

    std::set<std::uint32_t> attribute;
    attribute.insert(proto::CITEMATTR_ACTIVE);

    if (setPrimary) {
        attribute.insert(proto::CITEMATTR_PRIMARY);
    }

    const Claim input{"",
                      proto::CONTACTSECTION_COMMUNICATION,
                      proto::CITEMTYPE_OPENTXS,
                      server,
                      0,
                      0,
                      attribute};

    const bool claimIsSet = identity_.AddClaim(*nym, input);
    apiLock.unlock();
    yield();

    return claimIsSet;
}

void OTME_too::refresh_contacts(nymAccountMap& nymsToCheck)
{
    Lock lock(contact_lock_);

    for (auto& contact : contact_map_) {
        auto& meta = contact.second;
        const auto& nymID = std::get<0>(meta);
        auto& revision = std::get<1>(meta);
        auto& checked = std::get<2>(meta);
        const auto now = std::time(nullptr);
        const std::chrono::seconds interval(now - checked);
        const std::chrono::hours limit(24 * CONTACT_REFRESH_DAYS);
        const bool haveNym = (0 < revision);

        if (!haveNym) {
            const auto nym = wallet_.Nym(Identifier(nymID));

            if (nym) {
                revision = nym->Revision();
                checked = now;
                write_contact_data(lock);
            } else {
                nymsToCheck[ALL_SERVERS].push_back(nymID);
            }
        } else {
            if (interval > limit) {
                const auto servers = extract_message_servers(nymID);

                for (const auto& server : servers) {
                    nymsToCheck[server].push_back(nymID);
                }
            }
        }
    }
}

void OTME_too::refresh_thread()
{
    Cleanup cleanup(refreshing_);
    serverTaskMap accounts;
    build_account_list(accounts);
    nymAccountMap nymsToCheck;
    refresh_contacts(nymsToCheck);
    add_checknym_tasks(nymsToCheck, accounts);

    for (const auto server : accounts) {
        bool updateServerNym = true;
        const auto& serverID = server.first;
        const auto& tasks = server.second;
        const auto& accountList = tasks.first;
        const auto& checkNym = tasks.second;
        bool nymsChecked = false;

        if (false == need_to_refresh(serverID)) {
            otInfo << OT_METHOD << __FUNCTION__
                   << ": Skipping update for server " << serverID << std::endl;

            continue;
        } else {
            otInfo << OT_METHOD << __FUNCTION__ << ": Updating server "
                   << serverID << std::endl;
        }

        for (const auto nym : accountList) {
            if (!yield()) {
                return;
            }

            const auto& nymID = nym.first;

            if (updateServerNym) {
                auto contract = wallet_.Server(Identifier(serverID));

                if (contract) {
                    const auto& serverNymID = contract->Nym()->ID();
                    const auto result = otme_.check_nym(
                        serverID, nymID, String(serverNymID).Get());
                    // If multiple nyms are registered on this server, we only
                    // need to successfully download the nym once.
                    updateServerNym =
                        !(1 == otme_.VerifyMessageSuccess(result));

                    if (false == updateServerNym) {
                        otme_.register_nym(serverID, nymID);
                    }
                }
            }

            bool notUsed = false;
            made_easy_.retrieve_nym(serverID, nymID, notUsed, true);

            // If the nym's credentials have been updated since the last time
            // it was registered on the server, upload the new credentials
            if (!check_nym_revision(nymID, serverID)) {
                check_server_registration(nymID, serverID, true, false);
            }

            for (auto& account : nym.second) {
                if (!yield()) {
                    return;
                }

                made_easy_.retrieve_account(serverID, nymID, account, true);
            }

            if (!nymsChecked) {
                for (const auto& nym : checkNym) {
                    made_easy_.check_nym(serverID, nymID, nym);

                    if (!yield()) {
                        return;
                    }
                }

                nymsChecked = true;
            }
        }
    }

    refresh_count_++;
    UpdatePairing();
    resend_peer_requests();
    refreshing_.store(false);
}

void OTME_too::Refresh(const std::string&)
{
    const auto refreshing = refreshing_.exchange(true);

    if (!refreshing) {
        if (refresh_thread_) {
            refresh_thread_->join();
            refresh_thread_.reset();
        }

        refresh_thread_.reset(new std::thread(&OTME_too::refresh_thread, this));
    }
}

bool OTME_too::RegisterNym(
    const std::string& nymID,
    const std::string& server,
    const bool setContactData) const
{
    const auto result = otme_.register_nym(server, nymID);

    if (!yield()) {
        return false;
    }

    const bool registered = (1 == otme_.VerifyMessageSuccess(result));

    if (!yield()) {
        return false;
    }

    if (registered) {
        if (setContactData) {

            return publish_server_registration(nymID, server, false);
        } else {

            return true;
        }
    }

    return false;
}

std::uint64_t OTME_too::RefreshCount() const { return refresh_count_.load(); }

bool OTME_too::RenameContact(
    const std::string& contactNymID,
    const std::string& name)
{
    Lock lock(contact_lock_);

    wallet_.SetNymAlias(Identifier(contactNymID), name);

    return add_update_contact(lock, contactNymID, "", name);
}

bool OTME_too::request_connection(
    const std::string& nym,
    const std::string& server,
    const std::string& bridgeNymID,
    const std::int64_t type) const
{
    const auto result =
        otme_.request_connection(server, nym, bridgeNymID, type);

    return exec_.Message_GetSuccess(result);
}

bool OTME_too::RequestConnection(
    const std::string& nym,
    const std::string& node,
    const std::int64_t type) const
{
    std::string bridgeNymID;
    auto index = find_node(node, bridgeNymID);

    if (!index) {
        return false;
    }

    const auto& server = std::get<3>(*index);

    OT_ASSERT(!bridgeNymID.empty());

    return request_connection(nym, server, bridgeNymID, type);
}

void OTME_too::resend_bailment(
    const Identifier& nymID,
    const proto::PeerRequest& request) const
{
    const auto result = otme_.initiate_bailment(
        request.bailment().serverid(),
        request.initiator(),
        request.recipient(),
        request.bailment().unitid());

    if (1 == exec_.Message_GetSuccess(result)) {
        wallet_.PeerRequestDelete(
            nymID, Identifier(request.id()), StorageBox::SENTPEERREQUEST);
    }
}

void OTME_too::resend_bailment_notification(
    const Identifier& nymID,
    const proto::PeerRequest& request) const
{
    const auto result = otme_.notify_bailment(
        request.pendingbailment().serverid(),
        request.initiator(),
        request.recipient(),
        request.pendingbailment().unitid(),
        request.pendingbailment().txid());

    if (1 == exec_.Message_GetSuccess(result)) {
        wallet_.PeerRequestDelete(
            nymID, Identifier(request.id()), StorageBox::SENTPEERREQUEST);
    }
}

void OTME_too::resend_connection_info(
    const Identifier& nymID,
    const proto::PeerRequest& request) const
{
    auto server = request.server();

    if (server.empty()) {
        otErr << __FUNCTION__ << ": This request was saved without the server "
              << "id. Attempting lookup based on recipient nym." << std::endl;

        server = obtain_server_id(request.recipient());
    }

    const auto result = otme_.request_connection(
        server,
        request.initiator(),
        request.recipient(),
        request.connectioninfo().type());

    if (1 == exec_.Message_GetSuccess(result)) {
        wallet_.PeerRequestDelete(
            nymID, Identifier(request.id()), StorageBox::SENTPEERREQUEST);
    }
}

void OTME_too::resend_outbailment(
    const Identifier& nymID,
    const proto::PeerRequest& request) const
{
    const auto result = otme_.initiate_outbailment(
        request.outbailment().serverid(),
        request.initiator(),
        request.recipient(),
        request.outbailment().unitid(),
        request.outbailment().amount(),
        request.outbailment().instructions());

    if (1 == exec_.Message_GetSuccess(result)) {
        wallet_.PeerRequestDelete(
            nymID, Identifier(request.id()), StorageBox::SENTPEERREQUEST);
    }
}

void OTME_too::resend_store_secret(
    const Identifier& nymID,
    const proto::PeerRequest& request) const
{
    auto server = request.server();

    if (server.empty()) {
        otErr << __FUNCTION__ << ": This request was saved without the server "
              << "id. Attempting lookup based on recipient nym." << std::endl;

        server = obtain_server_id(request.recipient());
    }

    const auto result = otme_.store_secret(
        server,
        request.initiator(),
        request.recipient(),
        request.storesecret().type(),
        request.storesecret().primary(),
        request.storesecret().secondary());

    if (1 == exec_.Message_GetSuccess(result)) {
        wallet_.PeerRequestDelete(
            nymID, Identifier(request.id()), StorageBox::SENTPEERREQUEST);
    }
}

void OTME_too::resend_peer_request(
    const Identifier& nymID,
    const Identifier& requestID) const
{
    std::unique_lock<std::recursive_mutex> apiLock(api_lock_);
    std::time_t notUsed;

    auto request = wallet_.PeerRequest(
        nymID, requestID, StorageBox::SENTPEERREQUEST, notUsed);

    if (!request) {
        return;
    }

    switch (request->type()) {
        case proto::PEERREQUEST_BAILMENT: {
            resend_bailment(nymID, *request);
        } break;
        case proto::PEERREQUEST_OUTBAILMENT: {
            resend_outbailment(nymID, *request);
        } break;
        case proto::PEERREQUEST_PENDINGBAILMENT: {
            resend_bailment_notification(nymID, *request);
        } break;
        case proto::PEERREQUEST_CONNECTIONINFO: {
            resend_connection_info(nymID, *request);
        } break;
        case proto::PEERREQUEST_STORESECRET: {
            resend_store_secret(nymID, *request);
        } break;
        default: {
        }
    }

    apiLock.unlock();
}

void OTME_too::resend_peer_requests() const
{
    std::list<std::string> nyms;
    build_nym_list(nyms);
    const std::string empty = std::to_string(std::time_t(0));
    const std::time_t now = std::time(nullptr);
    const std::chrono::hours limit(24);

    for (const auto& nym : nyms) {
        const Identifier nymID(nym);
        const auto sent = wallet_.PeerRequestSent(nymID);

        for (const auto& request : sent) {
            const auto& id = request.first;
            const auto& alias = request.second;

            if (alias == empty) {
                otErr << __FUNCTION__ << ": Timestamp for peer request " << id
                      << " is not set. Setting now." << std::endl;

                wallet_.PeerRequestUpdate(
                    nymID, Identifier(id), StorageBox::SENTPEERREQUEST);
            } else {
                std::chrono::seconds interval(now - get_time(alias));

                if (interval > limit) {
                    otInfo << __FUNCTION__ << ": request " << id << " is "
                           << interval.count() << " seconds old. Resending."
                           << std::endl;
                    resend_peer_request(nymID, Identifier(id));
                } else {
                    otInfo << __FUNCTION__ << ": request " << id << " is "
                           << interval.count() << " seconds old. It's fine."
                           << std::endl;
                }
            }
        }
    }
}

bool OTME_too::send_backup(const std::string& bridgeNymID, PairedNode& node)
    const
{
    auto& notaryID = std::get<3>(node);
    auto& owner = std::get<1>(node);
    const auto result = otme_.store_secret(
        notaryID,
        owner,
        bridgeNymID,
        proto::SECRETTYPE_BIP39,
        exec_.Wallet_GetWords(),
        exec_.Wallet_GetPassphrase());
    yield();

    return 1 == otme_.VerifyMessageSuccess(result);
}

bool OTME_too::send_server_name(
    const std::string& nym,
    const std::string& server,
    const std::string& password,
    const std::string& name) const
{
    otErr << OT_METHOD << __FUNCTION__ << ": Renaming server " << server
          << " to " << server << std::endl;

    otme_.request_admin(server, nym, password);

    const auto result = otme_.server_add_claim(
        server,
        nym,
        std::to_string(proto::CONTACTSECTION_SCOPE),
        std::to_string(proto::CITEMTYPE_SERVER),
        name,
        true);

    return (1 == otme_.VerifyMessageSuccess(result));
}

void OTME_too::set_server_names(const ServerNameData& servers)
{
    for (const auto server : servers) {
        const auto& notaryID = server.first;
        const auto& myNymID = std::get<0>(server.second);
        const auto& bridgeNymID = std::get<1>(server.second);
        const auto& password = std::get<2>(server.second);
        const auto contract = wallet_.Server(Identifier(notaryID));

        if (!contract) {
            continue;
        }

        const std::string localName = exec_.GetServer_Name(notaryID);

        if (!yield()) {
            return;
        }

        const auto serialized = contract->Contract();
        const auto& originalName = serialized.name();
        const auto& serverNymID = serialized.nymid();

        if (localName == originalName) {
            // Never attempted to rename this server. Nothing to do
            continue;
        }

        bool retry = false;
        bool done = false;

        while (true) {
            const auto credentialName = extract_server_name(serverNymID);

            if (localName == credentialName) {
                // Server was renamed, and has published new credentials.
                mark_finished(bridgeNymID);

                if (!yield()) {
                    return;
                }

                done = true;
            }

            if (done || retry) {
                break;
            }

            otErr << OT_METHOD << __FUNCTION__ << ": Notary " << notaryID
                  << " has been locally renamed to " << localName
                  << " but still advertises a name of " << credentialName
                  << " in its credentials." << std::endl;

            retry = true;

            otErr << OT_METHOD << __FUNCTION__
                  << ": Downloading a new copy of the server nym credentials."
                  << std::endl;

            // Perhaps our copy of the server nym credentials is out of date
            download_nym(myNymID, serverNymID, notaryID);
        }

        if (done) {
            continue;
        }

        otErr << OT_METHOD << __FUNCTION__
              << ": Instructing notary to update server nym credentials."
              << std::endl;

        if (send_server_name(myNymID, notaryID, password, localName)) {
            mark_renamed(bridgeNymID);
        }
    }
}

std::string OTME_too::SetIntroductionServer(const std::string& contract) const
{
    std::string id = exec_.AddServerContract(contract);

    if (!yield()) {
        return {};
    }

    if (!id.empty()) {
        bool dontCare = false;
        std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
        const bool set = config_.Set_str(
            MASTER_SECTION, INTRODUCTION_SERVER_KEY, String(id), dontCare);

        if (!set) {
            id.clear();
        } else {
            config_.Save();
        }
    }

    return id;
}

void OTME_too::SetInterval(
    const std::string& server,
    const std::uint64_t interval) const
{
    Lock lock(refresh_interval_lock_);

    refresh_interval_[server] = interval;
}

void OTME_too::scan_contacts()
{
    Lock lock(contact_lock_);

    for (std::uint64_t n = 0; n < ContactCount(); n++) {

        if (!yield()) {
            return;
        }

        parse_contact_section(lock, n);
    }

    if (!yield()) {
        return;
    }

    import_contacts(lock);

    if (!yield()) {
        return;
    }
}

std::int64_t OTME_too::scan_incomplete_pairing(const std::string& bridgeNym)
{
    std::int64_t index = -1;

    for (std::uint64_t n = 0; n < PairedNodeCount(); n++) {

        if (!yield()) {
            return index;
        }

        bool notUsed = false;
        String existing;
        String section = PAIRED_SECTION_PREFIX;
        const String key = std::to_string(n).c_str();
        section.Concatenate(key);
        std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
        config_.Check_str(section, BRIDGE_NYM_KEY, existing, notUsed);
        const std::string compareNym(existing.Get());

        if (compareNym == bridgeNym) {
            index = n;

            break;
        }
    }

    return index;
}

void OTME_too::scan_pairing()
{
    std::lock_guard<std::mutex> lock(pair_lock_);

    for (std::uint64_t n = 0; n < PairedNodeCount(); n++) {

        if (!yield()) {
            return;
        }

        parse_pairing_section(n);
    }

    yield();
}

void OTME_too::Shutdown()
{
    clean_background_threads();

    while (refreshing_.load()) {
        Log::Sleep(std::chrono::milliseconds(250));
    }

    while (pairing_.load()) {
        Log::Sleep(std::chrono::milliseconds(250));
    }

    if (refresh_thread_) {
        refresh_thread_->join();
        refresh_thread_.reset();
    }

    if (pairing_thread_) {
        pairing_thread_->join();
        pairing_thread_.reset();
    }
}

ThreadStatus OTME_too::Status(const Identifier& thread)
{
    if (shutdown_.load()) {
        return ThreadStatus::SHUTDOWN;
    }

    std::unique_lock<std::mutex> lock(thread_lock_);

    auto it = threads_.find(thread);

    if (threads_.end() == it) {
        return ThreadStatus::ERROR;
    }

    const auto& status = std::get<0>(it->second);
    const bool exit_status = std::get<2>(it->second);

    if (status.load()) {
        return ThreadStatus::RUNNING;
    }

    const auto& handle = std::get<1>(it->second);

    OT_ASSERT(handle);

    if (handle->joinable()) {
        handle->join();
    }

    threads_.erase(it);

    if (exit_status) {

        return ThreadStatus::FINISHED_SUCCESS;
    }

    return ThreadStatus::FINISHED_FAILED;
}

std::set<std::string> OTME_too::unique_servers(
    const std::list<std::string>& input) const
{
    std::set<std::string> output;

    for (const auto& it : input) {
        output.insert(it);
    }

    return output;
}

bool OTME_too::update_accounts(const PairedNode& node)
{
    bool dontCare = false;
    String section = PAIRED_SECTION_PREFIX;
    String sectionKey = std::to_string(std::get<0>(node)).c_str();
    section.Concatenate(sectionKey);
    const auto& accountMap = std::get<5>(node);
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);

    for (const auto account : accountMap) {
        const auto& type = account.first;
        const auto& id = account.second;
        String accountKey = ACCOUNT_ID_PREFIX_KEY;
        String index = std::to_string(type).c_str();
        accountKey.Concatenate(index);

        if (!config_.Set_str(section, accountKey, String(id), dontCare)) {

            return false;
        }
    }

    return config_.Save();
}

bool OTME_too::update_assets(PairedNode& node)
{
    bool dontCare = false;
    String section = PAIRED_SECTION_PREFIX;
    String sectionKey = std::to_string(std::get<0>(node)).c_str();
    section.Concatenate(sectionKey);
    auto& unitMap = std::get<4>(node);
    const auto count = unitMap.size();
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);

    if (!config_.Set_long(section, ISSUED_UNITS_KEY, count, dontCare)) {

        return false;
    }

    std::int64_t index = 0;

    for (const auto unit : unitMap) {
        const auto& type = unit.first;
        const auto& id = unit.second;
        String assetKey = ASSET_ID_PREFIX_KEY;
        String assetIndex = std::to_string(type).c_str();
        assetKey.Concatenate(assetIndex);
        String unitKey = ISSUED_UNIT_PREFIX_KEY;
        String unitIndex = std::to_string(index).c_str();
        unitKey.Concatenate(unitIndex);

        if (!config_.Set_long(section, unitKey, type, dontCare)) {

            return false;
        }

        if (!config_.Set_str(section, assetKey, String(id), dontCare)) {

            return false;
        }

        index++;
    }

    return config_.Save();
}

bool OTME_too::update_notary(const std::string& id, PairedNode& node)
{
    bool dontCare = false;
    String section = PAIRED_SECTION_PREFIX;
    String key = std::to_string(std::get<0>(node)).c_str();
    section.Concatenate(key);
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);

    const bool set =
        config_.Set_str(section, NOTARY_ID_KEY, String(id), dontCare);

    if (!set) {
        return false;
    }

    auto& notary = std::get<3>(node);
    notary = id;

    return config_.Save();
}

bool OTME_too::update_nym_revision(
    const std::string& nymID,
    const std::string& server) const
{
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);

    auto nym = wallet_.Nym(Identifier(nymID));

    if (!nym) {
        return false;
    }

    bool dontCare = false;
    String section = NYM_REVISION_SECTION_PREFIX;
    section.Concatenate(String(nymID));
    const String key(server);
    const auto& revision = nym->Revision();

    if (!config_.Set_long(section, key, revision, dontCare)) {

        return false;
    }

    return config_.Save();
}

void OTME_too::UpdatePairing(const std::string&)
{
    const auto pairing = pairing_.exchange(true);

    if (!pairing) {
        if (pairing_thread_) {
            pairing_thread_->join();
            pairing_thread_.reset();
        }

        pairing_thread_.reset(new std::thread(&OTME_too::pairing_thread, this));
    }
}

proto::ContactItemType OTME_too::validate_unit(const std::int64_t type)
{
    proto::ContactItemType unit = static_cast<proto::ContactItemType>(type);

    switch (unit) {
        case proto::CITEMTYPE_BTC:
        case proto::CITEMTYPE_ETH:
        case proto::CITEMTYPE_XRP:
        case proto::CITEMTYPE_LTC:
        case proto::CITEMTYPE_DAO:
        case proto::CITEMTYPE_XEM:
        case proto::CITEMTYPE_DASH:
        case proto::CITEMTYPE_MAID:
        case proto::CITEMTYPE_LSK:
        case proto::CITEMTYPE_DOGE:
        case proto::CITEMTYPE_DGD:
        case proto::CITEMTYPE_XMR:
        case proto::CITEMTYPE_WAVES:
        case proto::CITEMTYPE_NXT:
        case proto::CITEMTYPE_SC:
        case proto::CITEMTYPE_STEEM:
        case proto::CITEMTYPE_AMP:
        case proto::CITEMTYPE_XLM:
        case proto::CITEMTYPE_FCT:
        case proto::CITEMTYPE_BTS:
        case proto::CITEMTYPE_USD:
        case proto::CITEMTYPE_EUR:
        case proto::CITEMTYPE_GBP:
        case proto::CITEMTYPE_INR:
        case proto::CITEMTYPE_AUD:
        case proto::CITEMTYPE_CAD:
        case proto::CITEMTYPE_SGD:
        case proto::CITEMTYPE_CHF:
        case proto::CITEMTYPE_MYR:
        case proto::CITEMTYPE_JPY:
        case proto::CITEMTYPE_CNY:
        case proto::CITEMTYPE_NZD:
        case proto::CITEMTYPE_THB:
        case proto::CITEMTYPE_HUF:
        case proto::CITEMTYPE_AED:
        case proto::CITEMTYPE_HKD:
        case proto::CITEMTYPE_MXN:
        case proto::CITEMTYPE_ZAR:
        case proto::CITEMTYPE_PHP:
        case proto::CITEMTYPE_SEK: {
        } break;
        default: {
            return proto::CITEMTYPE_ERROR;
        }
    }

    return unit;
}

bool OTME_too::verify_lock(const Lock& lock, const std::mutex& mutex) const
{
    if (lock.mutex() != &mutex) {
        otErr << __FUNCTION__ << ": Incorrect mutex." << std::endl;

        return false;
    }

    if (false == lock.owns_lock()) {
        otErr << __FUNCTION__ << ": Lock not owned." << std::endl;

        return false;
    }

    return true;
}

bool OTME_too::write_contact_data()
{
    Lock lock(contact_lock_);

    return write_contact_data(lock);
}

bool OTME_too::write_contact_data(const Lock& lock)
{
    OT_ASSERT(verify_lock(lock, contact_lock_));

    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
    bool dontCare = false;

    if (!config_.Set_long(
            MASTER_SECTION, CONTACT_COUNT_KEY, contact_map_.size(), dontCare)) {

        return false;
    }

    bool output = true;
    std::uint64_t index = 0;

    for (const auto& contact : contact_map_) {
        const auto& meta = contact.second;
        const auto& nymID = std::get<0>(meta);
        const auto& revision = std::get<1>(meta);
        const auto& checked = std::get<2>(meta);
        const auto& label = std::get<4>(meta);
        const auto& code = std::get<5>(meta);
        String section = CONTACT_SECTION_PREFIX;
        String key = std::to_string(index).c_str();
        section.Concatenate(key);

        output &= config_.Set_str(
            section, CONTACT_NYMID_KEY, String(nymID), dontCare);
        output &=
            config_.Set_long(section, CONTACT_UPDATED_KEY, checked, dontCare);
        output &=
            config_.Set_long(section, CONTACT_REVISION_KEY, revision, dontCare);
        output &= config_.Set_str(
            section, CONTACT_LABEL_KEY, label.c_str(), dontCare);
        output &= config_.Set_str(
            section, CONTACT_PAYMENTCODE_KEY, code.c_str(), dontCare);

        index++;
    }

    config_.Save();

    return output;
}

bool OTME_too::yield() const
{
    Log::Sleep(std::chrono::milliseconds(50));

    return !shutdown_.load();
}

OTME_too::~OTME_too() { Shutdown(); }
}  // namespace opentxs

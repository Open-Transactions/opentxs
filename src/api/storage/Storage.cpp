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

#include "stdafx.hpp"

#include "opentxs/api/Editor.hpp"
#include "opentxs/core/Flag.hpp"

#include "storage/drivers/StorageMultiplex.hpp"
#include "storage/tree/Accounts.hpp"
#include "storage/tree/Bip47Context.hpp"
#include "storage/tree/Bip47Contexts.hpp"
#include "storage/tree/BlockchainTransactions.hpp"
#include "storage/tree/Contacts.hpp"
#include "storage/tree/Contexts.hpp"
#include "storage/tree/Credentials.hpp"
#include "storage/tree/Issuers.hpp"
#include "storage/tree/Mailbox.hpp"
#include "storage/tree/Nym.hpp"
#include "storage/tree/Nyms.hpp"
#include "storage/tree/PaymentWorkflows.hpp"
#include "storage/tree/PeerReplies.hpp"
#include "storage/tree/PeerRequests.hpp"
#include "storage/tree/Root.hpp"
#include "storage/tree/Seeds.hpp"
#include "storage/tree/Servers.hpp"
#include "storage/tree/Thread.hpp"
#include "storage/tree/Threads.hpp"
#include "storage/tree/Tree.hpp"
#include "storage/tree/Units.hpp"
#include "storage/StorageConfig.hpp"
#include "Factory.hpp"
#include "StorageInternal.hpp"

#include <cassert>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include "Storage.hpp"

#define OT_METHOD "opentxs::api::storage::implementation::Storage::"

namespace opentxs
{
api::storage::StorageInternal* Factory::Storage(
    const Flag& running,
    const StorageConfig& config,
    const String& primary,
    const bool migrate,
    const String& previous,
    const Digest& hash,
    const Random& random)
{
    return new api::storage::implementation::Storage(
        running, config, primary, migrate, previous, hash, random);
}
}  // namespace opentxs

namespace opentxs::api::storage::implementation
{
const std::uint32_t Storage::HASH_TYPE = 2;  // BTC160

Storage::Storage(
    const Flag& running,
    const StorageConfig& config,
    const String& primary,
    const bool migrate,
    const String& previous,
    const Digest& hash,
    const Random& random)
    : running_(running)
    , gc_interval_(config.gc_interval_)
    , write_lock_()
    , root_(nullptr)
    , primary_bucket_(Flag::Factory(false))
    , background_threads_()
    , config_(config)
    , multiplex_p_(new StorageMultiplex(
          *this,
          primary_bucket_,
          config_,
          primary,
          migrate,
          previous,
          hash,
          random))
    , multiplex_(*multiplex_p_)
{
    OT_ASSERT(multiplex_p_);
}

ObjectList Storage::AccountList() const
{
    return Root().Tree().AccountNode().List();
}

OTIdentifier Storage::AccountContract(const Identifier& accountID) const
{
    return Root().Tree().AccountNode().AccountContract(accountID);
}

OTIdentifier Storage::AccountIssuer(const Identifier& accountID) const
{
    return Root().Tree().AccountNode().AccountIssuer(accountID);
}

OTIdentifier Storage::AccountOwner(const Identifier& accountID) const
{
    return Root().Tree().AccountNode().AccountOwner(accountID);
}

OTIdentifier Storage::AccountServer(const Identifier& accountID) const
{
    return Root().Tree().AccountNode().AccountServer(accountID);
}

OTIdentifier Storage::AccountSigner(const Identifier& accountID) const
{
    return Root().Tree().AccountNode().AccountSigner(accountID);
}

proto::ContactItemType Storage::AccountUnit(const Identifier& accountID) const
{
    return Root().Tree().AccountNode().AccountUnit(accountID);
}

std::set<OTIdentifier> Storage::AccountsByContract(
    const Identifier& contract) const
{
    return Root().Tree().AccountNode().AccountsByContract(contract);
}

std::set<OTIdentifier> Storage::AccountsByIssuer(
    const Identifier& issuerNym) const
{
    return Root().Tree().AccountNode().AccountsByIssuer(issuerNym);
}

std::set<OTIdentifier> Storage::AccountsByOwner(
    const Identifier& ownerNym) const
{
    return Root().Tree().AccountNode().AccountsByOwner(ownerNym);
}

std::set<OTIdentifier> Storage::AccountsByServer(const Identifier& server) const
{
    return Root().Tree().AccountNode().AccountsByServer(server);
}

std::set<OTIdentifier> Storage::AccountsByUnit(
    const proto::ContactItemType unit) const
{
    return Root().Tree().AccountNode().AccountsByUnit(unit);
}

std::set<proto::ContactItemType> Storage::Bip47ChainList(
    const std::string& nymID) const
{
    if (false == Root().Tree().NymNode().Exists(nymID)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID
              << " doesn't exist." << std::endl;

        return {};
    }

    return Root().Tree().NymNode().Nym(nymID).Bip47ChainList();
}

std::set<Bip47ChannelID> Storage::Bip47ChannelList(
    const std::string& nymID,
    const std::string& contactID,
    const proto::ContactItemType chain) const
{
    if (false == Root().Tree().NymNode().Exists(nymID)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID
              << " doesn't exist." << std::endl;

        return {};
    }

    return Root().Tree().NymNode().Nym(nymID).Bip47ChannelList(
        contactID, chain);
}

std::set<std::string> Storage::Bip47ContactList(
    const std::string& nymID,
    const proto::ContactItemType chain) const
{
    if (false == Root().Tree().NymNode().Exists(nymID)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID
              << " doesn't exist." << std::endl;

        return {};
    }

    return Root().Tree().NymNode().Nym(nymID).Bip47ContactList(chain);
}

std::set<std::string> Storage::BlockchainAccountList(
    const std::string& nymID,
    const proto::ContactItemType type) const
{
    return Root().Tree().NymNode().Nym(nymID).BlockchainAccountList(type);
}

std::string Storage::BlockchainAddressOwner(
    proto::ContactItemType chain,
    std::string address) const
{
    return Root().Tree().ContactNode().AddressOwner(chain, address);
}

ObjectList Storage::BlockchainTransactionList() const
{

    return Root().Tree().BlockchainNode().List();
}

void Storage::Cleanup_Storage()
{
    for (auto& thread : background_threads_) {
        if (thread.joinable()) { thread.join(); }
    }

    if (root_) { root_->cleanup(); }
}

void Storage::Cleanup() { Cleanup_Storage(); }

void Storage::CollectGarbage() const { Root().Migrate(multiplex_.Primary()); }

std::string Storage::ContactAlias(const std::string& id) const
{
    return Root().Tree().ContactNode().Alias(id);
}

ObjectList Storage::ContactList() const
{
    return Root().Tree().ContactNode().List();
}

std::string Storage::ContactOwnerNym(const std::string& nymID) const
{
    return Root().Tree().ContactNode().NymOwner(nymID);
}

void Storage::ContactSaveIndices() const
{
    mutable_Root().It().mutable_Tree().It().mutable_Contacts().It().Save();
}

std::uint32_t Storage::ContactUpgradeLevel() const
{
    return Root().Tree().ContactNode().UpgradeLevel();
}

ObjectList Storage::ContextList(const std::string& nymID) const
{
    return Root().Tree().NymNode().Nym(nymID).Contexts().List();
}

bool Storage::CreateThread(
    const std::string& nymID,
    const std::string& threadID,
    const std::set<std::string>& participants) const
{
    const auto id = mutable_Root()
                        .It()
                        .mutable_Tree()
                        .It()
                        .mutable_Nyms()
                        .It()
                        .mutable_Nym(nymID)
                        .It()
                        .mutable_Threads()
                        .It()
                        .Create(threadID, participants);

    return (false == id.empty());
}

std::string Storage::DefaultSeed() const
{
    return Root().Tree().SeedNode().Default();
}

bool Storage::DeleteAccount(const std::string& id) const
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Accounts()
        .It()
        .Delete(id);
}

bool Storage::DeleteContact(const std::string& id) const
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Contacts()
        .It()
        .Delete(id);
}

bool Storage::DeletePaymentWorkflow(
    const std::string& nymID,
    const std::string& workflowID) const
{
    const bool exists = Root().Tree().NymNode().Exists(nymID);

    if (false == exists) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID
              << " doesn't exist." << std::endl;

        return false;
    }

    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Nyms()
        .It()
        .mutable_Nym(nymID)
        .It()
        .mutable_PaymentWorkflows()
        .It()
        .Delete(workflowID);
}

std::uint32_t Storage::HashType() const { return HASH_TYPE; }

void Storage::InitBackup() { multiplex_.InitBackup(); }

void Storage::InitEncryptedBackup(std::unique_ptr<SymmetricKey>& key)
{
    multiplex_.InitEncryptedBackup(key);
}

void Storage::InitPlugins()
{
    bool syncPrimary{false};
    const auto hash = multiplex_.best_root(syncPrimary);

    if (hash.empty()) { return; }

    std::unique_ptr<opentxs::storage::Root> root{new opentxs::storage::Root(
        multiplex_,
        hash,
        std::numeric_limits<std::int64_t>::max(),
        primary_bucket_)};

    OT_ASSERT(root);

    multiplex_.synchronize_plugins(hash, *root, syncPrimary);
}

ObjectList Storage::IssuerList(const std::string& nymID) const
{
    const bool exists = Root().Tree().NymNode().Exists(nymID);

    if (false == exists) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID
              << " doesn't exist." << std::endl;

        return {};
    }

    return Root().Tree().NymNode().Nym(nymID).Issuers().List();
}

bool Storage::Load(
    const std::string& accountID,
    std::string& output,
    std::string& alias,
    const bool checking) const
{
    return Root().Tree().AccountNode().Load(accountID, output, alias, checking);
}

bool Storage::Load(
    const std::string& nymID,
    const std::string& accountID,
    std::shared_ptr<proto::Bip44Account>& output,
    const bool checking) const
{
    return Root().Tree().NymNode().Nym(nymID).Load(accountID, output, checking);
}

bool Storage::Load(
    const std::string& nymID,
    const std::string& paymentCode,
    std::shared_ptr<proto::Bip47Context>& context,
    const bool checking) const
{
    const bool exists =
        Root().Tree().NymNode().Nym(nymID).Bip47Contexts().Exists(paymentCode);

    if (!exists) { return false; }

    context.reset(new proto::Bip47Context);

    if (!context) { return false; }

    *context = Root()
                   .Tree()
                   .NymNode()
                   .Nym(nymID)
                   .Bip47Contexts()
                   .Bip47Context(paymentCode)
                   .Items();

    return bool(context);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::BlockchainTransaction>& transaction,
    const bool checking) const
{
    return Root().Tree().BlockchainNode().Load(id, transaction, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::Contact>& contact,
    const bool checking) const
{
    std::string notUsed{};

    return Load(id, contact, notUsed, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::Contact>& contact,
    std::string& alias,
    const bool checking) const
{
    return Root().Tree().ContactNode().Load(id, contact, alias, checking);
}

bool Storage::Load(
    const std::string& nym,
    const std::string& id,
    std::shared_ptr<proto::Context>& context,
    const bool checking) const
{
    std::string notUsed;

    return Root().Tree().NymNode().Nym(nym).Contexts().Load(
        id, context, notUsed, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::Credential>& cred,
    const bool checking) const
{
    return Root().Tree().CredentialNode().Load(id, cred, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::CredentialIndex>& nym,
    const bool checking) const
{
    std::string notUsed;

    return Load(id, nym, notUsed, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::CredentialIndex>& nym,
    std::string& alias,
    const bool checking) const
{
    return Root().Tree().NymNode().Nym(id).Load(nym, alias, checking);
}

bool Storage::Load(
    const std::string& nymID,
    const std::string& id,
    std::shared_ptr<proto::Issuer>& issuer,
    const bool checking) const
{
    if (false == Root().Tree().NymNode().Exists(nymID)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID
              << " doesn't exist." << std::endl;

        return false;
    }

    std::string notUsed{""};

    return Root().Tree().NymNode().Nym(nymID).Issuers().Load(
        id, issuer, notUsed, checking);
}

bool Storage::Load(
    const std::string& nymID,
    const std::string& workflowID,
    std::shared_ptr<proto::PaymentWorkflow>& workflow,
    const bool checking) const
{
    if (false == Root().Tree().NymNode().Exists(nymID)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID
              << " doesn't exist." << std::endl;

        return false;
    }

    return Root().Tree().NymNode().Nym(nymID).PaymentWorkflows().Load(
        workflowID, workflow, checking);
}

bool Storage::Load(
    const std::string& nymID,
    const std::string& id,
    const StorageBox box,
    std::string& output,
    std::string& alias,
    const bool checking) const
{
    switch (box) {
        case StorageBox::MAILINBOX: {
            return Root().Tree().NymNode().Nym(nymID).MailInbox().Load(
                id, output, alias, checking);
        }
        case StorageBox::MAILOUTBOX: {
            return Root().Tree().NymNode().Nym(nymID).MailOutbox().Load(
                id, output, alias, checking);
        }
        default: {
            return false;
        }
    }
}

bool Storage::Load(
    const std::string& nymID,
    const std::string& id,
    const StorageBox box,
    std::shared_ptr<proto::PeerReply>& reply,
    const bool checking) const
{
    switch (box) {
        case StorageBox::SENTPEERREPLY: {
            return Root().Tree().NymNode().Nym(nymID).SentReplyBox().Load(
                id, reply, checking);
        } break;
        case StorageBox::INCOMINGPEERREPLY: {
            return Root().Tree().NymNode().Nym(nymID).IncomingReplyBox().Load(
                id, reply, checking);
        } break;
        case StorageBox::FINISHEDPEERREPLY: {
            return Root().Tree().NymNode().Nym(nymID).FinishedReplyBox().Load(
                id, reply, checking);
        } break;
        case StorageBox::PROCESSEDPEERREPLY: {
            return Root().Tree().NymNode().Nym(nymID).ProcessedReplyBox().Load(
                id, reply, checking);
        } break;
        default: {
            return false;
        }
    }
}

bool Storage::Load(
    const std::string& nymID,
    const std::string& id,
    const StorageBox box,
    std::shared_ptr<proto::PeerRequest>& request,
    std::time_t& time,
    const bool checking) const
{
    bool output = false;
    std::string alias;

    switch (box) {
        case StorageBox::SENTPEERREQUEST: {
            output = Root().Tree().NymNode().Nym(nymID).SentRequestBox().Load(
                id, request, alias, checking);
        } break;
        case StorageBox::INCOMINGPEERREQUEST: {
            output =
                Root().Tree().NymNode().Nym(nymID).IncomingRequestBox().Load(
                    id, request, alias, checking);
        } break;
        case StorageBox::FINISHEDPEERREQUEST: {
            output =
                Root().Tree().NymNode().Nym(nymID).FinishedRequestBox().Load(
                    id, request, alias, checking);
        } break;
        case StorageBox::PROCESSEDPEERREQUEST: {
            output =
                Root().Tree().NymNode().Nym(nymID).ProcessedRequestBox().Load(
                    id, request, alias, checking);
        } break;
        default: {
        }
    }

    if (output) {
        try {
            time = std::stoi(alias);
        } catch (const std::invalid_argument&) {
            time = 0;
        } catch (const std::out_of_range&) {
            time = 0;
        }
    }

    return output;
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::Seed>& seed,
    const bool checking) const
{
    std::string notUsed;

    return Load(id, seed, notUsed, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::Seed>& seed,
    std::string& alias,
    const bool checking) const
{
    return Root().Tree().SeedNode().Load(id, seed, alias, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::ServerContract>& contract,
    const bool checking) const
{
    std::string notUsed;

    return Load(id, contract, notUsed, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::ServerContract>& contract,
    std::string& alias,
    const bool checking) const
{
    return Root().Tree().ServerNode().Load(id, contract, alias, checking);
}

bool Storage::Load(
    const std::string& nymId,
    const std::string& threadId,
    std::shared_ptr<proto::StorageThread>& thread) const
{
    const bool exists =
        Root().Tree().NymNode().Nym(nymId).Threads().Exists(threadId);

    if (!exists) { return false; }

    thread.reset(new proto::StorageThread);

    if (!thread) { return false; }

    *thread =
        Root().Tree().NymNode().Nym(nymId).Threads().Thread(threadId).Items();

    return bool(thread);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::UnitDefinition>& contract,
    const bool checking) const
{
    std::string notUsed;

    return Load(id, contract, notUsed, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::UnitDefinition>& contract,
    std::string& alias,
    const bool checking) const
{
    return Root().Tree().UnitNode().Load(id, contract, alias, checking);
}

const std::set<std::string> Storage::LocalNyms() const
{
    return Root().Tree().NymNode().LocalNyms();
}

// Applies a lambda to all public nyms in the database in a detached thread.
void Storage::MapPublicNyms(NymLambda& lambda) const
{
    std::thread bgMap(&Storage::RunMapPublicNyms, this, lambda);
    bgMap.detach();
}

// Applies a lambda to all server contracts in the database in a detached
// thread.
void Storage::MapServers(ServerLambda& lambda) const
{
    std::thread bgMap(&Storage::RunMapServers, this, lambda);
    bgMap.detach();
}

// Applies a lambda to all unit definitions in the database in a detached
// thread.
void Storage::MapUnitDefinitions(UnitLambda& lambda) const
{
    std::thread bgMap(&Storage::RunMapUnits, this, lambda);
    bgMap.detach();
}

opentxs::storage::Root* Storage::root() const
{
    Lock lock(write_lock_);

    if (!root_) {
        root_.reset(new opentxs::storage::Root(
            multiplex_, multiplex_.LoadRoot(), gc_interval_, primary_bucket_));
    }

    OT_ASSERT(root_);

    lock.unlock();

    return root_.get();
}

const opentxs::storage::Root& Storage::Root() const { return *root(); }

bool Storage::MoveThreadItem(
    const std::string& nymId,
    const std::string& fromThreadID,
    const std::string& toThreadID,
    const std::string& itemID) const
{
    const bool fromExists =
        Root().Tree().NymNode().Nym(nymId).Threads().Exists(fromThreadID);

    if (false == fromExists) {
        otErr << OT_METHOD << __FUNCTION__ << ": From thread does not exist."
              << std::endl;

        return false;
    }

    const bool toExists =
        Root().Tree().NymNode().Nym(nymId).Threads().Exists(toThreadID);

    if (false == toExists) {
        otErr << OT_METHOD << __FUNCTION__ << ": To thread does not exist."
              << std::endl;

        return false;
    }

    auto& fromThread = mutable_Root()
                           .It()
                           .mutable_Tree()
                           .It()
                           .mutable_Nyms()
                           .It()
                           .mutable_Nym(nymId)
                           .It()
                           .mutable_Threads()
                           .It()
                           .mutable_Thread(fromThreadID)
                           .It();
    const auto thread = fromThread.Items();
    bool found = false;
    std::uint64_t time{};
    StorageBox box{};

    for (const auto& item : thread.item()) {
        if (item.id() == itemID) {
            found = true;
            time = item.time();
            box = static_cast<StorageBox>(item.box());

            break;
        }
    }

    if (false == found) {
        otErr << OT_METHOD << __FUNCTION__ << ": Item does not exist."
              << std::endl;

        return false;
    }

    if (false == fromThread.Remove(itemID)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to remove item."
              << std::endl;

        return false;
    }

    auto& toThread = mutable_Root()
                         .It()
                         .mutable_Tree()
                         .It()
                         .mutable_Nyms()
                         .It()
                         .mutable_Nym(nymId)
                         .It()
                         .mutable_Threads()
                         .It()
                         .mutable_Thread(toThreadID)
                         .It();

    if (false == toThread.Add(itemID, time, box, {}, {})) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to insert item."
              << std::endl;

        return false;
    }

    return true;
}

Editor<opentxs::storage::Root> Storage::mutable_Root() const
{
    std::function<void(opentxs::storage::Root*, Lock&)> callback =
        [&](opentxs::storage::Root* in, Lock& lock) -> void {
        this->save(in, lock);
    };

    return Editor<opentxs::storage::Root>(write_lock_, root(), callback);
}

ObjectList Storage::NymBoxList(const std::string& nymID, const StorageBox box)
    const
{
    switch (box) {
        case StorageBox::SENTPEERREQUEST: {
            return Root().Tree().NymNode().Nym(nymID).SentRequestBox().List();
        } break;
        case StorageBox::INCOMINGPEERREQUEST: {
            return Root()
                .Tree()
                .NymNode()
                .Nym(nymID)
                .IncomingRequestBox()
                .List();
        } break;
        case StorageBox::SENTPEERREPLY: {
            return Root().Tree().NymNode().Nym(nymID).SentReplyBox().List();
        } break;
        case StorageBox::INCOMINGPEERREPLY: {
            return Root().Tree().NymNode().Nym(nymID).IncomingReplyBox().List();
        } break;
        case StorageBox::FINISHEDPEERREQUEST: {
            return Root()
                .Tree()
                .NymNode()
                .Nym(nymID)
                .FinishedRequestBox()
                .List();
        } break;
        case StorageBox::FINISHEDPEERREPLY: {
            return Root().Tree().NymNode().Nym(nymID).FinishedReplyBox().List();
        } break;
        case StorageBox::PROCESSEDPEERREQUEST: {
            return Root()
                .Tree()
                .NymNode()
                .Nym(nymID)
                .ProcessedRequestBox()
                .List();
        } break;
        case StorageBox::PROCESSEDPEERREPLY: {
            return Root()
                .Tree()
                .NymNode()
                .Nym(nymID)
                .ProcessedReplyBox()
                .List();
        } break;
        case StorageBox::MAILINBOX: {
            return Root().Tree().NymNode().Nym(nymID).MailInbox().List();
        }
        case StorageBox::MAILOUTBOX: {
            return Root().Tree().NymNode().Nym(nymID).MailOutbox().List();
        }
        default: {
            return {};
        }
    }
}

ObjectList Storage::NymList() const { return Root().Tree().NymNode().List(); }

ObjectList Storage::PaymentWorkflowList(const std::string& nymID) const
{
    if (false == Root().Tree().NymNode().Exists(nymID)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID
              << " doesn't exist." << std::endl;

        return {};
    }

    return Root().Tree().NymNode().Nym(nymID).PaymentWorkflows().List();
}

std::string Storage::PaymentWorkflowLookup(
    const std::string& nymID,
    const std::string& sourceID) const
{
    if (false == Root().Tree().NymNode().Exists(nymID)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID
              << " doesn't exist." << std::endl;

        return {};
    }

    return Root().Tree().NymNode().Nym(nymID).PaymentWorkflows().LookupBySource(
        sourceID);
}

std::set<std::string> Storage::PaymentWorkflowsByAccount(
    const std::string& nymID,
    const std::string& accountID) const
{
    if (false == Root().Tree().NymNode().Exists(nymID)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID
              << " doesn't exist." << std::endl;

        return {};
    }

    return Root().Tree().NymNode().Nym(nymID).PaymentWorkflows().ListByAccount(
        accountID);
}

std::set<std::string> Storage::PaymentWorkflowsByState(
    const std::string& nymID,
    const proto::PaymentWorkflowType type,
    const proto::PaymentWorkflowState state) const
{
    if (false == Root().Tree().NymNode().Exists(nymID)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID
              << " doesn't exist." << std::endl;

        return {};
    }

    return Root().Tree().NymNode().Nym(nymID).PaymentWorkflows().ListByState(
        type, state);
}

std::set<std::string> Storage::PaymentWorkflowsByUnit(
    const std::string& nymID,
    const std::string& unitID) const
{
    if (false == Root().Tree().NymNode().Exists(nymID)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID
              << " doesn't exist." << std::endl;

        return {};
    }

    return Root().Tree().NymNode().Nym(nymID).PaymentWorkflows().ListByUnit(
        unitID);
}

std::pair<proto::PaymentWorkflowType, proto::PaymentWorkflowState> Storage::
    PaymentWorkflowState(
        const std::string& nymID,
        const std::string& workflowID) const
{
    if (false == Root().Tree().NymNode().Exists(nymID)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID
              << " doesn't exist." << std::endl;

        return {};
    }

    return Root().Tree().NymNode().Nym(nymID).PaymentWorkflows().GetState(
        workflowID);
}

bool Storage::RelabelThread(
    const std::string& threadID,
    const std::string& label) const
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Nyms()
        .It()
        .RelabelThread(threadID, label);
}

bool Storage::RemoveNymBoxItem(
    const std::string& nymID,
    const StorageBox box,
    const std::string& itemID) const
{
    switch (box) {
        case StorageBox::SENTPEERREQUEST: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_SentRequestBox()
                .It()
                .Delete(itemID);
        } break;
        case StorageBox::INCOMINGPEERREQUEST: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_IncomingRequestBox()
                .It()
                .Delete(itemID);
        } break;
        case StorageBox::SENTPEERREPLY: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_SentReplyBox()
                .It()
                .Delete(itemID);
        } break;
        case StorageBox::INCOMINGPEERREPLY: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_IncomingReplyBox()
                .It()
                .Delete(itemID);
        } break;
        case StorageBox::FINISHEDPEERREQUEST: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_FinishedRequestBox()
                .It()
                .Delete(itemID);
        } break;
        case StorageBox::FINISHEDPEERREPLY: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_FinishedReplyBox()
                .It()
                .Delete(itemID);
        } break;
        case StorageBox::PROCESSEDPEERREQUEST: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_ProcessedRequestBox()
                .It()
                .Delete(itemID);
        } break;
        case StorageBox::PROCESSEDPEERREPLY: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_ProcessedReplyBox()
                .It()
                .Delete(itemID);
        } break;
        case StorageBox::MAILINBOX: {
            const bool foundInThread = mutable_Root()
                                           .It()
                                           .mutable_Tree()
                                           .It()
                                           .mutable_Nyms()
                                           .It()
                                           .mutable_Nym(nymID)
                                           .It()
                                           .mutable_Threads()
                                           .It()
                                           .FindAndDeleteItem(itemID);
            bool foundInBox = false;

            if (!foundInThread) {
                foundInBox = mutable_Root()
                                 .It()
                                 .mutable_Tree()
                                 .It()
                                 .mutable_Nyms()
                                 .It()
                                 .mutable_Nym(nymID)
                                 .It()
                                 .mutable_MailInbox()
                                 .It()
                                 .Delete(itemID);
            }

            return foundInThread || foundInBox;
        }
        case StorageBox::MAILOUTBOX: {
            const bool foundInThread = mutable_Root()
                                           .It()
                                           .mutable_Tree()
                                           .It()
                                           .mutable_Nyms()
                                           .It()
                                           .mutable_Nym(nymID)
                                           .It()
                                           .mutable_Threads()
                                           .It()
                                           .FindAndDeleteItem(itemID);
            bool foundInBox = false;

            if (!foundInThread) {
                foundInBox = mutable_Root()
                                 .It()
                                 .mutable_Tree()
                                 .It()
                                 .mutable_Nyms()
                                 .It()
                                 .mutable_Nym(nymID)
                                 .It()
                                 .mutable_MailOutbox()
                                 .It()
                                 .Delete(itemID);
            }

            return foundInThread || foundInBox;
        }
        default: {
            return false;
        }
    }
}

bool Storage::RemoveServer(const std::string& id) const
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Servers()
        .It()
        .Delete(id);
}

bool Storage::RemoveUnitDefinition(const std::string& id) const
{
    return mutable_Root().It().mutable_Tree().It().mutable_Units().It().Delete(
        id);
}

bool Storage::RenameThread(
    const std::string& nymId,
    const std::string& threadId,
    const std::string& newID) const
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Nyms()
        .It()
        .mutable_Nym(nymId)
        .It()
        .mutable_Threads()
        .It()
        .Rename(threadId, newID);
}

void Storage::RunGC() const
{
    if (!running_) { return; }

    CollectGarbage();
}

void Storage::RunMapPublicNyms(NymLambda lambda) const
{
    return Root().Tree().NymNode().Map(lambda);
}

void Storage::RunMapServers(ServerLambda lambda) const
{
    return Root().Tree().ServerNode().Map(lambda);
}

void Storage::RunMapUnits(UnitLambda lambda) const
{
    return Root().Tree().UnitNode().Map(lambda);
}

void Storage::save(opentxs::storage::Root* in, const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));
    OT_ASSERT(nullptr != in);

    multiplex_.StoreRoot(true, in->root_);
}

bool Storage::SetContactAlias(const std::string& id, const std::string& alias)
    const
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Contacts()
        .It()
        .SetAlias(id, alias);
}

bool Storage::SetDefaultSeed(const std::string& id) const
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Seeds()
        .It()
        .SetDefault(id);
}

bool Storage::SetNymAlias(const std::string& id, const std::string& alias) const
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Nyms()
        .It()
        .mutable_Nym(id)
        .It()
        .SetAlias(alias);
}

bool Storage::SetPeerRequestTime(
    const std::string& nymID,
    const std::string& id,
    const StorageBox box) const
{
    const std::string now = std::to_string(time(nullptr));

    switch (box) {
        case StorageBox::SENTPEERREQUEST: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_SentRequestBox()
                .It()
                .SetAlias(id, now);
        } break;
        case StorageBox::INCOMINGPEERREQUEST: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_IncomingRequestBox()
                .It()
                .SetAlias(id, now);
        } break;
        case StorageBox::FINISHEDPEERREQUEST: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_FinishedRequestBox()
                .It()
                .SetAlias(id, now);
        } break;
        case StorageBox::PROCESSEDPEERREQUEST: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_ProcessedRequestBox()
                .It()
                .SetAlias(id, now);
        } break;
        default: {
            return false;
        }
    }
}

bool Storage::SetReadState(
    const std::string& nymId,
    const std::string& threadId,
    const std::string& itemId,
    const bool unread) const
{
    auto& nyms = mutable_Root().It().mutable_Tree().It().mutable_Nyms().It();

    if (false == nyms.Exists(nymId)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymId
              << " does not exist." << std::endl;

        return false;
    }

    auto& threads = nyms.mutable_Nym(nymId).It().mutable_Threads().It();

    if (false == threads.Exists(threadId)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Thread " << threadId
              << " does not exist." << std::endl;

        return false;
    }

    return threads.mutable_Thread(threadId).It().Read(itemId, unread);
}

bool Storage::SetSeedAlias(const std::string& id, const std::string& alias)
    const
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Seeds()
        .It()
        .SetAlias(id, alias);
}

bool Storage::SetServerAlias(const std::string& id, const std::string& alias)
    const
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Servers()
        .It()
        .SetAlias(id, alias);
}

bool Storage::SetThreadAlias(
    const std::string& nymId,
    const std::string& threadId,
    const std::string& alias) const
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Nyms()
        .It()
        .mutable_Nym(nymId)
        .It()
        .mutable_Threads()
        .It()
        .mutable_Thread(threadId)
        .It()
        .SetAlias(alias);
}

bool Storage::SetUnitDefinitionAlias(
    const std::string& id,
    const std::string& alias) const
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Units()
        .It()
        .SetAlias(id, alias);
}

std::string Storage::ServerAlias(const std::string& id) const
{
    return Root().Tree().ServerNode().Alias(id);
}

ObjectList Storage::ServerList() const
{
    return Root().Tree().ServerNode().List();
}

void Storage::start() { InitPlugins(); }

bool Storage::Store(
    const std::string& accountID,
    const std::string& data,
    const std::string& alias,
    const Identifier& ownerNym,
    const Identifier& signerNym,
    const Identifier& issuerNym,
    const Identifier& server,
    const Identifier& contract,
    const proto::ContactItemType unit) const
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Accounts()
        .It()
        .Store(
            accountID,
            data,
            alias,
            ownerNym,
            signerNym,
            issuerNym,
            server,
            contract,
            unit);
}

bool Storage::Store(
    const std::string& nymID,
    const proto::ContactItemType type,
    const proto::Bip44Account& data) const
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Nyms()
        .It()
        .mutable_Nym(nymID)
        .It()
        .Store(type, data);
}

bool Storage::Store(const std::string& nymID, const proto::Bip47Context& data)
    const
{
    const bool exists = Root().Tree().NymNode().Exists(nymID);

    if (false == exists) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID
              << " doesn't exist." << std::endl;

        return false;
    }

    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Nyms()
        .It()
        .mutable_Nym(nymID)
        .It()
        .Store(data);
}

bool Storage::Store(const proto::BlockchainTransaction& data) const
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Blockchain()
        .It()
        .Store(data);
}

bool Storage::Store(const proto::Contact& data) const
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Contacts()
        .It()
        .Store(data, data.label());
}

bool Storage::Store(const proto::Context& data) const
{
    std::string notUsed;

    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Nyms()
        .It()
        .mutable_Nym(data.localnym())
        .It()
        .mutable_Contexts()
        .It()
        .Store(data, notUsed);
}

bool Storage::Store(const proto::Credential& data) const
{
    std::string notUsed;

    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Credentials()
        .It()
        .Store(data, notUsed);
}

bool Storage::Store(
    const proto::CredentialIndex& data,
    const std::string& alias) const
{
    std::string plaintext;
    const bool saved = mutable_Root()
                           .It()
                           .mutable_Tree()
                           .It()
                           .mutable_Nyms()
                           .It()
                           .mutable_Nym(data.nymid())
                           .It()
                           .Store(data, alias, plaintext);

    if (saved) {
        if (config_.auto_publish_nyms_ && config_.dht_callback_) {
            config_.dht_callback_(data.nymid(), plaintext);
        }

        return true;
    }

    return false;
}

bool Storage::Store(const std::string& nymID, const proto::Issuer& data) const
{
    const bool exists = Root().Tree().NymNode().Exists(nymID);

    if (false == exists) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID
              << " doesn't exist." << std::endl;

        return false;
    }

    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Nyms()
        .It()
        .mutable_Nym(nymID)
        .It()
        .mutable_Issuers()
        .It()
        .Store(data, {""});
}

bool Storage::Store(
    const std::string& nymID,
    const proto::PaymentWorkflow& data) const
{
    const bool exists = Root().Tree().NymNode().Exists(nymID);

    if (false == exists) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID
              << " doesn't exist." << std::endl;

        return false;
    }

    std::string notUsed{};

    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Nyms()
        .It()
        .mutable_Nym(nymID)
        .It()
        .mutable_PaymentWorkflows()
        .It()
        .Store(data, notUsed);
}

bool Storage::Store(
    const std::string& nymid,
    const std::string& threadid,
    const std::string& itemid,
    const std::uint64_t time,
    const std::string& alias,
    const std::string& data,
    const StorageBox box,
    const std::string& account) const
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Nyms()
        .It()
        .mutable_Nym(nymid)
        .It()
        .mutable_Threads()
        .It()
        .mutable_Thread(threadid)
        .It()
        .Add(itemid, time, box, alias, data, 0, account);
}

bool Storage::Store(
    const proto::PeerReply& data,
    const std::string& nymID,
    const StorageBox box) const
{
    switch (box) {
        case StorageBox::SENTPEERREPLY: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_SentReplyBox()
                .It()
                .Store(data);
        } break;
        case StorageBox::INCOMINGPEERREPLY: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_IncomingReplyBox()
                .It()
                .Store(data);
        } break;
        case StorageBox::FINISHEDPEERREPLY: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_FinishedReplyBox()
                .It()
                .Store(data);
        } break;
        case StorageBox::PROCESSEDPEERREPLY: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_ProcessedReplyBox()
                .It()
                .Store(data);
        } break;
        default: {
            return false;
        }
    }
}

bool Storage::Store(
    const proto::PeerRequest& data,
    const std::string& nymID,
    const StorageBox box) const
{
    // Use the alias field to store the time at which the request was saved.
    // Useful for managing retry logic in the high level api
    const std::string now = std::to_string(time(nullptr));

    switch (box) {
        case StorageBox::SENTPEERREQUEST: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_SentRequestBox()
                .It()
                .Store(data, now);
        } break;
        case StorageBox::INCOMINGPEERREQUEST: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_IncomingRequestBox()
                .It()
                .Store(data, now);
        } break;
        case StorageBox::FINISHEDPEERREQUEST: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_FinishedRequestBox()
                .It()
                .Store(data, now);
        } break;
        case StorageBox::PROCESSEDPEERREQUEST: {
            return mutable_Root()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_ProcessedRequestBox()
                .It()
                .Store(data, now);
        } break;
        default: {
            return false;
        }
    }
}

bool Storage::Store(const proto::Seed& data, const std::string& alias) const
{
    return mutable_Root().It().mutable_Tree().It().mutable_Seeds().It().Store(
        data, alias);
}

bool Storage::Store(const proto::ServerContract& data, const std::string& alias)
    const
{
    auto storageVersion(data);
    storageVersion.clear_publicnym();
    std::string plaintext;
    const bool saved =
        mutable_Root().It().mutable_Tree().It().mutable_Servers().It().Store(
            data, alias, plaintext);

    if (saved) {
        if (config_.auto_publish_servers_ && config_.dht_callback_) {
            config_.dht_callback_(storageVersion.id(), plaintext);
        }

        return true;
    }

    return false;
}

bool Storage::Store(const proto::UnitDefinition& data, const std::string& alias)
    const
{
    auto storageVersion(data);
    storageVersion.clear_publicnym();
    std::string plaintext;
    const bool saved =
        mutable_Root().It().mutable_Tree().It().mutable_Units().It().Store(
            data, alias, plaintext);

    if (saved) {
        if (config_.auto_publish_units_ && config_.dht_callback_) {
            config_.dht_callback_(storageVersion.id(), plaintext);
        }

        return true;
    }

    return false;
}

ObjectList Storage::ThreadList(const std::string& nymID, const bool unreadOnly)
    const
{
    return Root().Tree().NymNode().Nym(nymID).Threads().List(unreadOnly);
}

std::string Storage::ThreadAlias(
    const std::string& nymID,
    const std::string& threadID) const
{
    return Root()
        .Tree()
        .NymNode()
        .Nym(nymID)
        .Threads()
        .Thread(threadID)
        .Alias();
}

std::string Storage::UnitDefinitionAlias(const std::string& id) const
{
    return Root().Tree().UnitNode().Alias(id);
}

ObjectList Storage::UnitDefinitionList() const
{
    return Root().Tree().UnitNode().List();
}

std::size_t Storage::UnreadCount(
    const std::string& nymId,
    const std::string& threadId) const
{
    auto& nyms = Root().Tree().NymNode();

    if (false == nyms.Exists(nymId)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymId
              << " does not exist." << std::endl;

        return 0;
    }

    auto& threads = nyms.Nym(nymId).Threads();

    if (false == threads.Exists(threadId)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Thread " << threadId
              << " does not exist." << std::endl;

        return 0;
    }

    return threads.Thread(threadId).UnreadCount();
}

void Storage::UpgradeNyms()
{
    const auto level = Root().Tree().NymNode().UpgradeLevel();

    if (3 > level) {
        otErr << OT_METHOD << __FUNCTION__ << ": Beginning upgrade for version "
              << level << std::endl;
        mutable_Root()
            .It()
            .mutable_Tree()
            .It()
            .mutable_Nyms()
            .It()
            .UpgradeLocalnym();
    } else {
        otInfo << OT_METHOD << __FUNCTION__ << ": No need to upgrade version "
               << level << std::endl;
    }
}

bool Storage::verify_write_lock(const Lock& lock) const
{
    if (lock.mutex() != &write_lock_) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect mutex." << std::endl;

        return false;
    }

    if (false == lock.owns_lock()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Lock not owned." << std::endl;

        return false;
    }

    return true;
}

Storage::~Storage() { Cleanup_Storage(); }
}  // namespace opentxs::api::storage::implementation

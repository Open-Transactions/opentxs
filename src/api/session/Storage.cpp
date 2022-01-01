// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"             // IWYU pragma: associated
#include "1_Internal.hpp"           // IWYU pragma: associated
#include "api/session/Storage.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <ctime>
#include <functional>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

#include "Proto.hpp"
#include "Proto.tpp"
#include "core/OTStorage.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Factory.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/storage/drivers/Drivers.hpp"
#include "internal/storage/drivers/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/contact/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Editor.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/Bip47Channel.pb.h"
#include "serialization/protobuf/Ciphertext.pb.h"
#include "serialization/protobuf/Contact.pb.h"
#include "serialization/protobuf/Context.pb.h"
#include "serialization/protobuf/Credential.pb.h"
#include "serialization/protobuf/HDAccount.pb.h"
#include "serialization/protobuf/Issuer.pb.h"
#include "serialization/protobuf/Nym.pb.h"
#include "serialization/protobuf/PaymentWorkflow.pb.h"
#include "serialization/protobuf/PeerReply.pb.h"
#include "serialization/protobuf/PeerRequest.pb.h"
#include "serialization/protobuf/Purse.pb.h"
#include "serialization/protobuf/Seed.pb.h"
#include "serialization/protobuf/ServerContract.pb.h"
#include "serialization/protobuf/StorageThread.pb.h"
#include "serialization/protobuf/StorageThreadItem.pb.h"
#include "serialization/protobuf/UnitDefinition.pb.h"
#include "storage/Config.hpp"
#include "storage/tree/Accounts.hpp"
#include "storage/tree/Bip47Channels.hpp"
#include "storage/tree/Contacts.hpp"
#include "storage/tree/Contexts.hpp"
#include "storage/tree/Credentials.hpp"
#include "storage/tree/Issuers.hpp"
#include "storage/tree/Mailbox.hpp"
#include "storage/tree/Notary.hpp"
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

namespace opentxs::factory
{
auto StorageAPI(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const Flag& running,
    const opentxs::storage::Config& config) noexcept
    -> std::unique_ptr<api::session::Storage>
{
    {
        auto otdb =
            std::unique_ptr<OTDB::StorageFS>{OTDB::StorageFS::Instantiate()};
    }

    return std::make_unique<api::session::imp::Storage>(
        crypto, asio, running, config);
}
}  // namespace opentxs::factory

namespace opentxs::api::session::imp
{
const std::uint32_t Storage::HASH_TYPE = 2;  // BTC160

Storage::Storage(
    const api::Crypto& crypto,
    const network::Asio& asio,
    const Flag& running,
    const opentxs::storage::Config& config)
    : crypto_(crypto)
    , asio_(asio)
    , running_(running)
    , gc_interval_(config.gc_interval_)
    , write_lock_()
    , root_(nullptr)
    , primary_bucket_(Flag::Factory(false))
    , config_(config)
    , multiplex_p_(factory::StorageMultiplex(
          crypto_,
          asio_,
          *this,
          primary_bucket_,
          config_))
    , multiplex_(*multiplex_p_)
{
    OT_ASSERT(multiplex_p_);
}

auto Storage::AccountAlias(const Identifier& accountID) const -> std::string
{
    return Root().Tree().Accounts().Alias(accountID.str());
}

auto Storage::AccountList() const -> ObjectList
{
    return Root().Tree().Accounts().List();
}

auto Storage::AccountContract(const Identifier& accountID) const -> OTUnitID
{
    return Root().Tree().Accounts().AccountContract(accountID);
}

auto Storage::AccountIssuer(const Identifier& accountID) const -> OTNymID
{
    return Root().Tree().Accounts().AccountIssuer(accountID);
}

auto Storage::AccountOwner(const Identifier& accountID) const -> OTNymID
{
    return Root().Tree().Accounts().AccountOwner(accountID);
}

auto Storage::AccountServer(const Identifier& accountID) const -> OTServerID
{
    return Root().Tree().Accounts().AccountServer(accountID);
}

auto Storage::AccountSigner(const Identifier& accountID) const -> OTNymID
{
    return Root().Tree().Accounts().AccountSigner(accountID);
}

auto Storage::AccountUnit(const Identifier& accountID) const -> core::UnitType
{
    return Root().Tree().Accounts().AccountUnit(accountID);
}

auto Storage::AccountsByContract(const identifier::UnitDefinition& contract)
    const -> std::pmr::set<OTIdentifier>
{
    return Root().Tree().Accounts().AccountsByContract(contract);
}

auto Storage::AccountsByIssuer(const identifier::Nym& issuerNym) const
    -> std::pmr::set<OTIdentifier>
{
    return Root().Tree().Accounts().AccountsByIssuer(issuerNym);
}

auto Storage::AccountsByOwner(const identifier::Nym& ownerNym) const
    -> std::pmr::set<OTIdentifier>
{
    return Root().Tree().Accounts().AccountsByOwner(ownerNym);
}

auto Storage::AccountsByServer(const identifier::Server& server) const
    -> std::pmr::set<OTIdentifier>
{
    return Root().Tree().Accounts().AccountsByServer(server);
}

auto Storage::AccountsByUnit(const core::UnitType unit) const
    -> std::pmr::set<OTIdentifier>
{
    return Root().Tree().Accounts().AccountsByUnit(unit);
}

auto Storage::Bip47Chain(
    const identifier::Nym& nymID,
    const Identifier& channelID) const -> core::UnitType
{
    const bool exists = Root().Tree().Nyms().Exists(nymID.str());

    if (false == exists) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return core::UnitType::Error;
    }

    return Root()
        .Tree()
        .Nyms()
        .Nym(nymID.str())
        .Bip47Channels()
        .Chain(channelID);
}

auto Storage::Bip47ChannelsByChain(
    const identifier::Nym& nymID,
    const core::UnitType chain) const -> Storage::Bip47ChannelList
{
    const bool exists = Root().Tree().Nyms().Exists(nymID.str());

    if (false == exists) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return {};
    }

    return Root()
        .Tree()
        .Nyms()
        .Nym(nymID.str())
        .Bip47Channels()
        .ChannelsByChain(chain);
}

auto Storage::blockchain_thread_item_id(
    const opentxs::blockchain::Type chain,
    const Data& txid) const noexcept -> std::string
{
    return opentxs::blockchain_thread_item_id(crypto_, chain, txid)->str();
}

auto Storage::BlockchainAccountList(
    const std::string& nymID,
    const core::UnitType type) const -> std::pmr::set<std::string>
{
    return Root().Tree().Nyms().Nym(nymID).BlockchainAccountList(type);
}

auto Storage::BlockchainAccountType(
    const std::string& nymID,
    const std::string& accountID) const -> core::UnitType
{
    return Root().Tree().Nyms().Nym(nymID).BlockchainAccountType(accountID);
}

auto Storage::BlockchainThreadMap(const identifier::Nym& nym, const Data& txid)
    const noexcept -> std::pmr::vector<OTIdentifier>
{
    const auto& nyms = Root().Tree().Nyms();

    if (false == nyms.Exists(nym.str())) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nym)(" does not exist.").Flush();

        return {};
    }

    return nyms.Nym(nym.str()).Threads().BlockchainThreadMap(txid);
}

auto Storage::BlockchainTransactionList(
    const identifier::Nym& nym) const noexcept -> std::pmr::vector<OTData>
{
    const auto& nyms = Root().Tree().Nyms();

    if (false == nyms.Exists(nym.str())) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nym)(" does not exist.").Flush();

        return {};
    }

    return nyms.Nym(nym.str()).Threads().BlockchainTransactionList();
}

auto Storage::CheckTokenSpent(
    const identifier::Server& notary,
    const identifier::UnitDefinition& unit,
    const std::uint64_t series,
    const std::string& key) const -> bool
{
    return Root().Tree().Notary(notary.str()).CheckSpent(unit, series, key);
}

void Storage::Cleanup_Storage()
{
    if (root_) { root_->cleanup(); }
}

void Storage::Cleanup() { Cleanup_Storage(); }

void Storage::CollectGarbage() const { Root().Migrate(multiplex_.Primary()); }

auto Storage::ContactAlias(const std::string& id) const -> std::string
{
    return Root().Tree().Contacts().Alias(id);
}

auto Storage::ContactList() const -> ObjectList
{
    return Root().Tree().Contacts().List();
}

auto Storage::ContactOwnerNym(const std::string& nymID) const -> std::string
{
    return Root().Tree().Contacts().NymOwner(nymID);
}

void Storage::ContactSaveIndices() const
{
    mutable_Root().get().mutable_Tree().get().mutable_Contacts().get().Save();
}

auto Storage::ContactUpgradeLevel() const -> VersionNumber
{
    return Root().Tree().Contacts().UpgradeLevel();
}

auto Storage::ContextList(const std::string& nymID) const -> ObjectList
{
    return Root().Tree().Nyms().Nym(nymID).Contexts().List();
}

auto Storage::CreateThread(
    const std::string& nymID,
    const std::string& threadID,
    const std::pmr::set<std::string>& participants) const -> bool
{
    const auto id = mutable_Root()
                        .get()
                        .mutable_Tree()
                        .get()
                        .mutable_Nyms()
                        .get()
                        .mutable_Nym(nymID)
                        .get()
                        .mutable_Threads()
                        .get()
                        .Create(threadID, participants);

    return (false == id.empty());
}

auto Storage::DefaultSeed() const -> std::string
{
    return Root().Tree().Seeds().Default();
}

auto Storage::DeleteAccount(const std::string& id) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Accounts()
        .get()
        .Delete(id);
}

auto Storage::DeleteContact(const std::string& id) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Contacts()
        .get()
        .Delete(id);
}

auto Storage::DeletePaymentWorkflow(
    const std::string& nymID,
    const std::string& workflowID) const -> bool
{
    const bool exists = Root().Tree().Nyms().Exists(nymID);

    if (false == exists) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return false;
    }

    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nymID)
        .get()
        .mutable_PaymentWorkflows()
        .get()
        .Delete(workflowID);
}

auto Storage::HashType() const -> std::uint32_t { return HASH_TYPE; }

void Storage::InitBackup() { multiplex_.InitBackup(); }

void Storage::InitEncryptedBackup(opentxs::crypto::key::Symmetric& key)
{
    multiplex_.InitEncryptedBackup(key);
}

void Storage::InitPlugins()
{
    bool syncPrimary{false};
    const auto hash = multiplex_.BestRoot(syncPrimary);

    if (hash.empty()) { return; }

    std::unique_ptr<opentxs::storage::Root> root{new opentxs::storage::Root(
        asio_,
        multiplex_,
        hash,
        std::numeric_limits<std::int64_t>::max(),
        primary_bucket_)};

    OT_ASSERT(root);

    multiplex_.SynchronizePlugins(hash, *root, syncPrimary);
}

auto Storage::IssuerList(const std::string& nymID) const -> ObjectList
{
    const bool exists = Root().Tree().Nyms().Exists(nymID);

    if (false == exists) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Tree().Nyms().Nym(nymID).Issuers().List();
}

auto Storage::Load(
    const std::string& accountID,
    std::string& output,
    std::string& alias,
    const bool checking) const -> bool
{
    return Root().Tree().Accounts().Load(accountID, output, alias, checking);
}

auto Storage::Load(
    const std::string& nymID,
    const std::string& accountID,
    proto::HDAccount& output,
    const bool checking) const -> bool
{
    auto temp = std::make_shared<proto::HDAccount>(output);
    const auto rc =
        Root().Tree().Nyms().Nym(nymID).Load(accountID, temp, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::Nym& nymID,
    const Identifier& channelID,
    proto::Bip47Channel& output,
    const bool checking) const -> bool
{
    const bool exists = Root().Tree().Nyms().Exists(nymID.str());

    if (false == exists) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return false;
    }

    auto temp = std::make_shared<proto::Bip47Channel>(output);
    const auto rc = Root()
                        .Tree()
                        .Nyms()
                        .Nym(nymID.str())
                        .Bip47Channels()
                        .Load(channelID, temp, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const std::string& id,
    proto::Contact& output,
    const bool checking) const -> bool
{
    auto notUsed = std::string{};

    return Load(id, output, notUsed, checking);
}

auto Storage::Load(
    const std::string& id,
    proto::Contact& output,
    std::string& alias,
    const bool checking) const -> bool
{
    auto temp = std::make_shared<proto::Contact>(output);
    const auto rc = Root().Tree().Contacts().Load(id, temp, alias, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const std::string& nym,
    const std::string& id,
    proto::Context& output,
    const bool checking) const -> bool
{
    auto notUsed = std::string{};
    auto temp = std::make_shared<proto::Context>(output);
    const auto rc = Root().Tree().Nyms().Nym(nym).Contexts().Load(
        id, temp, notUsed, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const std::string& id,
    proto::Credential& output,
    const bool checking) const -> bool
{
    auto temp = std::make_shared<proto::Credential>(output);
    const auto rc = Root().Tree().Credentials().Load(id, temp, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::Nym& id,
    proto::Nym& output,
    const bool checking) const -> bool
{
    auto notUsed = std::string{};

    return Load(id, output, notUsed, checking);
}

auto Storage::LoadNym(
    const identifier::Nym& id,
    AllocateOutput destination,
    const bool checking) const -> bool
{
    auto temp = std::make_shared<proto::Nym>();
    auto alias = std::string{};

    if (false ==
        Root().Tree().Nyms().Nym(id.str()).Load(temp, alias, checking)) {
        LogError()(OT_PRETTY_CLASS())("Failed to load nym ")(id).Flush();

        return false;
    }

    OT_ASSERT(temp);

    return write(*temp, destination);
}

auto Storage::Load(
    const identifier::Nym& id,
    proto::Nym& output,
    std::string& alias,
    const bool checking) const -> bool
{
    auto temp = std::make_shared<proto::Nym>(output);
    const auto rc =
        Root().Tree().Nyms().Nym(id.str()).Load(temp, alias, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const std::string& nymID,
    const std::string& id,
    proto::Issuer& output,
    const bool checking) const -> bool
{
    if (false == Root().Tree().Nyms().Exists(nymID)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return false;
    }

    auto notUsed = std::string{};
    auto temp = std::make_shared<proto::Issuer>(output);
    const auto rc = Root().Tree().Nyms().Nym(nymID).Issuers().Load(
        id, temp, notUsed, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const std::string& nymID,
    const std::string& workflowID,
    proto::PaymentWorkflow& output,
    const bool checking) const -> bool
{
    if (false == Root().Tree().Nyms().Exists(nymID)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return false;
    }

    auto temp = std::make_shared<proto::PaymentWorkflow>(output);
    const auto rc = Root().Tree().Nyms().Nym(nymID).PaymentWorkflows().Load(
        workflowID, temp, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const std::string& nymID,
    const std::string& id,
    const StorageBox box,
    std::string& output,
    std::string& alias,
    const bool checking) const -> bool
{
    switch (box) {
        case StorageBox::MAILINBOX: {
            return Root().Tree().Nyms().Nym(nymID).MailInbox().Load(
                id, output, alias, checking);
        }
        case StorageBox::MAILOUTBOX: {
            return Root().Tree().Nyms().Nym(nymID).MailOutbox().Load(
                id, output, alias, checking);
        }
        default: {
            return false;
        }
    }
}

auto Storage::Load(
    const std::string& nymID,
    const std::string& id,
    const StorageBox box,
    proto::PeerReply& output,
    const bool checking) const -> bool
{
    auto temp = std::make_shared<proto::PeerReply>(output);
    const auto rc = [&] {
        switch (box) {
            case StorageBox::SENTPEERREPLY: {
                return Root().Tree().Nyms().Nym(nymID).SentReplyBox().Load(
                    id, temp, checking);
            }
            case StorageBox::INCOMINGPEERREPLY: {
                return Root().Tree().Nyms().Nym(nymID).IncomingReplyBox().Load(
                    id, temp, checking);
            }
            case StorageBox::FINISHEDPEERREPLY: {
                return Root().Tree().Nyms().Nym(nymID).FinishedReplyBox().Load(
                    id, temp, checking);
            }
            case StorageBox::PROCESSEDPEERREPLY: {
                return Root().Tree().Nyms().Nym(nymID).ProcessedReplyBox().Load(
                    id, temp, checking);
            }
            default: {
                return false;
            }
        }
    }();

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const std::string& nymID,
    const std::string& id,
    const StorageBox box,
    proto::PeerRequest& output,
    std::time_t& time,
    const bool checking) const -> bool
{
    auto temp = std::make_shared<proto::PeerRequest>(output);
    auto alias = std::string{};
    const auto rc = [&] {
        switch (box) {
            case StorageBox::SENTPEERREQUEST: {
                return Root().Tree().Nyms().Nym(nymID).SentRequestBox().Load(
                    id, temp, alias, checking);
            }
            case StorageBox::INCOMINGPEERREQUEST: {
                return Root()
                    .Tree()
                    .Nyms()
                    .Nym(nymID)
                    .IncomingRequestBox()
                    .Load(id, temp, alias, checking);
            }
            case StorageBox::FINISHEDPEERREQUEST: {
                return Root()
                    .Tree()
                    .Nyms()
                    .Nym(nymID)
                    .FinishedRequestBox()
                    .Load(id, temp, alias, checking);
            }
            case StorageBox::PROCESSEDPEERREQUEST: {
                return Root()
                    .Tree()
                    .Nyms()
                    .Nym(nymID)
                    .ProcessedRequestBox()
                    .Load(id, temp, alias, checking);
            }
            default: {

                return false;
            }
        }
    }();

    if (rc && temp) {
        output = *temp;

        try {
            time = std::stoi(alias);
        } catch (const std::invalid_argument&) {
            time = 0;
        } catch (const std::out_of_range&) {
            time = 0;
        }
    }

    return rc;
}

auto Storage::Load(
    const identifier::Nym& nym,
    const identifier::Server& notary,
    const identifier::UnitDefinition& unit,
    proto::Purse& output,
    const bool checking) const -> bool
{
    const auto& nymNode = Root().Tree().Nyms();

    if (false == nymNode.Exists(nym.str())) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nym)(" doesn't exist.").Flush();

        return false;
    }

    auto temp = std::make_shared<proto::Purse>(output);
    const auto rc = nymNode.Nym(nym.str()).Load(notary, unit, temp, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const std::string& id,
    proto::Seed& output,
    const bool checking) const -> bool
{
    auto notUsed = std::string{};

    return Load(id, output, notUsed, checking);
}

auto Storage::Load(
    const std::string& id,
    proto::Seed& output,
    std::string& alias,
    const bool checking) const -> bool
{
    auto temp = std::make_shared<proto::Seed>(output);
    const auto rc = Root().Tree().Seeds().Load(id, temp, alias, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::Server& id,
    proto::ServerContract& output,
    const bool checking) const -> bool
{
    auto notUsed = std::string{};

    return Load(id, output, notUsed, checking);
}

auto Storage::Load(
    const identifier::Server& id,
    proto::ServerContract& output,
    std::string& alias,
    const bool checking) const -> bool
{
    auto temp = std::make_shared<proto::ServerContract>(output);
    const auto rc =
        Root().Tree().Servers().Load(id.str(), temp, alias, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const std::string& nymId,
    const std::string& threadId,
    proto::StorageThread& output) const -> bool
{
    const bool exists =
        Root().Tree().Nyms().Nym(nymId).Threads().Exists(threadId);

    if (!exists) { return false; }

    output = Root().Tree().Nyms().Nym(nymId).Threads().Thread(threadId).Items();

    return true;
}

auto Storage::Load(proto::Ciphertext& output, const bool checking) const -> bool
{
    auto temp = std::make_shared<proto::Ciphertext>(output);
    const auto rc = Root().Tree().Load(temp, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::UnitDefinition& id,
    proto::UnitDefinition& output,
    const bool checking) const -> bool
{
    auto notUsed = std::string{};

    return Load(id, output, notUsed, checking);
}

auto Storage::Load(
    const identifier::UnitDefinition& id,
    proto::UnitDefinition& output,
    std::string& alias,
    const bool checking) const -> bool
{
    auto temp = std::make_shared<proto::UnitDefinition>(output);
    const auto rc = Root().Tree().Units().Load(id.str(), temp, alias, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::LocalNyms() const -> const std::pmr::set<std::string>
{
    return Root().Tree().Nyms().LocalNyms();
}

// Applies a lambda to all public nyms in the database in a detached thread.
void Storage::MapPublicNyms(NymLambda& cb) const
{
    asio_.Internal().Post(ThreadPool::General, [=] { RunMapPublicNyms(cb); });
}

// Applies a lambda to all server contracts in the database in a detached
// thread.
void Storage::MapServers(ServerLambda& cb) const
{
    asio_.Internal().Post(ThreadPool::General, [=] { RunMapServers(cb); });
}

// Applies a lambda to all unit definitions in the database in a detached
// thread.
void Storage::MapUnitDefinitions(UnitLambda& cb) const
{
    asio_.Internal().Post(ThreadPool::General, [=] { RunMapUnits(cb); });
}

auto Storage::MarkTokenSpent(
    const identifier::Server& notary,
    const identifier::UnitDefinition& unit,
    const std::uint64_t series,
    const std::string& key) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Notary(notary.str())
        .get()
        .MarkSpent(unit, series, key);
}

auto Storage::MoveThreadItem(
    const std::string& nymId,
    const std::string& fromThreadID,
    const std::string& toThreadID,
    const std::string& itemID) const -> bool
{
    const auto& nyms = Root().Tree().Nyms();

    if (false == nyms.Exists(nymId)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymId)(" does not exist.")
            .Flush();

        return false;
    }

    const auto& threads = nyms.Nym(nymId).Threads();

    if (false == threads.Exists(fromThreadID)) {
        LogError()(OT_PRETTY_CLASS())("Source thread ")(
            fromThreadID)(" does not exist.")
            .Flush();

        return false;
    }

    if (false == threads.Exists(toThreadID)) {
        LogError()(OT_PRETTY_CLASS())("Destination thread ")(
            toThreadID)(" does not exist.")
            .Flush();

        return false;
    }

    auto& fromThread = mutable_Root()
                           .get()
                           .mutable_Tree()
                           .get()
                           .mutable_Nyms()
                           .get()
                           .mutable_Nym(nymId)
                           .get()
                           .mutable_Threads()
                           .get()
                           .mutable_Thread(fromThreadID)
                           .get();
    const auto thread = fromThread.Items();
    auto found{false};
    auto time = std::uint64_t{};
    auto box = StorageBox{};
    const auto alias = std::string{};
    const auto contents = std::string{};
    auto index = std::uint64_t{};
    auto account = std::string{};

    for (const auto& item : thread.item()) {
        if (item.id() == itemID) {
            found = true;
            time = item.time();
            box = static_cast<StorageBox>(item.box());
            index = item.index();
            account = item.account();

            break;
        }
    }

    if (false == found) {
        LogError()(OT_PRETTY_CLASS())("Item does not exist.").Flush();

        return false;
    }

    if (false == fromThread.Remove(itemID)) {
        LogError()(OT_PRETTY_CLASS())("Failed to remove item.").Flush();

        return false;
    }

    auto& toThread = mutable_Root()
                         .get()
                         .mutable_Tree()
                         .get()
                         .mutable_Nyms()
                         .get()
                         .mutable_Nym(nymId)
                         .get()
                         .mutable_Threads()
                         .get()
                         .mutable_Thread(toThreadID)
                         .get();
    const auto added =
        toThread.Add(itemID, time, box, alias, contents, index, account);

    if (false == added) {
        LogError()(OT_PRETTY_CLASS())("Failed to insert item.").Flush();

        return false;
    }

    return true;
}

auto Storage::mutable_Root() const -> Editor<opentxs::storage::Root>
{
    std::function<void(opentxs::storage::Root*, Lock&)> callback =
        [&](opentxs::storage::Root* in, Lock& lock) -> void {
        this->save(in, lock);
    };

    return Editor<opentxs::storage::Root>(write_lock_, root(), callback);
}

auto Storage::NymBoxList(const std::string& nymID, const StorageBox box) const
    -> ObjectList
{
    switch (box) {
        case StorageBox::SENTPEERREQUEST: {
            return Root().Tree().Nyms().Nym(nymID).SentRequestBox().List();
        }
        case StorageBox::INCOMINGPEERREQUEST: {
            return Root().Tree().Nyms().Nym(nymID).IncomingRequestBox().List();
        }
        case StorageBox::SENTPEERREPLY: {
            return Root().Tree().Nyms().Nym(nymID).SentReplyBox().List();
        }
        case StorageBox::INCOMINGPEERREPLY: {
            return Root().Tree().Nyms().Nym(nymID).IncomingReplyBox().List();
        }
        case StorageBox::FINISHEDPEERREQUEST: {
            return Root().Tree().Nyms().Nym(nymID).FinishedRequestBox().List();
        }
        case StorageBox::FINISHEDPEERREPLY: {
            return Root().Tree().Nyms().Nym(nymID).FinishedReplyBox().List();
        }
        case StorageBox::PROCESSEDPEERREQUEST: {
            return Root().Tree().Nyms().Nym(nymID).ProcessedRequestBox().List();
        }
        case StorageBox::PROCESSEDPEERREPLY: {
            return Root().Tree().Nyms().Nym(nymID).ProcessedReplyBox().List();
        }
        case StorageBox::MAILINBOX: {
            return Root().Tree().Nyms().Nym(nymID).MailInbox().List();
        }
        case StorageBox::MAILOUTBOX: {
            return Root().Tree().Nyms().Nym(nymID).MailOutbox().List();
        }
        default: {
            return {};
        }
    }
}

auto Storage::NymList() const -> ObjectList
{
    return Root().Tree().Nyms().List();
}

auto Storage::PaymentWorkflowList(const std::string& nymID) const -> ObjectList
{
    if (false == Root().Tree().Nyms().Exists(nymID)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Tree().Nyms().Nym(nymID).PaymentWorkflows().List();
}

auto Storage::PaymentWorkflowLookup(
    const std::string& nymID,
    const std::string& sourceID) const -> std::string
{
    if (false == Root().Tree().Nyms().Exists(nymID)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Tree().Nyms().Nym(nymID).PaymentWorkflows().LookupBySource(
        sourceID);
}

auto Storage::PaymentWorkflowsByAccount(
    const std::string& nymID,
    const std::string& accountID) const -> std::pmr::set<std::string>
{
    if (false == Root().Tree().Nyms().Exists(nymID)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Tree().Nyms().Nym(nymID).PaymentWorkflows().ListByAccount(
        accountID);
}

auto Storage::PaymentWorkflowsByState(
    const std::string& nymID,
    const otx::client::PaymentWorkflowType type,
    const otx::client::PaymentWorkflowState state) const
    -> std::pmr::set<std::string>
{
    if (false == Root().Tree().Nyms().Exists(nymID)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Tree().Nyms().Nym(nymID).PaymentWorkflows().ListByState(
        type, state);
}

auto Storage::PaymentWorkflowsByUnit(
    const std::string& nymID,
    const std::string& unitID) const -> std::pmr::set<std::string>
{
    if (false == Root().Tree().Nyms().Exists(nymID)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Tree().Nyms().Nym(nymID).PaymentWorkflows().ListByUnit(
        unitID);
}

auto Storage::PaymentWorkflowState(
    const std::string& nymID,
    const std::string& workflowID) const -> std::
    pair<otx::client::PaymentWorkflowType, otx::client::PaymentWorkflowState>
{
    if (false == Root().Tree().Nyms().Exists(nymID)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Tree().Nyms().Nym(nymID).PaymentWorkflows().GetState(
        workflowID);
}

auto Storage::RelabelThread(
    const std::string& threadID,
    const std::string& label) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Nyms()
        .get()
        .RelabelThread(threadID, label);
}

auto Storage::RemoveBlockchainThreadItem(
    const identifier::Nym& nym,
    const Identifier& threadID,
    const opentxs::blockchain::Type chain,
    const Data& txid) const noexcept -> bool
{
    const auto& nyms = Root().Tree().Nyms();

    if (false == nyms.Exists(nym.str())) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nym)(" does not exist.").Flush();

        return false;
    }

    const auto& threads = nyms.Nym(nym.str()).Threads();

    if (false == threads.Exists(threadID.str())) {
        LogError()(OT_PRETTY_CLASS())("Thread ")(threadID)(" does not exist.")
            .Flush();

        return false;
    }

    auto& fromThread = mutable_Root()
                           .get()
                           .mutable_Tree()
                           .get()
                           .mutable_Nyms()
                           .get()
                           .mutable_Nym(nym.str())
                           .get()
                           .mutable_Threads(txid, threadID, false)
                           .get()
                           .mutable_Thread(threadID.str())
                           .get();
    const auto thread = fromThread.Items();
    auto found{false};
    const auto id = blockchain_thread_item_id(chain, txid);

    for (const auto& item : thread.item()) {
        if (item.id() == id) {
            found = true;

            break;
        }
    }

    if (false == found) {
        LogError()(OT_PRETTY_CLASS())("Item does not exist.").Flush();

        return false;
    }

    if (false == fromThread.Remove(id)) {
        LogError()(OT_PRETTY_CLASS())("Failed to remove item.").Flush();

        return false;
    }

    return true;
}

auto Storage::RemoveNymBoxItem(
    const std::string& nymID,
    const StorageBox box,
    const std::string& itemID) const -> bool
{
    switch (box) {
        case StorageBox::SENTPEERREQUEST: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_SentRequestBox()
                .get()
                .Delete(itemID);
        }
        case StorageBox::INCOMINGPEERREQUEST: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_IncomingRequestBox()
                .get()
                .Delete(itemID);
        }
        case StorageBox::SENTPEERREPLY: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_SentReplyBox()
                .get()
                .Delete(itemID);
        }
        case StorageBox::INCOMINGPEERREPLY: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_IncomingReplyBox()
                .get()
                .Delete(itemID);
        }
        case StorageBox::FINISHEDPEERREQUEST: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_FinishedRequestBox()
                .get()
                .Delete(itemID);
        }
        case StorageBox::FINISHEDPEERREPLY: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_FinishedReplyBox()
                .get()
                .Delete(itemID);
        }
        case StorageBox::PROCESSEDPEERREQUEST: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_ProcessedRequestBox()
                .get()
                .Delete(itemID);
        }
        case StorageBox::PROCESSEDPEERREPLY: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_ProcessedReplyBox()
                .get()
                .Delete(itemID);
        }
        case StorageBox::MAILINBOX: {
            const bool foundInThread = mutable_Root()
                                           .get()
                                           .mutable_Tree()
                                           .get()
                                           .mutable_Nyms()
                                           .get()
                                           .mutable_Nym(nymID)
                                           .get()
                                           .mutable_Threads()
                                           .get()
                                           .FindAndDeleteItem(itemID);
            bool foundInBox = false;

            if (!foundInThread) {
                foundInBox = mutable_Root()
                                 .get()
                                 .mutable_Tree()
                                 .get()
                                 .mutable_Nyms()
                                 .get()
                                 .mutable_Nym(nymID)
                                 .get()
                                 .mutable_MailInbox()
                                 .get()
                                 .Delete(itemID);
            }

            return foundInThread || foundInBox;
        }
        case StorageBox::MAILOUTBOX: {
            const bool foundInThread = mutable_Root()
                                           .get()
                                           .mutable_Tree()
                                           .get()
                                           .mutable_Nyms()
                                           .get()
                                           .mutable_Nym(nymID)
                                           .get()
                                           .mutable_Threads()
                                           .get()
                                           .FindAndDeleteItem(itemID);
            bool foundInBox = false;

            if (!foundInThread) {
                foundInBox = mutable_Root()
                                 .get()
                                 .mutable_Tree()
                                 .get()
                                 .mutable_Nyms()
                                 .get()
                                 .mutable_Nym(nymID)
                                 .get()
                                 .mutable_MailOutbox()
                                 .get()
                                 .Delete(itemID);
            }

            return foundInThread || foundInBox;
        }
        default: {
            return false;
        }
    }
}

auto Storage::RemoveServer(const std::string& id) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Servers()
        .get()
        .Delete(id);
}

auto Storage::RemoveThreadItem(
    const identifier::Nym& nym,
    const Identifier& threadID,
    const std::string& id) const -> bool
{
    const auto& nyms = Root().Tree().Nyms();

    if (false == nyms.Exists(nym.str())) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nym)(" does not exist.").Flush();

        return false;
    }

    const auto& threads = nyms.Nym(nym.str()).Threads();

    if (false == threads.Exists(threadID.str())) {
        LogError()(OT_PRETTY_CLASS())("Thread ")(threadID)(" does not exist.")
            .Flush();

        return false;
    }

    auto& fromThread = mutable_Root()
                           .get()
                           .mutable_Tree()
                           .get()
                           .mutable_Nyms()
                           .get()
                           .mutable_Nym(nym.str())
                           .get()
                           .mutable_Threads()
                           .get()
                           .mutable_Thread(threadID.str())
                           .get();
    const auto thread = fromThread.Items();
    auto found{false};

    for (const auto& item : thread.item()) {
        if (item.id() == id) {
            found = true;

            break;
        }
    }

    if (false == found) {
        LogError()(OT_PRETTY_CLASS())("Item does not exist.").Flush();

        return false;
    }

    if (false == fromThread.Remove(id)) {
        LogError()(OT_PRETTY_CLASS())("Failed to remove item.").Flush();

        return false;
    }

    return true;
}

auto Storage::RemoveUnitDefinition(const std::string& id) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Units()
        .get()
        .Delete(id);
}

auto Storage::RenameThread(
    const std::string& nymId,
    const std::string& threadId,
    const std::string& newID) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nymId)
        .get()
        .mutable_Threads()
        .get()
        .Rename(threadId, newID);
}

auto Storage::root() const -> opentxs::storage::Root*
{
    Lock lock(write_lock_);

    if (!root_) {
        root_.reset(new opentxs::storage::Root(
            asio_,
            multiplex_,
            multiplex_.LoadRoot(),
            gc_interval_,
            primary_bucket_));
    }

    OT_ASSERT(root_);

    lock.unlock();

    return root_.get();
}

auto Storage::Root() const -> const opentxs::storage::Root& { return *root(); }

void Storage::RunGC() const
{
    if (!running_) { return; }

    CollectGarbage();
}

void Storage::RunMapPublicNyms(NymLambda lambda) const
{
    return Root().Tree().Nyms().Map(lambda);
}

void Storage::RunMapServers(ServerLambda lambda) const
{
    return Root().Tree().Servers().Map(lambda);
}

void Storage::RunMapUnits(UnitLambda lambda) const
{
    return Root().Tree().Units().Map(lambda);
}

void Storage::save(opentxs::storage::Root* in, const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));
    OT_ASSERT(nullptr != in);

    multiplex_.StoreRoot(true, in->root_);
}

auto Storage::SeedList() const -> ObjectList
{
    return Root().Tree().Seeds().List();
}

auto Storage::SetAccountAlias(const std::string& id, const std::string& alias)
    const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Accounts()
        .get()
        .SetAlias(id, alias);
}

auto Storage::SetContactAlias(const std::string& id, const std::string& alias)
    const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Contacts()
        .get()
        .SetAlias(id, alias);
}

auto Storage::SetDefaultSeed(const std::string& id) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Seeds()
        .get()
        .SetDefault(id);
}

auto Storage::SetNymAlias(const identifier::Nym& id, const std::string& alias)
    const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(id.str())
        .get()
        .SetAlias(alias);
}

auto Storage::SetPeerRequestTime(
    const std::string& nymID,
    const std::string& id,
    const StorageBox box) const -> bool
{
    const std::string now = std::to_string(time(nullptr));

    switch (box) {
        case StorageBox::SENTPEERREQUEST: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_SentRequestBox()
                .get()
                .SetAlias(id, now);
        }
        case StorageBox::INCOMINGPEERREQUEST: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_IncomingRequestBox()
                .get()
                .SetAlias(id, now);
        }
        case StorageBox::FINISHEDPEERREQUEST: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_FinishedRequestBox()
                .get()
                .SetAlias(id, now);
        }
        case StorageBox::PROCESSEDPEERREQUEST: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_ProcessedRequestBox()
                .get()
                .SetAlias(id, now);
        }
        default: {
            return false;
        }
    }
}

auto Storage::SetReadState(
    const std::string& nymId,
    const std::string& threadId,
    const std::string& itemId,
    const bool unread) const -> bool
{
    auto& nyms = mutable_Root().get().mutable_Tree().get().mutable_Nyms().get();

    if (false == nyms.Exists(nymId)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymId)(" does not exist.")
            .Flush();

        return false;
    }

    auto& threads = nyms.mutable_Nym(nymId).get().mutable_Threads().get();

    if (false == threads.Exists(threadId)) {
        LogError()(OT_PRETTY_CLASS())("Thread ")(threadId)(" does not exist.")
            .Flush();

        return false;
    }

    return threads.mutable_Thread(threadId).get().Read(itemId, unread);
}

auto Storage::SetSeedAlias(const std::string& id, const std::string& alias)
    const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Seeds()
        .get()
        .SetAlias(id, alias);
}

auto Storage::SetServerAlias(
    const identifier::Server& id,
    const std::string& alias) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Servers()
        .get()
        .SetAlias(id.str(), alias);
}

auto Storage::SetThreadAlias(
    const std::string& nymId,
    const std::string& threadId,
    const std::string& alias) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nymId)
        .get()
        .mutable_Threads()
        .get()
        .mutable_Thread(threadId)
        .get()
        .SetAlias(alias);
}

auto Storage::SetUnitDefinitionAlias(
    const identifier::UnitDefinition& id,
    const std::string& alias) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Units()
        .get()
        .SetAlias(id.str(), alias);
}

auto Storage::ServerAlias(const std::string& id) const -> std::string
{
    return Root().Tree().Servers().Alias(id);
}

auto Storage::ServerList() const -> ObjectList
{
    return Root().Tree().Servers().List();
}

void Storage::start() { InitPlugins(); }

auto Storage::Store(
    const std::string& accountID,
    const std::string& data,
    const std::string& alias,
    const identifier::Nym& ownerNym,
    const identifier::Nym& signerNym,
    const identifier::Nym& issuerNym,
    const identifier::Server& server,
    const identifier::UnitDefinition& contract,
    const core::UnitType unit) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Accounts()
        .get()
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

auto Storage::Store(
    const std::string& nymID,
    const contact::ClaimType type,
    const proto::HDAccount& data) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nymID)
        .get()
        .Store(ClaimToUnit(type), data);
}

auto Storage::Store(
    const identifier::Nym& nymID,
    const Identifier& channelID,
    const proto::Bip47Channel& data) const -> bool
{
    const bool exists = Root().Tree().Nyms().Exists(nymID.str());

    if (false == exists) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return false;
    }

    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nymID.str())
        .get()
        .mutable_Bip47Channels()
        .get()
        .Store(channelID, data);
}

auto Storage::Store(const proto::Contact& data) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Contacts()
        .get()
        .Store(data, data.label());
}

auto Storage::Store(const proto::Context& data) const -> bool
{
    auto notUsed = std::string{};

    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(data.localnym())
        .get()
        .mutable_Contexts()
        .get()
        .Store(data, notUsed);
}

auto Storage::Store(const proto::Credential& data) const -> bool
{
    auto notUsed = std::string{};

    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Credentials()
        .get()
        .Store(data, notUsed);
}

auto Storage::Store(const proto::Nym& data, const std::string& alias) const
    -> bool
{
    std::string plaintext;
    const bool saved = mutable_Root()
                           .get()
                           .mutable_Tree()
                           .get()
                           .mutable_Nyms()
                           .get()
                           .mutable_Nym(data.nymid())
                           .get()
                           .Store(data, alias, plaintext);

    if (saved) {
        if (config_.auto_publish_nyms_ && config_.dht_callback_) {
            config_.dht_callback_(data.nymid(), plaintext);
        }

        return true;
    }

    return false;
}

auto Storage::Store(const ReadView& view, const std::string& alias) const
    -> bool
{
    return Store(proto::Factory<proto::Nym>(view), alias);
}

auto Storage::Store(const std::string& nymID, const proto::Issuer& data) const
    -> bool
{
    const bool exists = Root().Tree().Nyms().Exists(nymID);

    if (false == exists) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return false;
    }

    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nymID)
        .get()
        .mutable_Issuers()
        .get()
        .Store(data, {""});
}

auto Storage::Store(
    const std::string& nymID,
    const proto::PaymentWorkflow& data) const -> bool
{
    const bool exists = Root().Tree().Nyms().Exists(nymID);

    if (false == exists) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return false;
    }

    std::string notUsed{};

    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nymID)
        .get()
        .mutable_PaymentWorkflows()
        .get()
        .Store(data, notUsed);
}

auto Storage::Store(
    const std::string& nymid,
    const std::string& threadid,
    const std::string& itemid,
    const std::uint64_t time,
    const std::string& alias,
    const std::string& data,
    const StorageBox box,
    const std::string& account) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nymid)
        .get()
        .mutable_Threads()
        .get()
        .mutable_Thread(threadid)
        .get()
        .Add(itemid, time, box, alias, data, 0, account);
}

auto Storage::Store(
    const identifier::Nym& nym,
    const Identifier& thread,
    const opentxs::blockchain::Type chain,
    const Data& txid,
    const Time time) const noexcept -> bool
{
    const auto alias = std::string{};
    const auto account = std::string{};
    const auto id = blockchain_thread_item_id(chain, txid);

    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nym.str())
        .get()
        .mutable_Threads(txid, thread, true)
        .get()
        .mutable_Thread(thread.str())
        .get()
        .Add(
            id,
            Clock::to_time_t(time),
            StorageBox::BLOCKCHAIN,
            alias,
            txid.str(),
            0,
            account,
            static_cast<std::uint32_t>(chain));
}

auto Storage::Store(
    const proto::PeerReply& data,
    const std::string& nymID,
    const StorageBox box) const -> bool
{
    switch (box) {
        case StorageBox::SENTPEERREPLY: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_SentReplyBox()
                .get()
                .Store(data);
        }
        case StorageBox::INCOMINGPEERREPLY: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_IncomingReplyBox()
                .get()
                .Store(data);
        }
        case StorageBox::FINISHEDPEERREPLY: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_FinishedReplyBox()
                .get()
                .Store(data);
        }
        case StorageBox::PROCESSEDPEERREPLY: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_ProcessedReplyBox()
                .get()
                .Store(data);
        }
        default: {
            return false;
        }
    }
}

auto Storage::Store(
    const proto::PeerRequest& data,
    const std::string& nymID,
    const StorageBox box) const -> bool
{
    // Use the alias field to store the time at which the request was saved.
    // Useful for managing retry logic in the high level api
    const std::string now = std::to_string(time(nullptr));

    switch (box) {
        case StorageBox::SENTPEERREQUEST: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_SentRequestBox()
                .get()
                .Store(data, now);
        }
        case StorageBox::INCOMINGPEERREQUEST: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_IncomingRequestBox()
                .get()
                .Store(data, now);
        }
        case StorageBox::FINISHEDPEERREQUEST: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_FinishedRequestBox()
                .get()
                .Store(data, now);
        }
        case StorageBox::PROCESSEDPEERREQUEST: {
            return mutable_Root()
                .get()
                .mutable_Tree()
                .get()
                .mutable_Nyms()
                .get()
                .mutable_Nym(nymID)
                .get()
                .mutable_ProcessedRequestBox()
                .get()
                .Store(data, now);
        }
        default: {
            return false;
        }
    }
}

auto Storage::Store(const identifier::Nym& nym, const proto::Purse& purse) const
    -> bool
{
    auto nymNode = mutable_Root().get().mutable_Tree().get().mutable_Nyms();

    if (false == nymNode.get().Exists(nym.str())) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nym)(" doesn't exist.").Flush();

        return false;
    }

    return nymNode.get().mutable_Nym(nym.str()).get().Store(purse);
}

auto Storage::Store(const proto::Seed& data, const std::string& alias) const
    -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Seeds()
        .get()
        .Store(data, alias);
}

auto Storage::Store(const proto::ServerContract& data, const std::string& alias)
    const -> bool
{
    auto storageVersion(data);
    storageVersion.clear_publicnym();
    std::string plaintext;
    const bool saved =
        mutable_Root().get().mutable_Tree().get().mutable_Servers().get().Store(
            data, alias, plaintext);

    if (saved) {
        if (config_.auto_publish_servers_ && config_.dht_callback_) {
            config_.dht_callback_(storageVersion.id(), plaintext);
        }

        return true;
    }

    return false;
}

auto Storage::Store(const proto::Ciphertext& serialized) const -> bool
{
    return mutable_Root().get().mutable_Tree().get().Store(serialized);
}

auto Storage::Store(const proto::UnitDefinition& data, const std::string& alias)
    const -> bool
{
    auto storageVersion(data);
    storageVersion.clear_issuer_nym();
    std::string plaintext;
    const bool saved =
        mutable_Root().get().mutable_Tree().get().mutable_Units().get().Store(
            data, alias, plaintext);

    if (saved) {
        if (config_.auto_publish_units_ && config_.dht_callback_) {
            config_.dht_callback_(storageVersion.id(), plaintext);
        }

        return true;
    }

    return false;
}

auto Storage::ThreadList(const std::string& nymID, const bool unreadOnly) const
    -> ObjectList
{
    return Root().Tree().Nyms().Nym(nymID).Threads().List(unreadOnly);
}

auto Storage::ThreadAlias(const std::string& nymID, const std::string& threadID)
    const -> std::string
{
    return Root().Tree().Nyms().Nym(nymID).Threads().Thread(threadID).Alias();
}

auto Storage::UnaffiliatedBlockchainTransaction(
    const identifier::Nym& nym,
    const Data& txid) const noexcept -> bool
{
    static const auto blank = Identifier::Factory();

    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nym.str())
        .get()
        .mutable_Threads()
        .get()
        .AddIndex(txid, blank);
}

auto Storage::UnitDefinitionAlias(const std::string& id) const -> std::string
{
    return Root().Tree().Units().Alias(id);
}

auto Storage::UnitDefinitionList() const -> ObjectList
{
    return Root().Tree().Units().List();
}

auto Storage::UnreadCount(const std::string& nymId, const std::string& threadId)
    const -> std::size_t
{
    auto& nyms = Root().Tree().Nyms();

    if (false == nyms.Exists(nymId)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymId)(" does not exist.")
            .Flush();

        return 0;
    }

    auto& threads = nyms.Nym(nymId).Threads();

    if (false == threads.Exists(threadId)) {
        LogError()(OT_PRETTY_CLASS())("Thread ")(threadId)(" does not exist.")
            .Flush();

        return 0;
    }

    return threads.Thread(threadId).UnreadCount();
}

void Storage::UpgradeNyms()
{
    const auto level = Root().Tree().Nyms().UpgradeLevel();

    if (3 > level) {
        LogError()(OT_PRETTY_CLASS())("Beginning upgrade for version ")(
            level)(".")
            .Flush();
        mutable_Root()
            .get()
            .mutable_Tree()
            .get()
            .mutable_Nyms()
            .get()
            .UpgradeLocalnym();
    } else {
        LogVerbose()(OT_PRETTY_CLASS())("No need to upgrade version ")(level)
            .Flush();
    }
}

auto Storage::verify_write_lock(const Lock& lock) const -> bool
{
    if (lock.mutex() != &write_lock_) {
        LogError()(OT_PRETTY_CLASS())("Incorrect mutex.").Flush();

        return false;
    }

    if (false == lock.owns_lock()) {
        LogError()(OT_PRETTY_CLASS())("Lock not owned.").Flush();

        return false;
    }

    return true;
}

Storage::~Storage() { Cleanup_Storage(); }
}  // namespace opentxs::api::session::imp

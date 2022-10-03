// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"             // IWYU pragma: associated
#include "api/session/Storage.hpp"  // IWYU pragma: associated

#include <Bip47Channel.pb.h>
#include <Ciphertext.pb.h>
#include <Contact.pb.h>
#include <Context.pb.h>
#include <Credential.pb.h>
#include <HDAccount.pb.h>
#include <Issuer.pb.h>
#include <Nym.pb.h>
#include <PaymentWorkflow.pb.h>
#include <PeerReply.pb.h>
#include <PeerRequest.pb.h>
#include <Purse.pb.h>
#include <Seed.pb.h>
#include <ServerContract.pb.h>
#include <StorageThread.pb.h>
#include <StorageThreadItem.pb.h>
#include <UnitDefinition.pb.h>
#include <cstdint>
#include <ctime>
#include <functional>
#include <limits>
#include <stdexcept>
#include <utility>

#include "Proto.hpp"
#include "Proto.tpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Factory.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/storage/drivers/Drivers.hpp"
#include "internal/util/storage/drivers/Factory.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "otx/common/OTStorage.hpp"
#include "util/storage/Config.hpp"
#include "util/storage/tree/Accounts.hpp"
#include "util/storage/tree/Bip47Channels.hpp"
#include "util/storage/tree/Contacts.hpp"
#include "util/storage/tree/Contexts.hpp"
#include "util/storage/tree/Credentials.hpp"
#include "util/storage/tree/Issuers.hpp"
#include "util/storage/tree/Mailbox.hpp"
#include "util/storage/tree/Notary.hpp"
#include "util/storage/tree/Nym.hpp"
#include "util/storage/tree/Nyms.hpp"
#include "util/storage/tree/PaymentWorkflows.hpp"
#include "util/storage/tree/PeerReplies.hpp"
#include "util/storage/tree/PeerRequests.hpp"
#include "util/storage/tree/Root.hpp"
#include "util/storage/tree/Seeds.hpp"
#include "util/storage/tree/Servers.hpp"
#include "util/storage/tree/Thread.hpp"
#include "util/storage/tree/Threads.hpp"
#include "util/storage/tree/Tree.hpp"
#include "util/storage/tree/Units.hpp"

namespace opentxs::factory
{
auto StorageAPI(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Factory& factory,
    const Flag& running,
    const opentxs::storage::Config& config) noexcept
    -> std::unique_ptr<api::session::Storage>
{
    {
        auto otdb =
            std::unique_ptr<OTDB::StorageFS>{OTDB::StorageFS::Instantiate()};
    }

    return std::make_unique<api::session::imp::Storage>(
        crypto, asio, factory, running, config);
}
}  // namespace opentxs::factory

namespace opentxs::api::session::imp
{
const std::uint32_t Storage::HASH_TYPE = 2;  // BTC160

Storage::Storage(
    const api::Crypto& crypto,
    const network::Asio& asio,
    const api::session::Factory& factory,
    const Flag& running,
    const opentxs::storage::Config& config)
    : crypto_(crypto)
    , factory_(factory)
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
          factory_,
          *this,
          primary_bucket_,
          config_))
    , multiplex_(*multiplex_p_)
{
    OT_ASSERT(multiplex_p_);
}

auto Storage::AccountAlias(const identifier::Generic& accountID) const
    -> UnallocatedCString
{
    return Root().Tree().Accounts().Alias(accountID.asBase58(crypto_));
}

auto Storage::AccountList() const -> ObjectList
{
    return Root().Tree().Accounts().List();
}

auto Storage::AccountContract(const identifier::Generic& accountID) const
    -> identifier::UnitDefinition
{
    return Root().Tree().Accounts().AccountContract(accountID);
}

auto Storage::AccountIssuer(const identifier::Generic& accountID) const
    -> identifier::Nym
{
    return Root().Tree().Accounts().AccountIssuer(accountID);
}

auto Storage::AccountOwner(const identifier::Generic& accountID) const
    -> identifier::Nym
{
    return Root().Tree().Accounts().AccountOwner(accountID);
}

auto Storage::AccountServer(const identifier::Generic& accountID) const
    -> identifier::Notary
{
    return Root().Tree().Accounts().AccountServer(accountID);
}

auto Storage::AccountSigner(const identifier::Generic& accountID) const
    -> identifier::Nym
{
    return Root().Tree().Accounts().AccountSigner(accountID);
}

auto Storage::AccountUnit(const identifier::Generic& accountID) const
    -> UnitType
{
    return Root().Tree().Accounts().AccountUnit(accountID);
}

auto Storage::AccountsByContract(const identifier::UnitDefinition& contract)
    const -> UnallocatedSet<identifier::Generic>
{
    return Root().Tree().Accounts().AccountsByContract(contract);
}

auto Storage::AccountsByIssuer(const identifier::Nym& issuerNym) const
    -> UnallocatedSet<identifier::Generic>
{
    return Root().Tree().Accounts().AccountsByIssuer(issuerNym);
}

auto Storage::AccountsByOwner(const identifier::Nym& ownerNym) const
    -> UnallocatedSet<identifier::Generic>
{
    return Root().Tree().Accounts().AccountsByOwner(ownerNym);
}

auto Storage::AccountsByServer(const identifier::Notary& server) const
    -> UnallocatedSet<identifier::Generic>
{
    return Root().Tree().Accounts().AccountsByServer(server);
}

auto Storage::AccountsByUnit(const UnitType unit) const
    -> UnallocatedSet<identifier::Generic>
{
    return Root().Tree().Accounts().AccountsByUnit(unit);
}

auto Storage::Bip47Chain(
    const identifier::Nym& nymID,
    const identifier::Generic& channelID) const -> UnitType
{
    const bool exists = Root().Tree().Nyms().Exists(nymID);

    if (false == exists) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return UnitType::Error;
    }

    return Root().Tree().Nyms().Nym(nymID).Bip47Channels().Chain(channelID);
}

auto Storage::Bip47ChannelsByChain(
    const identifier::Nym& nymID,
    const UnitType chain) const -> Storage::Bip47ChannelList
{
    const bool exists = Root().Tree().Nyms().Exists(nymID);

    if (false == exists) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Tree().Nyms().Nym(nymID).Bip47Channels().ChannelsByChain(
        chain);
}

auto Storage::blockchain_thread_item_id(
    const opentxs::blockchain::Type chain,
    const Data& txid) const noexcept -> UnallocatedCString
{
    return opentxs::blockchain_thread_item_id(crypto_, factory_, chain, txid)
        .asBase58(crypto_);
}

auto Storage::BlockchainAccountList(
    const identifier::Nym& nymID,
    const UnitType type) const -> UnallocatedSet<UnallocatedCString>
{
    return Root().Tree().Nyms().Nym(nymID).BlockchainAccountList(type);
}

auto Storage::BlockchainSubaccountAccountType(
    const identifier::Nym& owner,
    const identifier::Generic& subaccount) const -> UnitType
{
    const auto& nym = Root().Tree().Nyms().Nym(owner);
    auto out = nym.BlockchainAccountType(subaccount.asBase58(crypto_));

    if (UnitType::Error == out) { out = nym.Bip47Channels().Chain(subaccount); }

    return out;
}

auto Storage::BlockchainThreadMap(const identifier::Nym& nym, const Data& txid)
    const noexcept -> UnallocatedVector<identifier::Generic>
{
    const auto& nyms = Root().Tree().Nyms();

    if (false == nyms.Exists(nym)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nym)(" does not exist.").Flush();

        return {};
    }

    return nyms.Nym(nym).Threads().BlockchainThreadMap(txid);
}

auto Storage::BlockchainTransactionList(
    const identifier::Nym& nym) const noexcept -> UnallocatedVector<ByteArray>
{
    const auto& nyms = Root().Tree().Nyms();

    if (false == nyms.Exists(nym)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nym)(" does not exist.").Flush();

        return {};
    }

    return nyms.Nym(nym).Threads().BlockchainTransactionList();
}

auto Storage::CheckTokenSpent(
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit,
    const std::uint64_t series,
    const UnallocatedCString& key) const -> bool
{
    return Root()
        .Tree()
        .Notary(notary.asBase58(crypto_))
        .CheckSpent(unit, series, key);
}

void Storage::Cleanup_Storage()
{
    if (root_) { root_->cleanup(); }
}

void Storage::Cleanup() { Cleanup_Storage(); }

void Storage::CollectGarbage() const { Root().Migrate(multiplex_.Primary()); }

auto Storage::ContactAlias(const UnallocatedCString& id) const
    -> UnallocatedCString
{
    return Root().Tree().Contacts().Alias(id);
}

auto Storage::ContactList() const -> ObjectList
{
    return Root().Tree().Contacts().List();
}

auto Storage::ContactOwnerNym(const identifier::Nym& nym) const
    -> identifier::Generic
{
    return Root().Tree().Contacts().NymOwner(nym);
}

void Storage::ContactSaveIndices() const
{
    mutable_Root().get().mutable_Tree().get().mutable_Contacts().get().Save();
}

auto Storage::ContactUpgradeLevel() const -> VersionNumber
{
    return Root().Tree().Contacts().UpgradeLevel();
}

auto Storage::ContextList(const identifier::Nym& nymID) const -> ObjectList
{
    return Root().Tree().Nyms().Nym(nymID).Contexts().List();
}

auto Storage::CreateThread(
    const identifier::Nym& nymID,
    const UnallocatedCString& threadID,
    const UnallocatedSet<UnallocatedCString>& participants) const -> bool
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

auto Storage::DefaultNym() const -> identifier::Nym
{
    return Root().Tree().Nyms().Default();
}

auto Storage::DefaultSeed() const -> UnallocatedCString
{
    return Root().Tree().Seeds().Default();
}

auto Storage::DeleteAccount(const UnallocatedCString& id) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Accounts()
        .get()
        .Delete(id);
}

auto Storage::DeleteContact(const UnallocatedCString& id) const -> bool
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
    const identifier::Nym& nymID,
    const UnallocatedCString& workflowID) const -> bool
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
        crypto_,
        factory_,
        multiplex_,
        hash,
        std::numeric_limits<std::int64_t>::max(),
        primary_bucket_)};

    OT_ASSERT(root);

    multiplex_.SynchronizePlugins(hash, *root, syncPrimary);
}

auto Storage::IssuerList(const identifier::Nym& nymID) const -> ObjectList
{
    const bool exists = Root().Tree().Nyms().Exists(nymID);

    if (false == exists) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Tree().Nyms().Nym(nymID).Issuers().List();
}

auto Storage::Load(
    const UnallocatedCString& accountID,
    UnallocatedCString& output,
    UnallocatedCString& alias,
    const bool checking) const -> bool
{
    return Root().Tree().Accounts().Load(accountID, output, alias, checking);
}

auto Storage::Load(
    const identifier::Nym& nymID,
    const UnallocatedCString& accountID,
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
    const identifier::Generic& channelID,
    proto::Bip47Channel& output,
    const bool checking) const -> bool
{
    const bool exists = Root().Tree().Nyms().Exists(nymID);

    if (false == exists) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return false;
    }

    auto temp = std::make_shared<proto::Bip47Channel>(output);
    const auto rc = Root().Tree().Nyms().Nym(nymID).Bip47Channels().Load(
        channelID, temp, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const UnallocatedCString& id,
    proto::Contact& output,
    const bool checking) const -> bool
{
    auto notUsed = UnallocatedCString{};

    return Load(id, output, notUsed, checking);
}

auto Storage::Load(
    const UnallocatedCString& id,
    proto::Contact& output,
    UnallocatedCString& alias,
    const bool checking) const -> bool
{
    auto temp = std::make_shared<proto::Contact>(output);
    const auto rc = Root().Tree().Contacts().Load(id, temp, alias, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::Nym& localNym,
    const identifier::Nym& remoteNym,
    proto::Context& output,
    const bool checking) const -> bool
{
    auto notUsed = UnallocatedCString{};
    auto temp = std::make_shared<proto::Context>(output);
    const auto rc = Root().Tree().Nyms().Nym(localNym).Contexts().Load(
        remoteNym, temp, notUsed, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const UnallocatedCString& id,
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
    auto notUsed = UnallocatedCString{};

    return Load(id, output, notUsed, checking);
}

auto Storage::LoadNym(
    const identifier::Nym& id,
    AllocateOutput destination,
    const bool checking) const -> bool
{
    auto temp = std::make_shared<proto::Nym>();
    auto alias = UnallocatedCString{};

    if (false == Root().Tree().Nyms().Nym(id).Load(temp, alias, checking)) {
        LogError()(OT_PRETTY_CLASS())("Failed to load nym ")(id).Flush();

        return false;
    }

    OT_ASSERT(temp);

    return write(*temp, destination);
}

auto Storage::Load(
    const identifier::Nym& id,
    proto::Nym& output,
    UnallocatedCString& alias,
    const bool checking) const -> bool
{
    auto temp = std::make_shared<proto::Nym>(output);
    const auto rc = Root().Tree().Nyms().Nym(id).Load(temp, alias, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::Nym& nymID,
    const UnallocatedCString& id,
    proto::Issuer& output,
    const bool checking) const -> bool
{
    if (false == Root().Tree().Nyms().Exists(nymID)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return false;
    }

    auto notUsed = UnallocatedCString{};
    auto temp = std::make_shared<proto::Issuer>(output);
    const auto rc = Root().Tree().Nyms().Nym(nymID).Issuers().Load(
        id, temp, notUsed, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::Nym& nymID,
    const UnallocatedCString& workflowID,
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
    const identifier::Nym& nymID,
    const UnallocatedCString& id,
    const otx::client::StorageBox box,
    UnallocatedCString& output,
    UnallocatedCString& alias,
    const bool checking) const -> bool
{
    switch (box) {
        case otx::client::StorageBox::MAILINBOX: {
            return Root().Tree().Nyms().Nym(nymID).MailInbox().Load(
                id, output, alias, checking);
        }
        case otx::client::StorageBox::MAILOUTBOX: {
            return Root().Tree().Nyms().Nym(nymID).MailOutbox().Load(
                id, output, alias, checking);
        }
        default: {
            return false;
        }
    }
}

auto Storage::Load(
    const identifier::Nym& nymID,
    const UnallocatedCString& id,
    const otx::client::StorageBox box,
    proto::PeerReply& output,
    const bool checking) const -> bool
{
    auto temp = std::make_shared<proto::PeerReply>(output);
    const auto rc = [&] {
        switch (box) {
            case otx::client::StorageBox::SENTPEERREPLY: {
                return Root().Tree().Nyms().Nym(nymID).SentReplyBox().Load(
                    id, temp, checking);
            }
            case otx::client::StorageBox::INCOMINGPEERREPLY: {
                return Root().Tree().Nyms().Nym(nymID).IncomingReplyBox().Load(
                    id, temp, checking);
            }
            case otx::client::StorageBox::FINISHEDPEERREPLY: {
                return Root().Tree().Nyms().Nym(nymID).FinishedReplyBox().Load(
                    id, temp, checking);
            }
            case otx::client::StorageBox::PROCESSEDPEERREPLY: {
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
    const identifier::Nym& nymID,
    const UnallocatedCString& id,
    const otx::client::StorageBox box,
    proto::PeerRequest& output,
    std::time_t& time,
    const bool checking) const -> bool
{
    auto temp = std::make_shared<proto::PeerRequest>(output);
    auto alias = UnallocatedCString{};
    const auto rc = [&] {
        switch (box) {
            case otx::client::StorageBox::SENTPEERREQUEST: {
                return Root().Tree().Nyms().Nym(nymID).SentRequestBox().Load(
                    id, temp, alias, checking);
            }
            case otx::client::StorageBox::INCOMINGPEERREQUEST: {
                return Root()
                    .Tree()
                    .Nyms()
                    .Nym(nymID)
                    .IncomingRequestBox()
                    .Load(id, temp, alias, checking);
            }
            case otx::client::StorageBox::FINISHEDPEERREQUEST: {
                return Root()
                    .Tree()
                    .Nyms()
                    .Nym(nymID)
                    .FinishedRequestBox()
                    .Load(id, temp, alias, checking);
            }
            case otx::client::StorageBox::PROCESSEDPEERREQUEST: {
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
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit,
    proto::Purse& output,
    const bool checking) const -> bool
{
    const auto& nymNode = Root().Tree().Nyms();

    if (false == nymNode.Exists(nym)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nym)(" doesn't exist.").Flush();

        return false;
    }

    auto temp = std::make_shared<proto::Purse>(output);
    const auto rc = nymNode.Nym(nym).Load(notary, unit, temp, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const UnallocatedCString& id,
    proto::Seed& output,
    const bool checking) const -> bool
{
    auto notUsed = UnallocatedCString{};

    return Load(id, output, notUsed, checking);
}

auto Storage::Load(
    const UnallocatedCString& id,
    proto::Seed& output,
    UnallocatedCString& alias,
    const bool checking) const -> bool
{
    auto temp = std::make_shared<proto::Seed>(output);
    const auto rc = Root().Tree().Seeds().Load(id, temp, alias, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::Notary& id,
    proto::ServerContract& output,
    const bool checking) const -> bool
{
    auto notUsed = UnallocatedCString{};

    return Load(id, output, notUsed, checking);
}

auto Storage::Load(
    const identifier::Notary& id,
    proto::ServerContract& output,
    UnallocatedCString& alias,
    const bool checking) const -> bool
{
    auto temp = std::make_shared<proto::ServerContract>(output);
    const auto rc = Root().Tree().Servers().Load(
        id.asBase58(crypto_), temp, alias, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::Nym& nymId,
    const UnallocatedCString& threadId,
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
    auto notUsed = UnallocatedCString{};

    return Load(id, output, notUsed, checking);
}

auto Storage::Load(
    const identifier::UnitDefinition& id,
    proto::UnitDefinition& output,
    UnallocatedCString& alias,
    const bool checking) const -> bool
{
    auto temp = std::make_shared<proto::UnitDefinition>(output);
    const auto rc =
        Root().Tree().Units().Load(id.asBase58(crypto_), temp, alias, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::LocalNyms() const -> Set<identifier::Nym>
{
    return Root().Tree().Nyms().LocalNyms();
}

// Applies a lambda to all public nyms in the database in a detached thread.
void Storage::MapPublicNyms(NymLambda& cb) const
{
    asio_.Internal().Post(
        ThreadPool::General,
        [=, this] { RunMapPublicNyms(cb); },
        "Storage Nyms");
}

// Applies a lambda to all server contracts in the database in a detached
// thread.
void Storage::MapServers(ServerLambda& cb) const
{
    asio_.Internal().Post(
        ThreadPool::General,
        [=, this] { RunMapServers(cb); },
        "Storage Servers");
}

// Applies a lambda to all unit definitions in the database in a detached
// thread.
void Storage::MapUnitDefinitions(UnitLambda& cb) const
{
    asio_.Internal().Post(
        ThreadPool::General, [=, this] { RunMapUnits(cb); }, "Storage Units");
}

auto Storage::MarkTokenSpent(
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit,
    const std::uint64_t series,
    const UnallocatedCString& key) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Notary(notary.asBase58(crypto_))
        .get()
        .MarkSpent(unit, series, key);
}

auto Storage::MoveThreadItem(
    const identifier::Nym& nymId,
    const UnallocatedCString& fromThreadID,
    const UnallocatedCString& toThreadID,
    const UnallocatedCString& itemID) const -> bool
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
    auto box = otx::client::StorageBox{};
    const auto alias = UnallocatedCString{};
    const auto contents = UnallocatedCString{};
    auto index = std::uint64_t{};
    auto account = UnallocatedCString{};

    for (const auto& item : thread.item()) {
        if (item.id() == itemID) {
            found = true;
            time = item.time();
            box = static_cast<otx::client::StorageBox>(item.box());
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

    return {write_lock_, root(), callback};
}

auto Storage::NymBoxList(
    const identifier::Nym& nymID,
    const otx::client::StorageBox box) const -> ObjectList
{
    switch (box) {
        case otx::client::StorageBox::SENTPEERREQUEST: {
            return Root().Tree().Nyms().Nym(nymID).SentRequestBox().List();
        }
        case otx::client::StorageBox::INCOMINGPEERREQUEST: {
            return Root().Tree().Nyms().Nym(nymID).IncomingRequestBox().List();
        }
        case otx::client::StorageBox::SENTPEERREPLY: {
            return Root().Tree().Nyms().Nym(nymID).SentReplyBox().List();
        }
        case otx::client::StorageBox::INCOMINGPEERREPLY: {
            return Root().Tree().Nyms().Nym(nymID).IncomingReplyBox().List();
        }
        case otx::client::StorageBox::FINISHEDPEERREQUEST: {
            return Root().Tree().Nyms().Nym(nymID).FinishedRequestBox().List();
        }
        case otx::client::StorageBox::FINISHEDPEERREPLY: {
            return Root().Tree().Nyms().Nym(nymID).FinishedReplyBox().List();
        }
        case otx::client::StorageBox::PROCESSEDPEERREQUEST: {
            return Root().Tree().Nyms().Nym(nymID).ProcessedRequestBox().List();
        }
        case otx::client::StorageBox::PROCESSEDPEERREPLY: {
            return Root().Tree().Nyms().Nym(nymID).ProcessedReplyBox().List();
        }
        case otx::client::StorageBox::MAILINBOX: {
            return Root().Tree().Nyms().Nym(nymID).MailInbox().List();
        }
        case otx::client::StorageBox::MAILOUTBOX: {
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

auto Storage::PaymentWorkflowList(const identifier::Nym& nymID) const
    -> ObjectList
{
    if (false == Root().Tree().Nyms().Exists(nymID)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Tree().Nyms().Nym(nymID).PaymentWorkflows().List();
}

auto Storage::PaymentWorkflowLookup(
    const identifier::Nym& nymID,
    const UnallocatedCString& sourceID) const -> UnallocatedCString
{
    if (false == Root().Tree().Nyms().Exists(nymID)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Tree().Nyms().Nym(nymID).PaymentWorkflows().LookupBySource(
        sourceID);
}

auto Storage::PaymentWorkflowsByAccount(
    const identifier::Nym& nymID,
    const UnallocatedCString& accountID) const
    -> UnallocatedSet<UnallocatedCString>
{
    if (false == Root().Tree().Nyms().Exists(nymID)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Tree().Nyms().Nym(nymID).PaymentWorkflows().ListByAccount(
        accountID);
}

auto Storage::PaymentWorkflowsByState(
    const identifier::Nym& nymID,
    const otx::client::PaymentWorkflowType type,
    const otx::client::PaymentWorkflowState state) const
    -> UnallocatedSet<UnallocatedCString>
{
    if (false == Root().Tree().Nyms().Exists(nymID)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Tree().Nyms().Nym(nymID).PaymentWorkflows().ListByState(
        type, state);
}

auto Storage::PaymentWorkflowsByUnit(
    const identifier::Nym& nymID,
    const UnallocatedCString& unitID) const
    -> UnallocatedSet<UnallocatedCString>
{
    if (false == Root().Tree().Nyms().Exists(nymID)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Tree().Nyms().Nym(nymID).PaymentWorkflows().ListByUnit(
        unitID);
}

auto Storage::PaymentWorkflowState(
    const identifier::Nym& nymID,
    const UnallocatedCString& workflowID) const -> std::
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
    const UnallocatedCString& threadID,
    const UnallocatedCString& label) const -> bool
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
    const identifier::Generic& threadID,
    const opentxs::blockchain::Type chain,
    const Data& txid) const noexcept -> bool
{
    const auto& nyms = Root().Tree().Nyms();

    if (false == nyms.Exists(nym)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nym)(" does not exist.").Flush();

        return false;
    }

    const auto& threads = nyms.Nym(nym).Threads();

    if (false == threads.Exists(threadID.asBase58(crypto_))) {
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
                           .mutable_Nym(nym)
                           .get()
                           .mutable_Threads(txid, threadID, false)
                           .get()
                           .mutable_Thread(threadID.asBase58(crypto_))
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
    const identifier::Nym& nymID,
    const otx::client::StorageBox box,
    const UnallocatedCString& itemID) const -> bool
{
    switch (box) {
        case otx::client::StorageBox::SENTPEERREQUEST: {
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
        case otx::client::StorageBox::INCOMINGPEERREQUEST: {
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
        case otx::client::StorageBox::SENTPEERREPLY: {
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
        case otx::client::StorageBox::INCOMINGPEERREPLY: {
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
        case otx::client::StorageBox::FINISHEDPEERREQUEST: {
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
        case otx::client::StorageBox::FINISHEDPEERREPLY: {
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
        case otx::client::StorageBox::PROCESSEDPEERREQUEST: {
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
        case otx::client::StorageBox::PROCESSEDPEERREPLY: {
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
        case otx::client::StorageBox::MAILINBOX: {
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
        case otx::client::StorageBox::MAILOUTBOX: {
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

auto Storage::RemoveServer(const UnallocatedCString& id) const -> bool
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
    const identifier::Generic& threadID,
    const UnallocatedCString& id) const -> bool
{
    const auto& nyms = Root().Tree().Nyms();

    if (false == nyms.Exists(nym)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nym)(" does not exist.").Flush();

        return false;
    }

    const auto& threads = nyms.Nym(nym).Threads();

    if (false == threads.Exists(threadID.asBase58(crypto_))) {
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
                           .mutable_Nym(nym)
                           .get()
                           .mutable_Threads()
                           .get()
                           .mutable_Thread(threadID.asBase58(crypto_))
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

auto Storage::RemoveUnitDefinition(const UnallocatedCString& id) const -> bool
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
    const identifier::Nym& nymId,
    const UnallocatedCString& threadId,
    const UnallocatedCString& newID) const -> bool
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
            crypto_,
            factory_,
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

auto Storage::SetAccountAlias(
    const UnallocatedCString& id,
    const UnallocatedCString& alias) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Accounts()
        .get()
        .SetAlias(id, alias);
}

auto Storage::SetContactAlias(
    const UnallocatedCString& id,
    const UnallocatedCString& alias) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Contacts()
        .get()
        .SetAlias(id, alias);
}

auto Storage::SetDefaultNym(const identifier::Nym& id) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Nyms()
        .get()
        .SetDefault(id);
}

auto Storage::SetDefaultSeed(const UnallocatedCString& id) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Seeds()
        .get()
        .SetDefault(id);
}

auto Storage::SetNymAlias(
    const identifier::Nym& id,
    const UnallocatedCString& alias) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(id)
        .get()
        .SetAlias(alias);
}

auto Storage::SetPeerRequestTime(
    const identifier::Nym& nymID,
    const UnallocatedCString& id,
    const otx::client::StorageBox box) const -> bool
{
    const UnallocatedCString now = std::to_string(time(nullptr));

    switch (box) {
        case otx::client::StorageBox::SENTPEERREQUEST: {
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
        case otx::client::StorageBox::INCOMINGPEERREQUEST: {
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
        case otx::client::StorageBox::FINISHEDPEERREQUEST: {
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
        case otx::client::StorageBox::PROCESSEDPEERREQUEST: {
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
    const identifier::Nym& nymId,
    const UnallocatedCString& threadId,
    const UnallocatedCString& itemId,
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

auto Storage::SetSeedAlias(
    const UnallocatedCString& id,
    const UnallocatedCString& alias) const -> bool
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
    const identifier::Notary& id,
    const UnallocatedCString& alias) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Servers()
        .get()
        .SetAlias(id.asBase58(crypto_), alias);
}

auto Storage::SetThreadAlias(
    const identifier::Nym& nymId,
    const UnallocatedCString& threadId,
    const UnallocatedCString& alias) const -> bool
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
    const UnallocatedCString& alias) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Units()
        .get()
        .SetAlias(id.asBase58(crypto_), alias);
}

auto Storage::ServerAlias(const UnallocatedCString& id) const
    -> UnallocatedCString
{
    return Root().Tree().Servers().Alias(id);
}

auto Storage::ServerList() const -> ObjectList
{
    return Root().Tree().Servers().List();
}

void Storage::start() { InitPlugins(); }

auto Storage::Store(
    const UnallocatedCString& accountID,
    const UnallocatedCString& data,
    const UnallocatedCString& alias,
    const identifier::Nym& ownerNym,
    const identifier::Nym& signerNym,
    const identifier::Nym& issuerNym,
    const identifier::Notary& server,
    const identifier::UnitDefinition& contract,
    const UnitType unit) const -> bool
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
    const identifier::Nym& nymID,
    const identity::wot::claim::ClaimType type,
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
    const identifier::Generic& channelID,
    const proto::Bip47Channel& data) const -> bool
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
    auto notUsed = UnallocatedCString{};

    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(factory_.NymIDFromBase58(data.localnym()))
        .get()
        .mutable_Contexts()
        .get()
        .Store(data, notUsed);
}

auto Storage::Store(const proto::Credential& data) const -> bool
{
    auto notUsed = UnallocatedCString{};

    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Credentials()
        .get()
        .Store(data, notUsed);
}

auto Storage::Store(const proto::Nym& data, const UnallocatedCString& alias)
    const -> bool
{
    auto plaintext = UnallocatedCString{};
    const auto& id = data.nymid();

    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(factory_.NymIDFromBase58(id))
        .get()
        .Store(data, alias, plaintext);
}

auto Storage::Store(const ReadView& view, const UnallocatedCString& alias) const
    -> bool
{
    return Store(proto::Factory<proto::Nym>(view), alias);
}

auto Storage::Store(const identifier::Nym& nymID, const proto::Issuer& data)
    const -> bool
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
    const identifier::Nym& nymID,
    const proto::PaymentWorkflow& data) const -> bool
{
    const bool exists = Root().Tree().Nyms().Exists(nymID);

    if (false == exists) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID)(" doesn't exist.").Flush();

        return false;
    }

    UnallocatedCString notUsed{};

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
    const identifier::Nym& nymid,
    const UnallocatedCString& threadid,
    const UnallocatedCString& itemid,
    const std::uint64_t time,
    const UnallocatedCString& alias,
    const UnallocatedCString& data,
    const otx::client::StorageBox box,
    const UnallocatedCString& account) const -> bool
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
    const identifier::Generic& thread,
    const opentxs::blockchain::Type chain,
    const Data& txid,
    const Time time) const noexcept -> bool
{
    const auto alias = UnallocatedCString{};
    const auto account = UnallocatedCString{};
    const auto id = blockchain_thread_item_id(chain, txid);

    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nym)
        .get()
        .mutable_Threads(txid, thread, true)
        .get()
        .mutable_Thread(thread.asBase58(crypto_))
        .get()
        .Add(
            id,
            Clock::to_time_t(time),
            otx::client::StorageBox::BLOCKCHAIN,
            alias,
            UnallocatedCString{txid.Bytes()},
            0,
            account,
            static_cast<std::uint32_t>(chain));
}

auto Storage::Store(
    const proto::PeerReply& data,
    const identifier::Nym& nymID,
    const otx::client::StorageBox box) const -> bool
{
    switch (box) {
        case otx::client::StorageBox::SENTPEERREPLY: {
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
        case otx::client::StorageBox::INCOMINGPEERREPLY: {
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
        case otx::client::StorageBox::FINISHEDPEERREPLY: {
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
        case otx::client::StorageBox::PROCESSEDPEERREPLY: {
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
    const identifier::Nym& nymID,
    const otx::client::StorageBox box) const -> bool
{
    // Use the alias field to store the time at which the request was saved.
    // Useful for managing retry logic in the high level api
    const UnallocatedCString now = std::to_string(time(nullptr));

    switch (box) {
        case otx::client::StorageBox::SENTPEERREQUEST: {
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
        case otx::client::StorageBox::INCOMINGPEERREQUEST: {
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
        case otx::client::StorageBox::FINISHEDPEERREQUEST: {
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
        case otx::client::StorageBox::PROCESSEDPEERREQUEST: {
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

    if (false == nymNode.get().Exists(nym)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nym)(" doesn't exist.").Flush();

        return false;
    }

    return nymNode.get().mutable_Nym(nym).get().Store(purse);
}

auto Storage::Store(const proto::Seed& data) const -> bool
{
    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Seeds()
        .get()
        .Store(data);
}

auto Storage::Store(
    const proto::ServerContract& data,
    const UnallocatedCString& alias) const -> bool
{
    auto storageVersion(data);
    storageVersion.clear_publicnym();
    UnallocatedCString plaintext;

    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Servers()
        .get()
        .Store(data, alias, plaintext);
}

auto Storage::Store(const proto::Ciphertext& serialized) const -> bool
{
    return mutable_Root().get().mutable_Tree().get().Store(serialized);
}

auto Storage::Store(
    const proto::UnitDefinition& data,
    const UnallocatedCString& alias) const -> bool
{
    auto storageVersion(data);
    storageVersion.clear_issuer_nym();
    UnallocatedCString plaintext;

    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Units()
        .get()
        .Store(data, alias, plaintext);
}

auto Storage::ThreadList(const identifier::Nym& nymID, const bool unreadOnly)
    const -> ObjectList
{
    return Root().Tree().Nyms().Nym(nymID).Threads().List(unreadOnly);
}

auto Storage::ThreadAlias(
    const identifier::Nym& nymID,
    const UnallocatedCString& threadID) const -> UnallocatedCString
{
    return Root().Tree().Nyms().Nym(nymID).Threads().Thread(threadID).Alias();
}

auto Storage::UnaffiliatedBlockchainTransaction(
    const identifier::Nym& nym,
    const Data& txid) const noexcept -> bool
{
    static const auto blank = identifier::Generic{};

    return mutable_Root()
        .get()
        .mutable_Tree()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nym)
        .get()
        .mutable_Threads()
        .get()
        .AddIndex(txid, blank);
}

auto Storage::UnitDefinitionAlias(const UnallocatedCString& id) const
    -> UnallocatedCString
{
    return Root().Tree().Units().Alias(id);
}

auto Storage::UnitDefinitionList() const -> ObjectList
{
    return Root().Tree().Units().List();
}

auto Storage::UnreadCount(
    const identifier::Nym& nymId,
    const UnallocatedCString& threadId) const -> std::size_t
{
    const auto& nyms = Root().Tree().Nyms();

    if (false == nyms.Exists(nymId)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymId)(" does not exist.")
            .Flush();

        return 0;
    }

    const auto& threads = nyms.Nym(nymId).Threads();

    if (false == threads.Exists(threadId)) {
        LogError()(OT_PRETTY_CLASS())("Thread ")(threadId)(" does not exist.")
            .Flush();

        return 0;
    }

    return threads.Thread(threadId).UnreadCount();
}

auto Storage::Upgrade() noexcept -> void
{
    if (Root().Tree().Nyms().NeedUpgrade()) {
        mutable_Root()
            .get()
            .mutable_Tree()
            .get()
            .mutable_Nyms()
            .get()
            .Upgrade();
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

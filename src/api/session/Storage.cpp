// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::session::Storage

#include "api/session/Storage.hpp"  // IWYU pragma: associated

#include <Bip47Channel.pb.h>
#include <BlockchainEthereumAccountData.pb.h>
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
#include <memory>
#include <utility>

#include "internal/blockchain/crypto/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/Editor.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "internal/util/storage/drivers/Factory.hpp"
#include "internal/util/storage/drivers/Plugin.hpp"
#include "internal/util/storage/tree/Types.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"  // IWYU pragma: keep
#include "opentxs/api/session/internal.factory.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/UnitType.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"
#include "otx/common/OTStorage.hpp"
#include "util/storage/Config.hpp"
#include "util/storage/tree/Accounts.hpp"
#include "util/storage/tree/Actor.hpp"
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
#include "util/storage/tree/Trunk.hpp"
#include "util/storage/tree/Units.hpp"

namespace opentxs::factory
{
auto StorageAPI(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const opentxs::storage::Config& config) noexcept
    -> std::shared_ptr<api::session::Storage>
{
    {
        auto otdb =
            std::unique_ptr<OTDB::StorageFS>{OTDB::StorageFS::Instantiate()};
    }

    return std::make_shared<api::session::imp::Storage>(
        crypto, factory, config);
}
}  // namespace opentxs::factory

namespace opentxs::api::session::imp
{
Storage::Storage(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const opentxs::storage::Config& config)
    : crypto_(crypto)
    , factory_(factory)
    , gc_interval_(config.gc_interval_)
    , write_lock_()
    , root_(nullptr)
    , primary_bucket_()
    , config_(config)
    , plugin_p_(
          factory::StoragePlugin(crypto_, factory_, primary_bucket_, config_))
    , plugin_(*plugin_p_)
{
    // TODO hold shared_ptr<api::Session> as a member variable
    assert_false(nullptr == plugin_p_);
}

auto Storage::AccountAlias(const identifier::Account& accountID) const noexcept
    -> UnallocatedCString
{
    return Root().Trunk().Accounts().Alias(accountID);
}

auto Storage::AccountList() const noexcept -> ObjectList
{
    return Root().Trunk().Accounts().List();
}

auto Storage::AccountContract(const identifier::Account& accountID)
    const noexcept -> identifier::UnitDefinition
{
    return Root().Trunk().Accounts().AccountContract(accountID);
}

auto Storage::AccountIssuer(const identifier::Account& accountID) const noexcept
    -> identifier::Nym
{
    return Root().Trunk().Accounts().AccountIssuer(accountID);
}

auto Storage::AccountOwner(const identifier::Account& accountID) const noexcept
    -> identifier::Nym
{
    return Root().Trunk().Accounts().AccountOwner(accountID);
}

auto Storage::AccountServer(const identifier::Account& accountID) const noexcept
    -> identifier::Notary
{
    return Root().Trunk().Accounts().AccountServer(accountID);
}

auto Storage::AccountSigner(const identifier::Account& accountID) const noexcept
    -> identifier::Nym
{
    return Root().Trunk().Accounts().AccountSigner(accountID);
}

auto Storage::AccountUnit(const identifier::Account& accountID) const noexcept
    -> UnitType
{
    return Root().Trunk().Accounts().AccountUnit(accountID);
}

auto Storage::AccountsByContract(const identifier::UnitDefinition& contract)
    const noexcept -> UnallocatedSet<identifier::Account>
{
    return Root().Trunk().Accounts().AccountsByContract(contract);
}

auto Storage::AccountsByIssuer(const identifier::Nym& issuerNym) const noexcept
    -> UnallocatedSet<identifier::Account>
{
    return Root().Trunk().Accounts().AccountsByIssuer(issuerNym);
}

auto Storage::AccountsByOwner(const identifier::Nym& ownerNym) const noexcept
    -> UnallocatedSet<identifier::Account>
{
    return Root().Trunk().Accounts().AccountsByOwner(ownerNym);
}

auto Storage::AccountsByServer(const identifier::Notary& server) const noexcept
    -> UnallocatedSet<identifier::Account>
{
    return Root().Trunk().Accounts().AccountsByServer(server);
}

auto Storage::AccountsByUnit(const UnitType unit) const noexcept
    -> UnallocatedSet<identifier::Account>
{
    return Root().Trunk().Accounts().AccountsByUnit(unit);
}

auto Storage::Bip47Chain(
    const identifier::Nym& nymID,
    const identifier::Account& channelID) const noexcept -> UnitType
{
    const bool exists = Root().Trunk().Nyms().Exists(nymID);

    if (false == exists) {
        LogError()()("Nym ")(nymID, crypto_)(" doesn't exist.").Flush();

        return UnitType::Error;
    }

    return Root().Trunk().Nyms().Nym(nymID).Bip47Channels().Chain(channelID);
}

auto Storage::Bip47ChannelsByChain(
    const identifier::Nym& nymID,
    const UnitType chain) const noexcept -> Storage::Bip47ChannelList
{
    const bool exists = Root().Trunk().Nyms().Exists(nymID);

    if (false == exists) {
        LogError()()("Nym ")(nymID, crypto_)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Trunk().Nyms().Nym(nymID).Bip47Channels().ChannelsByChain(
        chain);
}

auto Storage::blockchain_thread_item_id(
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::block::TransactionHash& txid) const noexcept
    -> identifier::Generic
{
    return opentxs::blockchain_thread_item_id(crypto_, factory_, chain, txid);
}

auto Storage::BlockchainAccountList(
    const identifier::Nym& nymID,
    const UnitType type) const noexcept -> UnallocatedSet<identifier::Account>
{
    return Root().Trunk().Nyms().Nym(nymID).BlockchainAccountList(type);
}

auto Storage::BlockchainEthereumAccountList(
    const identifier::Nym& nymID,
    const UnitType type) const noexcept -> UnallocatedSet<identifier::Account>
{
    return Root().Trunk().Nyms().Nym(nymID).BlockchainEthereumAccountList(type);
}

auto Storage::BlockchainSubaccountAccountType(
    const identifier::Nym& owner,
    const identifier::Account& subaccount) const noexcept -> UnitType
{
    using enum UnitType;
    const auto& nym = Root().Trunk().Nyms().Nym(owner);
    auto out = nym.BlockchainAccountType(subaccount);

    if (Error == out) { out = nym.Bip47Channels().Chain(subaccount); }

    if (Error == out) { out = nym.BlockchainEthereumAccountType(subaccount); }

    return out;
}

auto Storage::BlockchainThreadMap(
    const identifier::Nym& nym,
    const opentxs::blockchain::block::TransactionHash& txid) const noexcept
    -> UnallocatedVector<identifier::Generic>
{
    const auto& nyms = Root().Trunk().Nyms();

    if (false == nyms.Exists(nym)) {
        LogError()()("Nym ")(nym, crypto_)(" does not exist.").Flush();

        return {};
    }

    return nyms.Nym(nym).Threads().BlockchainThreadMap(txid);
}

auto Storage::BlockchainTransactionList(
    const identifier::Nym& nym) const noexcept -> UnallocatedVector<ByteArray>
{
    const auto& nyms = Root().Trunk().Nyms();

    if (false == nyms.Exists(nym)) {
        LogError()()("Nym ")(nym, crypto_)(" does not exist.").Flush();

        return {};
    }

    return nyms.Nym(nym).Threads().BlockchainTransactionList();
}

auto Storage::CheckTokenSpent(
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit,
    const std::uint64_t series,
    const UnallocatedCString& key) const noexcept -> bool
{
    return Root().Trunk().Notary(notary).CheckSpent(unit, series, key);
}

auto Storage::ContactAlias(const identifier::Generic& id) const noexcept
    -> UnallocatedCString
{
    return Root().Trunk().Contacts().Alias(id);
}

auto Storage::ContactList() const noexcept -> ObjectList
{
    return Root().Trunk().Contacts().List();
}

auto Storage::ContactOwnerNym(const identifier::Nym& nym) const noexcept
    -> identifier::Generic
{
    return Root().Trunk().Contacts().NymOwner(nym);
}

void Storage::ContactSaveIndices() const noexcept
{
    mutable_Root().get().mutable_Trunk().get().mutable_Contacts().get().Save();
}

auto Storage::ContactUpgradeLevel() const noexcept -> VersionNumber
{
    return Root().Trunk().Contacts().UpgradeLevel();
}

auto Storage::ContextList(const identifier::Nym& nymID) const noexcept
    -> ObjectList
{
    return Root().Trunk().Nyms().Nym(nymID).Contexts().List();
}

auto Storage::CreateThread(
    const identifier::Nym& nymID,
    const identifier::Generic& threadID,
    const UnallocatedSet<identifier::Generic>& participants) const noexcept
    -> bool
{
    const auto id = mutable_Root()
                        .get()
                        .mutable_Trunk()
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

auto Storage::DefaultNym() const noexcept -> identifier::Nym
{
    return Root().Trunk().Nyms().Default();
}

auto Storage::DefaultSeed() const noexcept -> opentxs::crypto::SeedID
{
    return Root().Trunk().Seeds().Default();
}

auto Storage::DeleteAccount(const identifier::Account& id) const noexcept
    -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Accounts()
        .get()
        .Delete(id);
}

auto Storage::DeleteContact(const identifier::Generic& id) const noexcept
    -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Contacts()
        .get()
        .Delete(id);
}

auto Storage::DeletePaymentWorkflow(
    const identifier::Nym& nymID,
    const identifier::Generic& workflowID) const noexcept -> bool
{
    const bool exists = Root().Trunk().Nyms().Exists(nymID);

    if (false == exists) {
        LogError()()("Nym ")(nymID, crypto_)(" doesn't exist.").Flush();

        return false;
    }

    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nymID)
        .get()
        .mutable_PaymentWorkflows()
        .get()
        .Delete(workflowID);
}

auto Storage::DoGC(const opentxs::storage::tree::GCParams& params) noexcept
    -> bool
{
    return plugin_.DoGC(params);
}

auto Storage::FinishGC(bool success) noexcept -> void
{
    mutable_Root().get().FinishGC(success);
}

auto Storage::GCStatus() const noexcept -> opentxs::storage::tree::GCParams
{
    return Root().GCStatus();
}

void Storage::InitBackup() { plugin_.InitBackup(); }

void Storage::InitEncryptedBackup(opentxs::crypto::symmetric::Key& key)
{
    plugin_.InitEncryptedBackup(key);
}

auto Storage::InitPlugins() -> void
{
    const auto hash = plugin_.FindBestRoot();
}

auto Storage::IssuerList(const identifier::Nym& nymID) const noexcept
    -> ObjectList
{
    const bool exists = Root().Trunk().Nyms().Exists(nymID);

    if (false == exists) {
        LogError()()("Nym ")(nymID, crypto_)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Trunk().Nyms().Nym(nymID).Issuers().List();
}

auto Storage::Load(
    const identifier::Account& accountID,
    UnallocatedCString& output,
    UnallocatedCString& alias,
    ErrorReporting checking) const noexcept -> bool
{
    return Root().Trunk().Accounts().Load(accountID, output, alias, checking);
}

auto Storage::Load(
    const identifier::Nym& nymID,
    const identifier::Account& accountID,
    proto::HDAccount& output,
    ErrorReporting checking) const noexcept -> bool
{
    auto temp = std::make_shared<proto::HDAccount>(output);
    const auto rc =
        Root().Trunk().Nyms().Nym(nymID).Load(accountID, temp, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::Nym& nymID,
    const identifier::Account& channelID,
    proto::Bip47Channel& output,
    ErrorReporting checking) const noexcept -> bool
{
    const bool exists = Root().Trunk().Nyms().Exists(nymID);

    if (false == exists) {
        LogError()()("Nym ")(nymID, crypto_)(" doesn't exist.").Flush();

        return false;
    }

    auto temp = std::make_shared<proto::Bip47Channel>(output);
    const auto rc = Root().Trunk().Nyms().Nym(nymID).Bip47Channels().Load(
        channelID, temp, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::Nym& nymID,
    const identifier::Account& accountID,
    proto::BlockchainEthereumAccountData& output,
    ErrorReporting checking) const noexcept -> bool
{
    auto temp = std::make_shared<proto::BlockchainEthereumAccountData>(output);
    const auto rc =
        Root().Trunk().Nyms().Nym(nymID).Load(accountID, temp, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::Generic& id,
    proto::Contact& output,
    ErrorReporting checking) const noexcept -> bool
{
    auto notUsed = UnallocatedCString{};

    return Load(id, output, notUsed, checking);
}

auto Storage::Load(
    const identifier::Generic& id,
    proto::Contact& output,
    UnallocatedCString& alias,
    ErrorReporting checking) const noexcept -> bool
{
    auto temp = std::make_shared<proto::Contact>(output);
    const auto rc = Root().Trunk().Contacts().Load(id, temp, alias, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::Nym& localNym,
    const identifier::Nym& remoteNym,
    proto::Context& output,
    ErrorReporting checking) const noexcept -> bool
{
    auto notUsed = UnallocatedCString{};
    auto temp = std::make_shared<proto::Context>(output);
    const auto rc = Root().Trunk().Nyms().Nym(localNym).Contexts().Load(
        remoteNym, temp, notUsed, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::Generic& id,
    proto::Credential& output,
    ErrorReporting checking) const noexcept -> bool
{
    auto temp = std::make_shared<proto::Credential>(output);
    const auto rc = Root().Trunk().Credentials().Load(id, temp, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::Nym& id,
    proto::Nym& output,
    ErrorReporting checking) const noexcept -> bool
{
    auto notUsed = UnallocatedCString{};

    return Load(id, output, notUsed, checking);
}

auto Storage::LoadNym(
    const identifier::Nym& id,
    Writer&& destination,
    ErrorReporting checking) const noexcept -> bool
{
    auto temp = std::make_shared<proto::Nym>();
    auto alias = UnallocatedCString{};

    if (false == Root().Trunk().Nyms().Nym(id).Load(temp, alias, checking)) {
        LogError()()("Failed to load nym ")(id, crypto_).Flush();

        return false;
    }

    assert_false(nullptr == temp);

    return write(*temp, std::move(destination));
}

auto Storage::Load(
    const identifier::Nym& id,
    proto::Nym& output,
    UnallocatedCString& alias,
    ErrorReporting checking) const noexcept -> bool
{
    auto temp = std::make_shared<proto::Nym>(output);
    const auto rc = Root().Trunk().Nyms().Nym(id).Load(temp, alias, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::Nym& nymID,
    const identifier::Nym& id,
    proto::Issuer& output,
    ErrorReporting checking) const noexcept -> bool
{
    if (false == Root().Trunk().Nyms().Exists(nymID)) {
        LogError()()("Nym ")(nymID, crypto_)(" doesn't exist.").Flush();

        return false;
    }

    auto notUsed = UnallocatedCString{};
    auto temp = std::make_shared<proto::Issuer>(output);
    const auto rc = Root().Trunk().Nyms().Nym(nymID).Issuers().Load(
        id, temp, notUsed, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::Nym& nymID,
    const identifier::Generic& workflowID,
    proto::PaymentWorkflow& output,
    ErrorReporting checking) const noexcept -> bool
{
    if (false == Root().Trunk().Nyms().Exists(nymID)) {
        LogError()()("Nym ")(nymID, crypto_)(" doesn't exist.").Flush();

        return false;
    }

    auto temp = std::make_shared<proto::PaymentWorkflow>(output);
    const auto rc = Root().Trunk().Nyms().Nym(nymID).PaymentWorkflows().Load(
        workflowID, temp, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::Nym& nymID,
    const identifier::Generic& id,
    const otx::client::StorageBox box,
    UnallocatedCString& output,
    UnallocatedCString& alias,
    ErrorReporting checking) const noexcept -> bool
{
    switch (box) {
        case otx::client::StorageBox::MAILINBOX: {
            return Root().Trunk().Nyms().Nym(nymID).MailInbox().Load(
                id, output, alias, checking);
        }
        case otx::client::StorageBox::MAILOUTBOX: {
            return Root().Trunk().Nyms().Nym(nymID).MailOutbox().Load(
                id, output, alias, checking);
        }
        default: {
            return false;
        }
    }
}

auto Storage::Load(
    const identifier::Nym& nymID,
    const identifier::Generic& id,
    const otx::client::StorageBox box,
    proto::PeerReply& output,
    ErrorReporting checking) const noexcept -> bool
{
    auto temp = std::make_shared<proto::PeerReply>(output);
    const auto rc = [&] {
        switch (box) {
            case otx::client::StorageBox::SENTPEERREPLY: {
                return Root().Trunk().Nyms().Nym(nymID).SentReplyBox().Load(
                    id, temp, checking);
            }
            case otx::client::StorageBox::INCOMINGPEERREPLY: {
                return Root().Trunk().Nyms().Nym(nymID).IncomingReplyBox().Load(
                    id, temp, checking);
            }
            case otx::client::StorageBox::FINISHEDPEERREPLY: {
                return Root().Trunk().Nyms().Nym(nymID).FinishedReplyBox().Load(
                    id, temp, checking);
            }
            case otx::client::StorageBox::PROCESSEDPEERREPLY: {
                return Root()
                    .Trunk()
                    .Nyms()
                    .Nym(nymID)
                    .ProcessedReplyBox()
                    .Load(id, temp, checking);
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
    const identifier::Generic& id,
    const otx::client::StorageBox box,
    proto::PeerRequest& output,
    Time& time,
    ErrorReporting checking) const noexcept -> bool
{
    auto temp = std::make_shared<proto::PeerRequest>(output);
    auto alias = UnallocatedCString{};
    const auto rc = [&] {
        switch (box) {
            case otx::client::StorageBox::SENTPEERREQUEST: {
                return Root().Trunk().Nyms().Nym(nymID).SentRequestBox().Load(
                    id, temp, alias, checking);
            }
            case otx::client::StorageBox::INCOMINGPEERREQUEST: {
                return Root()
                    .Trunk()
                    .Nyms()
                    .Nym(nymID)
                    .IncomingRequestBox()
                    .Load(id, temp, alias, checking);
            }
            case otx::client::StorageBox::FINISHEDPEERREQUEST: {
                return Root()
                    .Trunk()
                    .Nyms()
                    .Nym(nymID)
                    .FinishedRequestBox()
                    .Load(id, temp, alias, checking);
            }
            case otx::client::StorageBox::PROCESSEDPEERREQUEST: {
                return Root()
                    .Trunk()
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
        time = Clock::from_time_t([&]() -> std::time_t {
            try {

                return std::stoi(alias);
            } catch (...) {

                return {};
            }
        }());
    }

    return rc;
}

auto Storage::Load(
    const identifier::Nym& nym,
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit,
    proto::Purse& output,
    ErrorReporting checking) const noexcept -> bool
{
    const auto& nymNode = Root().Trunk().Nyms();

    if (false == nymNode.Exists(nym)) {
        LogError()()("Nym ")(nym, crypto_)(" doesn't exist.").Flush();

        return false;
    }

    auto temp = std::make_shared<proto::Purse>(output);
    const auto rc = nymNode.Nym(nym).Load(notary, unit, temp, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const opentxs::crypto::SeedID& id,
    proto::Seed& output,
    ErrorReporting checking) const noexcept -> bool
{
    auto notUsed = UnallocatedCString{};

    return Load(id, output, notUsed, checking);
}

auto Storage::Load(
    const opentxs::crypto::SeedID& id,
    proto::Seed& output,
    UnallocatedCString& alias,
    ErrorReporting checking) const noexcept -> bool
{
    auto temp = std::make_shared<proto::Seed>(output);
    const auto rc = Root().Trunk().Seeds().Load(id, temp, alias, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::Notary& id,
    proto::ServerContract& output,
    ErrorReporting checking) const noexcept -> bool
{
    auto notUsed = UnallocatedCString{};

    return Load(id, output, notUsed, checking);
}

auto Storage::Load(
    const identifier::Notary& id,
    proto::ServerContract& output,
    UnallocatedCString& alias,
    ErrorReporting checking) const noexcept -> bool
{
    auto temp = std::make_shared<proto::ServerContract>(output);
    const auto rc = Root().Trunk().Servers().Load(id, temp, alias, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::Nym& nymId,
    const identifier::Generic& threadId,
    proto::StorageThread& output) const noexcept -> bool
{
    const bool exists =
        Root().Trunk().Nyms().Nym(nymId).Threads().Exists(threadId);

    if (!exists) { return false; }

    output =
        Root().Trunk().Nyms().Nym(nymId).Threads().Thread(threadId).Items();

    return true;
}

auto Storage::Load(proto::Ciphertext& output, ErrorReporting checking)
    const noexcept -> bool
{
    auto temp = std::make_shared<proto::Ciphertext>(output);
    const auto rc = Root().Trunk().Load(temp, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::Load(
    const identifier::UnitDefinition& id,
    proto::UnitDefinition& output,
    ErrorReporting checking) const noexcept -> bool
{
    auto notUsed = UnallocatedCString{};

    return Load(id, output, notUsed, checking);
}

auto Storage::Load(
    const identifier::UnitDefinition& id,
    proto::UnitDefinition& output,
    UnallocatedCString& alias,
    ErrorReporting checking) const noexcept -> bool
{
    auto temp = std::make_shared<proto::UnitDefinition>(output);
    const auto rc = Root().Trunk().Units().Load(id, temp, alias, checking);

    if (rc && temp) { output = *temp; }

    return rc;
}

auto Storage::LocalNyms() const noexcept -> Set<identifier::Nym>
{
    return Root().Trunk().Nyms().LocalNyms();
}

auto Storage::MarkTokenSpent(
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit,
    const std::uint64_t series,
    const UnallocatedCString& key) const noexcept -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Notary(notary)
        .get()
        .MarkSpent(unit, series, key);
}

auto Storage::MoveThreadItem(
    const identifier::Nym& nymId,
    const identifier::Generic& fromThreadID,
    const identifier::Generic& toThreadID,
    const identifier::Generic& itemID) const noexcept -> bool
{
    const auto& nyms = Root().Trunk().Nyms();

    if (false == nyms.Exists(nymId)) {
        LogError()()("Nym ")(nymId, crypto_)(" does not exist.").Flush();

        return false;
    }

    const auto& threads = nyms.Nym(nymId).Threads();

    if (false == threads.Exists(fromThreadID)) {
        LogError()()("Source thread ")(fromThreadID, crypto_)(
            " does not exist.")
            .Flush();

        return false;
    }

    if (false == threads.Exists(toThreadID)) {
        LogError()()("Destination thread ")(toThreadID, crypto_)(
            " does not exist.")
            .Flush();

        return false;
    }

    auto& fromThread = mutable_Root()
                           .get()
                           .mutable_Trunk()
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
    auto account = identifier::Generic{};

    for (const auto& item : thread.item()) {
        if (item.id() == itemID.asBase58(crypto_)) {
            found = true;
            time = item.time();
            box = static_cast<otx::client::StorageBox>(item.box());
            index = item.index();
            account = factory_.IdentifierFromBase58(item.account());

            break;
        }
    }

    if (false == found) {
        LogError()()("Item does not exist.").Flush();

        return false;
    }

    if (false == fromThread.Remove(itemID)) {
        LogError()()("Failed to remove item.").Flush();

        return false;
    }

    auto& toThread = mutable_Root()
                         .get()
                         .mutable_Trunk()
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
        LogError()()("Failed to insert item.").Flush();

        return false;
    }

    return true;
}

auto Storage::mutable_Root() const noexcept
    -> Editor<opentxs::storage::tree::Root>
{
    std::function<void(opentxs::storage::tree::Root*, Lock&)> callback =
        [&](opentxs::storage::tree::Root* in, Lock& lock) -> void {
        this->save(in, lock);
    };

    return {write_lock_, root(), callback};
}

auto Storage::NymBoxList(
    const identifier::Nym& nymID,
    const otx::client::StorageBox box) const noexcept -> ObjectList
{
    switch (box) {
        case otx::client::StorageBox::SENTPEERREQUEST: {
            return Root().Trunk().Nyms().Nym(nymID).SentRequestBox().List();
        }
        case otx::client::StorageBox::INCOMINGPEERREQUEST: {
            return Root().Trunk().Nyms().Nym(nymID).IncomingRequestBox().List();
        }
        case otx::client::StorageBox::SENTPEERREPLY: {
            return Root().Trunk().Nyms().Nym(nymID).SentReplyBox().List();
        }
        case otx::client::StorageBox::INCOMINGPEERREPLY: {
            return Root().Trunk().Nyms().Nym(nymID).IncomingReplyBox().List();
        }
        case otx::client::StorageBox::FINISHEDPEERREQUEST: {
            return Root().Trunk().Nyms().Nym(nymID).FinishedRequestBox().List();
        }
        case otx::client::StorageBox::FINISHEDPEERREPLY: {
            return Root().Trunk().Nyms().Nym(nymID).FinishedReplyBox().List();
        }
        case otx::client::StorageBox::PROCESSEDPEERREQUEST: {
            return Root()
                .Trunk()
                .Nyms()
                .Nym(nymID)
                .ProcessedRequestBox()
                .List();
        }
        case otx::client::StorageBox::PROCESSEDPEERREPLY: {
            return Root().Trunk().Nyms().Nym(nymID).ProcessedReplyBox().List();
        }
        case otx::client::StorageBox::MAILINBOX: {
            return Root().Trunk().Nyms().Nym(nymID).MailInbox().List();
        }
        case otx::client::StorageBox::MAILOUTBOX: {
            return Root().Trunk().Nyms().Nym(nymID).MailOutbox().List();
        }
        default: {
            return {};
        }
    }
}

auto Storage::NymList() const noexcept -> ObjectList
{
    return Root().Trunk().Nyms().List();
}

auto Storage::PaymentWorkflowList(const identifier::Nym& nymID) const noexcept
    -> ObjectList
{
    if (false == Root().Trunk().Nyms().Exists(nymID)) {
        LogError()()("Nym ")(nymID, crypto_)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Trunk().Nyms().Nym(nymID).PaymentWorkflows().List();
}

auto Storage::PaymentWorkflowLookup(
    const identifier::Nym& nymID,
    const identifier::Generic& sourceID) const noexcept -> identifier::Generic
{
    if (false == Root().Trunk().Nyms().Exists(nymID)) {
        LogError()()("Nym ")(nymID, crypto_)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Trunk().Nyms().Nym(nymID).PaymentWorkflows().LookupBySource(
        sourceID);
}

auto Storage::PaymentWorkflowsByAccount(
    const identifier::Nym& nymID,
    const identifier::Account& accountID) const noexcept
    -> UnallocatedSet<identifier::Generic>
{
    if (false == Root().Trunk().Nyms().Exists(nymID)) {
        LogError()()("Nym ")(nymID, crypto_)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Trunk().Nyms().Nym(nymID).PaymentWorkflows().ListByAccount(
        accountID);
}

auto Storage::PaymentWorkflowsByState(
    const identifier::Nym& nymID,
    const otx::client::PaymentWorkflowType type,
    const otx::client::PaymentWorkflowState state) const noexcept
    -> UnallocatedSet<identifier::Generic>
{
    if (false == Root().Trunk().Nyms().Exists(nymID)) {
        LogError()()("Nym ")(nymID, crypto_)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Trunk().Nyms().Nym(nymID).PaymentWorkflows().ListByState(
        type, state);
}

auto Storage::PaymentWorkflowsByUnit(
    const identifier::Nym& nymID,
    const identifier::UnitDefinition& unitID) const noexcept
    -> UnallocatedSet<identifier::Generic>
{
    if (false == Root().Trunk().Nyms().Exists(nymID)) {
        LogError()()("Nym ")(nymID, crypto_)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Trunk().Nyms().Nym(nymID).PaymentWorkflows().ListByUnit(
        unitID);
}

auto Storage::PaymentWorkflowState(
    const identifier::Nym& nymID,
    const identifier::Generic& workflowID) const noexcept -> std::
    pair<otx::client::PaymentWorkflowType, otx::client::PaymentWorkflowState>
{
    if (false == Root().Trunk().Nyms().Exists(nymID)) {
        LogError()()("Nym ")(nymID, crypto_)(" doesn't exist.").Flush();

        return {};
    }

    return Root().Trunk().Nyms().Nym(nymID).PaymentWorkflows().GetState(
        workflowID);
}

auto Storage::RelabelThread(
    const identifier::Generic& threadID,
    const UnallocatedCString& label) const noexcept -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Nyms()
        .get()
        .RelabelThread(threadID, label);
}

auto Storage::RemoveBlockchainThreadItem(
    const identifier::Nym& nym,
    const identifier::Generic& threadID,
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::block::TransactionHash& txid) const noexcept
    -> bool
{
    const auto& nyms = Root().Trunk().Nyms();

    if (false == nyms.Exists(nym)) {
        LogError()()("Nym ")(nym, crypto_)(" does not exist.").Flush();

        return false;
    }

    const auto& threads = nyms.Nym(nym).Threads();

    if (false == threads.Exists(threadID)) {
        LogError()()("Thread ")(threadID, crypto_)(" does not exist.").Flush();

        return false;
    }

    auto& fromThread = mutable_Root()
                           .get()
                           .mutable_Trunk()
                           .get()
                           .mutable_Nyms()
                           .get()
                           .mutable_Nym(nym)
                           .get()
                           .mutable_Threads(txid, threadID, false)
                           .get()
                           .mutable_Thread(threadID)
                           .get();
    const auto thread = fromThread.Items();
    auto found{false};
    const auto id = blockchain_thread_item_id(chain, txid);

    for (const auto& item : thread.item()) {
        if (item.id() == id.asBase58(crypto_)) {
            found = true;

            break;
        }
    }

    if (false == found) {
        LogError()()("Item does not exist.").Flush();

        return false;
    }

    if (false == fromThread.Remove(id)) {
        LogError()()("Failed to remove item.").Flush();

        return false;
    }

    return true;
}

auto Storage::RemoveNymBoxItem(
    const identifier::Nym& nymID,
    const otx::client::StorageBox box,
    const identifier::Generic& itemID) const noexcept -> bool
{
    switch (box) {
        case otx::client::StorageBox::SENTPEERREQUEST: {
            return mutable_Root()
                .get()
                .mutable_Trunk()
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
                .mutable_Trunk()
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
                .mutable_Trunk()
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
                .mutable_Trunk()
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
                .mutable_Trunk()
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
                .mutable_Trunk()
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
                .mutable_Trunk()
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
                .mutable_Trunk()
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
                                           .mutable_Trunk()
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
                                 .mutable_Trunk()
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
                                           .mutable_Trunk()
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
                                 .mutable_Trunk()
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

auto Storage::RemoveServer(const identifier::Notary& id) const noexcept -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Servers()
        .get()
        .Delete(id);
}

auto Storage::RemoveThreadItem(
    const identifier::Nym& nym,
    const identifier::Generic& threadID,
    const identifier::Generic& id) const noexcept -> bool
{
    const auto& nyms = Root().Trunk().Nyms();

    if (false == nyms.Exists(nym)) {
        LogError()()("Nym ")(nym, crypto_)(" does not exist.").Flush();

        return false;
    }

    const auto& threads = nyms.Nym(nym).Threads();

    if (false == threads.Exists(threadID)) {
        LogError()()("Thread ")(threadID, crypto_)(" does not exist.").Flush();

        return false;
    }

    auto& fromThread = mutable_Root()
                           .get()
                           .mutable_Trunk()
                           .get()
                           .mutable_Nyms()
                           .get()
                           .mutable_Nym(nym)
                           .get()
                           .mutable_Threads()
                           .get()
                           .mutable_Thread(threadID)
                           .get();
    const auto thread = fromThread.Items();
    auto found{false};

    for (const auto& item : thread.item()) {
        if (item.id() == id.asBase58(crypto_)) {
            found = true;

            break;
        }
    }

    if (false == found) {
        LogError()()("Item does not exist.").Flush();

        return false;
    }

    if (false == fromThread.Remove(id)) {
        LogError()()("Failed to remove item.").Flush();

        return false;
    }

    return true;
}

auto Storage::RemoveUnitDefinition(
    const identifier::UnitDefinition& id) const noexcept -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Units()
        .get()
        .Delete(id);
}

auto Storage::RenameThread(
    const identifier::Nym& nymId,
    const identifier::Generic& threadId,
    const identifier::Generic& newID) const noexcept -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nymId)
        .get()
        .mutable_Threads()
        .get()
        .Rename(threadId, newID);
}

auto Storage::root() const noexcept -> opentxs::storage::tree::Root*
{
    Lock lock(write_lock_);

    if (!root_) {
        root_ = std::make_unique<opentxs::storage::tree::Root>(
            crypto_, factory_, plugin_, plugin_.LoadRoot(), primary_bucket_);
    }

    assert_false(nullptr == root_);

    lock.unlock();

    return root_.get();
}

auto Storage::Root() const noexcept -> const opentxs::storage::tree::Root&
{
    return *root();
}

auto Storage::save(opentxs::storage::tree::Root* in, const Lock& lock)
    const noexcept -> void
{
    assert_true(verify_write_lock(lock));
    assert_false(nullptr == in);

    plugin_.StoreRoot(in->root_);
}

auto Storage::SeedList() const noexcept -> ObjectList
{
    return Root().Trunk().Seeds().List();
}

auto Storage::SetAccountAlias(
    const identifier::Account& id,
    std::string_view alias) const noexcept -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Accounts()
        .get()
        .SetAlias(id, alias);
}

auto Storage::SetContactAlias(
    const identifier::Generic& id,
    std::string_view alias) const noexcept -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Contacts()
        .get()
        .SetAlias(id, alias);
}

auto Storage::SetDefaultNym(const identifier::Nym& id) const noexcept -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Nyms()
        .get()
        .SetDefault(id);
}

auto Storage::SetDefaultSeed(const opentxs::crypto::SeedID& id) const noexcept
    -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Seeds()
        .get()
        .SetDefault(id);
}

auto Storage::SetNymAlias(const identifier::Nym& id, std::string_view alias)
    const noexcept -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(id)
        .get()
        .SetAlias(alias);
}

auto Storage::SetPeerRequestTime(
    const identifier::Nym& nymID,
    const identifier::Generic& id,
    const otx::client::StorageBox box) const noexcept -> bool
{
    const UnallocatedCString now = std::to_string(time(nullptr));

    switch (box) {
        case otx::client::StorageBox::SENTPEERREQUEST: {
            return mutable_Root()
                .get()
                .mutable_Trunk()
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
                .mutable_Trunk()
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
                .mutable_Trunk()
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
                .mutable_Trunk()
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
    const identifier::Generic& threadId,
    const identifier::Generic& itemId,
    const bool unread) const noexcept -> bool
{
    auto& nyms =
        mutable_Root().get().mutable_Trunk().get().mutable_Nyms().get();

    if (false == nyms.Exists(nymId)) {
        LogError()()("Nym ")(nymId, crypto_)(" does not exist.").Flush();

        return false;
    }

    auto& threads = nyms.mutable_Nym(nymId).get().mutable_Threads().get();

    if (false == threads.Exists(threadId)) {
        LogError()()("Thread ")(threadId, crypto_)(" does not exist.").Flush();

        return false;
    }

    return threads.mutable_Thread(threadId).get().Read(itemId, unread);
}

auto Storage::SetSeedAlias(
    const opentxs::crypto::SeedID& id,
    std::string_view alias) const noexcept -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Seeds()
        .get()
        .SetAlias(id, alias);
}

auto Storage::SetServerAlias(
    const identifier::Notary& id,
    std::string_view alias) const noexcept -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Servers()
        .get()
        .SetAlias(id, alias);
}

auto Storage::SetThreadAlias(
    const identifier::Nym& nymId,
    const identifier::Generic& threadId,
    std::string_view alias) const noexcept -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
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
    std::string_view alias) const noexcept -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Units()
        .get()
        .SetAlias(id, alias);
}

auto Storage::ServerAlias(const identifier::Notary& id) const noexcept
    -> UnallocatedCString
{
    return Root().Trunk().Servers().Alias(id);
}

auto Storage::ServerList() const noexcept -> ObjectList
{
    return Root().Trunk().Servers().List();
}

auto Storage::Start(std::shared_ptr<const api::internal::Session> api) noexcept
    -> void
{
    const auto me = shared_from_this();

    assert_false(nullptr == me);

    const auto& zmq = api->Network().ZeroMQ().Context().Internal();
    const auto batchID = zmq.PreallocateBatch();
    auto* alloc = zmq.Alloc(batchID);
    using Actor = opentxs::storage::tree::Actor;
    auto actor = std::allocate_shared<Actor>(
        alloc::PMR<Actor>{alloc},
        api,
        me,
        batchID,
        gc_interval_,
        opentxs::network::zeromq::MakeArbitraryInproc(alloc));

    assert_false(nullptr == actor);

    actor->Init(actor);
}

void Storage::start() { InitPlugins(); }

auto Storage::StartGC() const noexcept
    -> std::optional<opentxs::storage::tree::GCParams>
{
    return mutable_Root().get().StartGC();
}

auto Storage::Store(
    const identifier::Account& accountID,
    const UnallocatedCString& data,
    std::string_view alias,
    const identifier::Nym& ownerNym,
    const identifier::Nym& signerNym,
    const identifier::Nym& issuerNym,
    const identifier::Notary& server,
    const identifier::UnitDefinition& contract,
    const UnitType unit) const noexcept -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
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
    const UnitType type,
    const proto::HDAccount& data) const noexcept -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nymID)
        .get()
        .Store(type, data);
}

auto Storage::Store(
    const identifier::Nym& nymID,
    const identifier::Account& channelID,
    const proto::Bip47Channel& data) const noexcept -> bool
{
    const bool exists = Root().Trunk().Nyms().Exists(nymID);

    if (false == exists) {
        LogError()()("Nym ")(nymID, crypto_)(" doesn't exist.").Flush();

        return false;
    }

    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nymID)
        .get()
        .mutable_Bip47Channels()
        .get()
        .Store(channelID, data);
}

auto Storage::Store(
    const identifier::Nym& nymID,
    const UnitType type,
    const proto::BlockchainEthereumAccountData& data) const noexcept -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nymID)
        .get()
        .Store(type, data);
}

auto Storage::Store(const proto::Contact& data) const noexcept -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Contacts()
        .get()
        .Store(data, data.label());
}

auto Storage::Store(const proto::Context& data) const noexcept -> bool
{
    auto notUsed = UnallocatedCString{};

    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(factory_.Internal().NymID(data.localnym()))
        .get()
        .mutable_Contexts()
        .get()
        .Store(data, notUsed);
}

auto Storage::Store(const proto::Credential& data) const noexcept -> bool
{
    auto notUsed = UnallocatedCString{};

    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Credentials()
        .get()
        .Store(data, notUsed);
}

auto Storage::Store(const proto::Nym& data, std::string_view alias)
    const noexcept -> bool
{
    auto plaintext = UnallocatedCString{};
    const auto id = factory_.Internal().NymID(data.id());

    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(id)
        .get()
        .Store(data, alias, plaintext);
}

auto Storage::Store(const ReadView& view, std::string_view alias) const noexcept
    -> bool
{
    return Store(proto::Factory<proto::Nym>(view), alias);
}

auto Storage::Store(const identifier::Nym& nymID, const proto::Issuer& data)
    const noexcept -> bool
{
    const bool exists = Root().Trunk().Nyms().Exists(nymID);

    if (false == exists) {
        LogError()()("Nym ")(nymID, crypto_)(" doesn't exist.").Flush();

        return false;
    }

    return mutable_Root()
        .get()
        .mutable_Trunk()
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
    const proto::PaymentWorkflow& data) const noexcept -> bool
{
    const bool exists = Root().Trunk().Nyms().Exists(nymID);

    if (false == exists) {
        LogError()()("Nym ")(nymID, crypto_)(" doesn't exist.").Flush();

        return false;
    }

    UnallocatedCString notUsed{};

    return mutable_Root()
        .get()
        .mutable_Trunk()
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
    const identifier::Generic& threadid,
    const identifier::Generic& itemid,
    const std::uint64_t time,
    std::string_view alias,
    const UnallocatedCString& data,
    const otx::client::StorageBox box,
    const identifier::Generic& workflow) const noexcept -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nymid)
        .get()
        .mutable_Threads()
        .get()
        .mutable_Thread(threadid)
        .get()
        .Add(itemid, time, box, alias, data, 0, workflow);
}

auto Storage::Store(
    const identifier::Nym& nym,
    const identifier::Generic& thread,
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::block::TransactionHash& txid,
    const Time time) const noexcept -> bool
{
    const auto alias = UnallocatedCString{};
    const auto account = identifier::Generic{};
    const auto id = blockchain_thread_item_id(chain, txid);

    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nym)
        .get()
        .mutable_Threads(txid, thread, true)
        .get()
        .mutable_Thread(thread)
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
    const otx::client::StorageBox box) const noexcept -> bool
{
    switch (box) {
        case otx::client::StorageBox::SENTPEERREPLY: {
            return mutable_Root()
                .get()
                .mutable_Trunk()
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
                .mutable_Trunk()
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
                .mutable_Trunk()
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
                .mutable_Trunk()
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
    const otx::client::StorageBox box) const noexcept -> bool
{
    // Use the alias field to store the time at which the request was saved.
    // Useful for managing retry logic in the high level api
    const UnallocatedCString now = std::to_string(time(nullptr));

    switch (box) {
        case otx::client::StorageBox::SENTPEERREQUEST: {
            return mutable_Root()
                .get()
                .mutable_Trunk()
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
                .mutable_Trunk()
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
                .mutable_Trunk()
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
                .mutable_Trunk()
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

auto Storage::Store(const identifier::Nym& nym, const proto::Purse& purse)
    const noexcept -> bool
{
    auto nymNode = mutable_Root().get().mutable_Trunk().get().mutable_Nyms();

    if (false == nymNode.get().Exists(nym)) {
        LogError()()("Nym ")(nym, crypto_)(" doesn't exist.").Flush();

        return false;
    }

    return nymNode.get().mutable_Nym(nym).get().Store(purse);
}

auto Storage::Store(const opentxs::crypto::SeedID& id, const proto::Seed& data)
    const noexcept -> bool
{
    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Seeds()
        .get()
        .Store(id, data);
}

auto Storage::Store(const proto::ServerContract& data, std::string_view alias)
    const noexcept -> bool
{
    auto storageVersion(data);
    storageVersion.clear_publicnym();
    UnallocatedCString plaintext;

    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Servers()
        .get()
        .Store(data, alias, plaintext);
}

auto Storage::Store(const proto::Ciphertext& serialized) const noexcept -> bool
{
    return mutable_Root().get().mutable_Trunk().get().Store(serialized);
}

auto Storage::Store(const proto::UnitDefinition& data, std::string_view alias)
    const noexcept -> bool
{
    auto storageVersion(data);
    storageVersion.clear_issuer_nym();
    UnallocatedCString plaintext;

    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Units()
        .get()
        .Store(data, alias, plaintext);
}

auto Storage::ThreadList(const identifier::Nym& nymID, const bool unreadOnly)
    const noexcept -> ObjectList
{
    return Root().Trunk().Nyms().Nym(nymID).Threads().List(unreadOnly);
}

auto Storage::ThreadAlias(
    const identifier::Nym& nymID,
    const identifier::Generic& threadID) const noexcept -> UnallocatedCString
{
    return Root().Trunk().Nyms().Nym(nymID).Threads().Thread(threadID).Alias();
}

auto Storage::UnaffiliatedBlockchainTransaction(
    const identifier::Nym& nym,
    const opentxs::blockchain::block::TransactionHash& txid) const noexcept
    -> bool
{
    static const auto blank = identifier::Generic{};

    return mutable_Root()
        .get()
        .mutable_Trunk()
        .get()
        .mutable_Nyms()
        .get()
        .mutable_Nym(nym)
        .get()
        .mutable_Threads()
        .get()
        .AddIndex(txid, blank);
}

auto Storage::UnitDefinitionAlias(
    const identifier::UnitDefinition& id) const noexcept -> UnallocatedCString
{
    return Root().Trunk().Units().Alias(id);
}

auto Storage::UnitDefinitionList() const noexcept -> ObjectList
{
    return Root().Trunk().Units().List();
}

auto Storage::UnreadCount(
    const identifier::Nym& nymId,
    const identifier::Generic& threadId) const noexcept -> std::size_t
{
    const auto& nyms = Root().Trunk().Nyms();

    if (false == nyms.Exists(nymId)) {
        LogError()()("Nym ")(nymId, crypto_)(" does not exist.").Flush();

        return 0;
    }

    const auto& threads = nyms.Nym(nymId).Threads();

    if (false == threads.Exists(threadId)) {
        LogError()()("Thread ")(threadId, crypto_)(" does not exist.").Flush();

        return 0;
    }

    return threads.Thread(threadId).UnreadCount();
}

auto Storage::Upgrade() noexcept -> void { mutable_Root().get().Upgrade(); }

auto Storage::verify_write_lock(const Lock& lock) const noexcept -> bool
{
    if (lock.mutex() != &write_lock_) {
        LogError()()("Incorrect mutex.").Flush();

        return false;
    }

    if (false == lock.owns_lock()) {
        LogError()()("Lock not owned.").Flush();

        return false;
    }

    return true;
}

Storage::~Storage() = default;
}  // namespace opentxs::api::session::imp

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <memory>
#include <mutex>
#include <optional>
#include <string_view>
#include <utility>

#include "internal/api/session/Storage.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/HDSeed.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/storage/Types.hpp"
#include "opentxs/storage/Types.internal.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "util/storage/Config.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Session;
}  // namespace internal

namespace session
{
class Factory;
}  // namespace session

class Crypto;
}  // namespace api

namespace crypto
{
namespace symmetric
{
class Key;
}  // namespace symmetric
}  // namespace crypto

namespace proto
{
class Bip47Channel;
class BlockchainEthereumAccountData;
class Ciphertext;
class Contact;
class Context;
class Credential;
class HDAccount;
class Issuer;
class Nym;
class PaymentWorkflow;
class PeerReply;
class PeerRequest;
class Purse;
class Seed;
class ServerContract;
class StorageThread;
class UnitDefinition;
}  // namespace proto

namespace storage
{
namespace driver
{
class Plugin;
}  // namespace driver

namespace tree
{
class Root;
struct GCParams;
}  // namespace tree
}  // namespace storage

class ByteArray;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::imp
{
using opentxs::storage::ErrorReporting;

// Content-aware storage module for opentxs
//
// Storage accepts serialized opentxs objects in protobuf form, writes them
// to persistant storage, and retrieves them on demand.
//
// All objects are stored in a key-value database. The keys are always the
// hash of the object being stored.
//
// This class maintains a set of index objects which map logical identifiers
// to object hashes. These index objects are stored in the same K-V namespace
// as the opentxs objects.
//
// The interface to a particular KV database is provided by child classes
// implementing this interface. Implementations need only provide methods for
// storing/retrieving arbitrary key-value pairs, and methods for setting and
// retrieving the hash of the root index object.
//
// The implementation of this interface must support the concept of "buckets"
// Objects are either stored and retrieved from either the primary bucket, or
// the alternate bucket. This allows for garbage collection of outdated keys
// to be implemented.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
class Storage final : public internal::Storage,
                      public std::enable_shared_from_this<Storage>
{
public:
    auto AccountAlias(const identifier::Account& accountID) const noexcept
        -> UnallocatedCString final;
    auto AccountList() const noexcept -> ObjectList final;
    auto AccountContract(const identifier::Account& accountID) const noexcept
        -> identifier::UnitDefinition final;
    auto AccountIssuer(const identifier::Account& accountID) const noexcept
        -> identifier::Nym final;
    auto AccountOwner(const identifier::Account& accountID) const noexcept
        -> identifier::Nym final;
    auto AccountServer(const identifier::Account& accountID) const noexcept
        -> identifier::Notary final;
    auto AccountSigner(const identifier::Account& accountID) const noexcept
        -> identifier::Nym final;
    auto AccountUnit(const identifier::Account& accountID) const noexcept
        -> UnitType final;
    auto AccountsByContract(const identifier::UnitDefinition& contract)
        const noexcept -> UnallocatedSet<identifier::Account> final;
    auto AccountsByIssuer(const identifier::Nym& issuerNym) const noexcept
        -> UnallocatedSet<identifier::Account> final;
    auto AccountsByOwner(const identifier::Nym& ownerNym) const noexcept
        -> UnallocatedSet<identifier::Account> final;
    auto AccountsByServer(const identifier::Notary& server) const noexcept
        -> UnallocatedSet<identifier::Account> final;
    auto AccountsByUnit(const UnitType unit) const noexcept
        -> UnallocatedSet<identifier::Account> final;
    auto Bip47Chain(
        const identifier::Nym& nymID,
        const identifier::Account& channelID) const noexcept -> UnitType final;
    auto Bip47ChannelsByChain(
        const identifier::Nym& nymID,
        const UnitType chain) const noexcept -> Bip47ChannelList final;
    auto BlockchainAccountList(
        const identifier::Nym& nymID,
        const UnitType type) const noexcept
        -> UnallocatedSet<identifier::Account> final;
    auto BlockchainEthereumAccountList(
        const identifier::Nym& nymID,
        const UnitType type) const noexcept
        -> UnallocatedSet<identifier::Account> final;
    auto BlockchainSubaccountAccountType(
        const identifier::Nym& owner,
        const identifier::Account& id) const noexcept -> UnitType final;
    auto BlockchainThreadMap(
        const identifier::Nym& nym,
        const opentxs::blockchain::block::TransactionHash& txid) const noexcept
        -> UnallocatedVector<identifier::Generic> final;
    auto BlockchainTransactionList(const identifier::Nym& nym) const noexcept
        -> UnallocatedVector<ByteArray> final;
    auto CheckTokenSpent(
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit,
        const std::uint64_t series,
        const UnallocatedCString& key) const noexcept -> bool final;
    auto ContactAlias(const identifier::Generic& id) const noexcept
        -> UnallocatedCString final;
    auto ContactList() const noexcept -> ObjectList final;
    auto ContextList(const identifier::Nym& nymID) const noexcept
        -> ObjectList final;
    auto ContactOwnerNym(const identifier::Nym& nym) const noexcept
        -> identifier::Generic final;
    auto ContactSaveIndices() const noexcept -> void final;
    auto ContactUpgradeLevel() const noexcept -> VersionNumber final;
    auto CreateThread(
        const identifier::Nym& nymID,
        const identifier::Generic& threadID,
        const UnallocatedSet<identifier::Generic>& participants) const noexcept
        -> bool final;
    auto DeleteAccount(const identifier::Account& id) const noexcept
        -> bool final;
    auto DeleteContact(const identifier::Generic& id) const noexcept
        -> bool final;
    auto DefaultNym() const noexcept -> identifier::Nym final;
    auto DefaultSeed() const noexcept -> opentxs::crypto::SeedID final;
    auto DeletePaymentWorkflow(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID) const noexcept -> bool final;
    auto GCStatus() const noexcept -> opentxs::storage::tree::GCParams;
    auto IssuerList(const identifier::Nym& nymID) const noexcept
        -> ObjectList final;
    auto Load(
        const identifier::Account& accountID,
        UnallocatedCString& output,
        UnallocatedCString& alias,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const identifier::Nym& nymID,
        const identifier::Account& accountID,
        proto::HDAccount& output,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const identifier::Nym& nymID,
        const identifier::Account& channelID,
        proto::Bip47Channel& output,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const identifier::Nym& nymID,
        const identifier::Account& accountID,
        proto::BlockchainEthereumAccountData& output,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool final;
    auto Load(
        const identifier::Generic& id,
        proto::Contact& contact,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const identifier::Generic& id,
        proto::Contact& contact,
        UnallocatedCString& alias,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const identifier::Nym& localNym,
        const identifier::Nym& remoteNym,
        proto::Context& context,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const identifier::Generic& id,
        proto::Credential& cred,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const identifier::Nym& id,
        proto::Nym& nym,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const identifier::Nym& id,
        proto::Nym& nym,
        UnallocatedCString& alias,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const identifier::Nym& nymID,
        const identifier::Nym& id,
        proto::Issuer& issuer,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID,
        proto::PaymentWorkflow& workflow,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const identifier::Nym& nymID,
        const identifier::Generic& id,
        const otx::client::StorageBox box,
        UnallocatedCString& output,
        UnallocatedCString& alias,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const identifier::Nym& nymID,
        const identifier::Generic& id,
        const otx::client::StorageBox box,
        proto::PeerReply& request,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const identifier::Nym& nymID,
        const identifier::Generic& id,
        const otx::client::StorageBox box,
        proto::PeerRequest& request,
        Time& time,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const identifier::Nym& nym,
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit,
        proto::Purse& output,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const opentxs::crypto::SeedID& id,
        proto::Seed& seed,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const opentxs::crypto::SeedID& id,
        proto::Seed& seed,
        UnallocatedCString& alias,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const identifier::Notary& id,
        proto::ServerContract& contract,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const identifier::Notary& id,
        proto::ServerContract& contract,
        UnallocatedCString& alias,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const identifier::Nym& nymId,
        const identifier::Generic& threadId,
        proto::StorageThread& thread) const noexcept -> bool final;
    auto Load(proto::Ciphertext& output, ErrorReporting checking) const noexcept
        -> bool final;
    auto Load(
        const identifier::UnitDefinition& id,
        proto::UnitDefinition& contract,
        ErrorReporting checking) const noexcept -> bool final;
    auto Load(
        const identifier::UnitDefinition& id,
        proto::UnitDefinition& contract,
        UnallocatedCString& alias,
        ErrorReporting checking) const noexcept -> bool final;
    auto LoadNym(
        const identifier::Nym& id,
        Writer&& destination,
        ErrorReporting checking) const noexcept -> bool final;
    auto LocalNyms() const noexcept -> Set<identifier::Nym> final;
    auto MarkTokenSpent(
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit,
        const std::uint64_t series,
        const UnallocatedCString& key) const noexcept -> bool final;
    auto MoveThreadItem(
        const identifier::Nym& nymId,
        const identifier::Generic& fromThreadID,
        const identifier::Generic& toThreadID,
        const identifier::Generic& itemID) const noexcept -> bool final;
    auto NymBoxList(
        const identifier::Nym& nymID,
        const otx::client::StorageBox box) const noexcept -> ObjectList final;
    auto NymList() const noexcept -> ObjectList final;
    auto PaymentWorkflowList(const identifier::Nym& nymID) const noexcept
        -> ObjectList final;
    auto PaymentWorkflowLookup(
        const identifier::Nym& nymID,
        const identifier::Generic& sourceID) const noexcept
        -> identifier::Generic final;
    auto PaymentWorkflowsByAccount(
        const identifier::Nym& nymID,
        const identifier::Account& accountID) const noexcept
        -> UnallocatedSet<identifier::Generic> final;
    auto PaymentWorkflowsByState(
        const identifier::Nym& nymID,
        const otx::client::PaymentWorkflowType type,
        const otx::client::PaymentWorkflowState state) const noexcept
        -> UnallocatedSet<identifier::Generic> final;
    auto PaymentWorkflowsByUnit(
        const identifier::Nym& nymID,
        const identifier::UnitDefinition& unitID) const noexcept
        -> UnallocatedSet<identifier::Generic> final;
    auto PaymentWorkflowState(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID) const noexcept
        -> std::pair<
            otx::client::PaymentWorkflowType,
            otx::client::PaymentWorkflowState> final;
    auto RelabelThread(
        const identifier::Generic& threadID,
        const UnallocatedCString& label) const noexcept -> bool final;
    auto RemoveBlockchainThreadItem(
        const identifier::Nym& nym,
        const identifier::Generic& thread,
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::TransactionHash& txid) const noexcept
        -> bool final;
    auto RemoveNymBoxItem(
        const identifier::Nym& nymID,
        const otx::client::StorageBox box,
        const identifier::Generic& itemID) const noexcept -> bool final;
    auto RemoveServer(const identifier::Notary& id) const noexcept
        -> bool final;
    auto RemoveThreadItem(
        const identifier::Nym& nym,
        const identifier::Generic& thread,
        const identifier::Generic& id) const noexcept -> bool final;
    auto RemoveUnitDefinition(
        const identifier::UnitDefinition& id) const noexcept -> bool final;
    auto RenameThread(
        const identifier::Nym& nymId,
        const identifier::Generic& threadId,
        const identifier::Generic& newID) const noexcept -> bool final;
    auto SeedList() const noexcept -> ObjectList final;
    auto ServerAlias(const identifier::Notary& id) const noexcept
        -> UnallocatedCString final;
    auto ServerList() const noexcept -> ObjectList final;
    auto SetAccountAlias(const identifier::Account& id, std::string_view alias)
        const noexcept -> bool final;
    auto SetContactAlias(const identifier::Generic& id, std::string_view alias)
        const noexcept -> bool final;
    auto SetDefaultNym(const identifier::Nym& id) const noexcept -> bool final;
    auto SetDefaultSeed(const opentxs::crypto::SeedID& id) const noexcept
        -> bool final;
    auto SetNymAlias(const identifier::Nym& id, std::string_view alias)
        const noexcept -> bool final;
    auto SetPeerRequestTime(
        const identifier::Nym& nymID,
        const identifier::Generic& id,
        const otx::client::StorageBox box) const noexcept -> bool final;
    auto SetReadState(
        const identifier::Nym& nymId,
        const identifier::Generic& threadId,
        const identifier::Generic& itemId,
        const bool unread) const noexcept -> bool final;
    auto SetSeedAlias(const opentxs::crypto::SeedID& id, std::string_view alias)
        const noexcept -> bool final;
    auto SetServerAlias(const identifier::Notary& id, std::string_view alias)
        const noexcept -> bool final;
    auto SetThreadAlias(
        const identifier::Nym& nymId,
        const identifier::Generic& threadId,
        std::string_view alias) const noexcept -> bool final;
    auto SetUnitDefinitionAlias(
        const identifier::UnitDefinition& id,
        std::string_view alias) const noexcept -> bool final;
    auto Store(
        const identifier::Account& accountID,
        const UnallocatedCString& data,
        std::string_view alias,
        const identifier::Nym& ownerNym,
        const identifier::Nym& signerNym,
        const identifier::Nym& issuerNym,
        const identifier::Notary& server,
        const identifier::UnitDefinition& contract,
        const UnitType unit) const noexcept -> bool final;
    auto Store(
        const identifier::Nym& nymID,
        const UnitType type,
        const proto::HDAccount& data) const noexcept -> bool final;
    auto Store(
        const identifier::Nym& nymID,
        const identifier::Account& channelID,
        const proto::Bip47Channel& data) const noexcept -> bool final;
    auto Store(
        const identifier::Nym& nymID,
        const UnitType type,
        const proto::BlockchainEthereumAccountData& data) const noexcept
        -> bool final;
    auto Store(const proto::Contact& data) const noexcept -> bool final;
    auto Store(const proto::Context& data) const noexcept -> bool final;
    auto Store(const proto::Credential& data) const noexcept -> bool final;
    auto Store(const proto::Nym& data, std::string_view alias = {})
        const noexcept -> bool final;
    auto Store(const ReadView& data, std::string_view alias = {}) const noexcept
        -> bool final;
    auto Store(const identifier::Nym& nymID, const proto::Issuer& data)
        const noexcept -> bool final;
    auto Store(const identifier::Nym& nymID, const proto::PaymentWorkflow& data)
        const noexcept -> bool final;
    auto Store(
        const identifier::Nym& nymid,
        const identifier::Generic& threadid,
        const identifier::Generic& itemid,
        Time time,
        std::string_view alias,
        const UnallocatedCString& data,
        const otx::client::StorageBox box,
        const identifier::Generic& workflow) const noexcept -> bool final;
    auto Store(
        const identifier::Nym& nym,
        const identifier::Generic& thread,
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::TransactionHash& txid,
        const Time time) const noexcept -> bool final;
    auto Store(
        const proto::PeerReply& data,
        const identifier::Nym& nymid,
        const otx::client::StorageBox box) const noexcept -> bool final;
    auto Store(
        const proto::PeerRequest& data,
        const identifier::Nym& nymid,
        const otx::client::StorageBox box) const noexcept -> bool final;
    auto Store(const identifier::Nym& nym, const proto::Purse& purse)
        const noexcept -> bool final;
    auto Store(const opentxs::crypto::SeedID& id, const proto::Seed& data)
        const noexcept -> bool final;
    auto Store(const proto::ServerContract& data, std::string_view alias = {})
        const noexcept -> bool final;
    auto Store(const proto::Ciphertext& serialized) const noexcept
        -> bool final;
    auto Store(const proto::UnitDefinition& data, std::string_view alias = {})
        const noexcept -> bool final;
    auto ThreadList(const identifier::Nym& nymID, const bool unreadOnly)
        const noexcept -> ObjectList final;
    auto ThreadAlias(
        const identifier::Nym& nymID,
        const identifier::Generic& threadID) const noexcept
        -> UnallocatedCString final;
    auto UnaffiliatedBlockchainTransaction(
        const identifier::Nym& recipient,
        const opentxs::blockchain::block::TransactionHash& txid) const noexcept
        -> bool final;
    auto UnitDefinitionAlias(const identifier::UnitDefinition& id)
        const noexcept -> UnallocatedCString final;
    auto UnitDefinitionList() const noexcept -> ObjectList final;
    auto UnreadCount(
        const identifier::Nym& nymId,
        const identifier::Generic& threadId) const noexcept
        -> std::size_t final;

    auto DoGC(const opentxs::storage::tree::GCParams& params) noexcept -> bool;
    auto FinishGC(bool success) noexcept -> void;
    auto Start(std::shared_ptr<const api::internal::Session> api) noexcept
        -> void final;
    auto StartGC() const noexcept
        -> std::optional<opentxs::storage::tree::GCParams>;
    auto Upgrade() noexcept -> void final;

    Storage(
        const api::Crypto& crypto,
        const session::Factory& factory,
        const opentxs::storage::Config& config);
    Storage(const Storage&) = delete;
    Storage(Storage&&) = delete;
    auto operator=(const Storage&) -> Storage& = delete;
    auto operator=(Storage&&) -> Storage& = delete;

    ~Storage() final;

private:
    const api::Crypto& crypto_;
    const api::session::Factory& factory_;
    const std::chrono::seconds gc_interval_;
    mutable std::mutex write_lock_;
    mutable std::unique_ptr<opentxs::storage::tree::Root> root_;
    mutable std::atomic<opentxs::storage::Bucket> primary_bucket_;
    const opentxs::storage::Config config_;
    std::shared_ptr<opentxs::storage::driver::Plugin> plugin_p_;
    opentxs::storage::driver::Plugin& plugin_;

    auto root() const noexcept -> opentxs::storage::tree::Root*;
    auto Root() const noexcept -> const opentxs::storage::tree::Root&;
    auto verify_write_lock(const Lock& lock) const noexcept -> bool;

    auto blockchain_thread_item_id(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::TransactionHash& txid) const noexcept
        -> identifier::Generic;
    void InitBackup() final;
    void InitEncryptedBackup(opentxs::crypto::symmetric::Key& key) final;
    void InitPlugins();
    auto mutable_Root() const noexcept -> Editor<opentxs::storage::tree::Root>;
    void save(opentxs::storage::tree::Root* in, const Lock& lock)
        const noexcept;
    void start() final;
};
#pragma GCC diagnostic pop
}  // namespace opentxs::api::session::imp

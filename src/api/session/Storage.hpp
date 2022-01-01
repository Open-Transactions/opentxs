// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <cstdint>
#include <ctime>
#include <iosfwd>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "Proto.hpp"
#include "internal/api/session/Storage.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/contact/ClaimType.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Editor.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/otx/client/PaymentWorkflowState.hpp"
#include "opentxs/otx/client/PaymentWorkflowType.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "serialization/protobuf/PaymentWorkflowEnums.pb.h"
#include "storage/Config.hpp"

namespace opentxs
{
namespace api
{
namespace network
{
class Asio;
}  // namespace network

class Crypto;
}  // namespace api

namespace crypto
{
namespace key
{
class Symmetric;
}  // namespace key
}  // namespace crypto

namespace identifier
{
class Nym;
class Server;
class UnitDefinition;
}  // namespace identifier

namespace proto
{
class Bip47Channel;
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
namespace internal
{
class Multiplex;
}  // namespace internal
}  // namespace driver

class Config;
class Root;
}  // namespace storage

class Data;
class String;
}  // namespace opentxs

namespace opentxs::api::session::imp
{
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
class Storage final : public internal::Storage
{
public:
    auto AccountAlias(const Identifier& accountID) const -> std::string final;
    auto AccountList() const -> ObjectList final;
    auto AccountContract(const Identifier& accountID) const -> OTUnitID final;
    auto AccountIssuer(const Identifier& accountID) const -> OTNymID final;
    auto AccountOwner(const Identifier& accountID) const -> OTNymID final;
    auto AccountServer(const Identifier& accountID) const -> OTServerID final;
    auto AccountSigner(const Identifier& accountID) const -> OTNymID final;
    auto AccountUnit(const Identifier& accountID) const -> core::UnitType final;
    auto AccountsByContract(const identifier::UnitDefinition& contract) const
        -> std::pmr::set<OTIdentifier> final;
    auto AccountsByIssuer(const identifier::Nym& issuerNym) const
        -> std::pmr::set<OTIdentifier> final;
    auto AccountsByOwner(const identifier::Nym& ownerNym) const
        -> std::pmr::set<OTIdentifier> final;
    auto AccountsByServer(const identifier::Server& server) const
        -> std::pmr::set<OTIdentifier> final;
    auto AccountsByUnit(const core::UnitType unit) const
        -> std::pmr::set<OTIdentifier> final;
    auto Bip47Chain(const identifier::Nym& nymID, const Identifier& channelID)
        const -> core::UnitType final;
    auto Bip47ChannelsByChain(
        const identifier::Nym& nymID,
        const core::UnitType chain) const -> Bip47ChannelList final;
    auto BlockchainAccountList(
        const std::string& nymID,
        const core::UnitType type) const -> std::pmr::set<std::string> final;
    auto BlockchainAccountType(
        const std::string& nymID,
        const std::string& accountID) const -> core::UnitType final;
    auto BlockchainThreadMap(const identifier::Nym& nym, const Data& txid)
        const noexcept -> std::pmr::vector<OTIdentifier> final;
    auto BlockchainTransactionList(const identifier::Nym& nym) const noexcept
        -> std::pmr::vector<OTData> final;
    auto CheckTokenSpent(
        const identifier::Server& notary,
        const identifier::UnitDefinition& unit,
        const std::uint64_t series,
        const std::string& key) const -> bool final;
    auto ContactAlias(const std::string& id) const -> std::string final;
    auto ContactList() const -> ObjectList final;
    auto ContextList(const std::string& nymID) const -> ObjectList final;
    auto ContactOwnerNym(const std::string& nymID) const -> std::string final;
    void ContactSaveIndices() const final;
    auto ContactUpgradeLevel() const -> VersionNumber final;
    auto CreateThread(
        const std::string& nymID,
        const std::string& threadID,
        const std::pmr::set<std::string>& participants) const -> bool final;
    auto DeleteAccount(const std::string& id) const -> bool final;
    auto DefaultSeed() const -> std::string final;
    auto DeleteContact(const std::string& id) const -> bool final;
    auto DeletePaymentWorkflow(
        const std::string& nymID,
        const std::string& workflowID) const -> bool final;
    auto HashType() const -> std::uint32_t final;
    auto IssuerList(const std::string& nymID) const -> ObjectList final;
    auto Load(
        const std::string& accountID,
        std::string& output,
        std::string& alias,
        const bool checking = false) const -> bool final;
    auto Load(
        const std::string& nymID,
        const std::string& accountID,
        proto::HDAccount& output,
        const bool checking = false) const -> bool final;
    auto Load(
        const identifier::Nym& nymID,
        const Identifier& channelID,
        proto::Bip47Channel& output,
        const bool checking = false) const -> bool final;
    auto Load(
        const std::string& id,
        proto::Contact& contact,
        const bool checking = false) const -> bool final;
    auto Load(
        const std::string& id,
        proto::Contact& contact,
        std::string& alias,
        const bool checking = false) const -> bool final;
    auto Load(
        const std::string& nym,
        const std::string& id,
        proto::Context& context,
        const bool checking = false) const -> bool final;
    auto Load(
        const std::string& id,
        proto::Credential& cred,
        const bool checking = false) const -> bool final;
    auto Load(
        const identifier::Nym& id,
        proto::Nym& nym,
        const bool checking = false) const -> bool final;
    auto Load(
        const identifier::Nym& id,
        proto::Nym& nym,
        std::string& alias,
        const bool checking = false) const -> bool final;
    auto LoadNym(
        const identifier::Nym& id,
        AllocateOutput destination,
        const bool checking = false) const -> bool final;
    auto Load(
        const std::string& nymID,
        const std::string& id,
        proto::Issuer& issuer,
        const bool checking = false) const -> bool final;
    auto Load(
        const std::string& nymID,
        const std::string& workflowID,
        proto::PaymentWorkflow& workflow,
        const bool checking = false) const -> bool final;
    auto Load(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box,
        std::string& output,
        std::string& alias,
        const bool checking = false) const -> bool final;
    auto Load(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box,
        proto::PeerReply& request,
        const bool checking = false) const -> bool final;
    auto Load(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box,
        proto::PeerRequest& request,
        std::time_t& time,
        const bool checking = false) const -> bool final;
    auto Load(
        const identifier::Nym& nym,
        const identifier::Server& notary,
        const identifier::UnitDefinition& unit,
        proto::Purse& output,
        const bool checking) const -> bool final;
    auto Load(
        const std::string& id,
        proto::Seed& seed,
        const bool checking = false) const -> bool final;
    auto Load(
        const std::string& id,
        proto::Seed& seed,
        std::string& alias,
        const bool checking = false) const -> bool final;
    auto Load(
        const identifier::Server& id,
        proto::ServerContract& contract,
        const bool checking = false) const -> bool final;
    auto Load(
        const identifier::Server& id,
        proto::ServerContract& contract,
        std::string& alias,
        const bool checking = false) const -> bool final;
    auto Load(
        const std::string& nymId,
        const std::string& threadId,
        proto::StorageThread& thread) const -> bool final;
    auto Load(proto::Ciphertext& output, const bool checking = false) const
        -> bool final;
    auto Load(
        const identifier::UnitDefinition& id,
        proto::UnitDefinition& contract,
        const bool checking = false) const -> bool final;
    auto Load(
        const identifier::UnitDefinition& id,
        proto::UnitDefinition& contract,
        std::string& alias,
        const bool checking = false) const -> bool final;
    auto LocalNyms() const -> const std::pmr::set<std::string> final;
    void MapPublicNyms(NymLambda& lambda) const final;
    void MapServers(ServerLambda& lambda) const final;
    void MapUnitDefinitions(UnitLambda& lambda) const final;
    auto MarkTokenSpent(
        const identifier::Server& notary,
        const identifier::UnitDefinition& unit,
        const std::uint64_t series,
        const std::string& key) const -> bool final;
    auto MoveThreadItem(
        const std::string& nymId,
        const std::string& fromThreadID,
        const std::string& toThreadID,
        const std::string& itemID) const -> bool final;
    auto NymBoxList(const std::string& nymID, const StorageBox box) const
        -> ObjectList final;
    auto NymList() const -> ObjectList final;
    auto PaymentWorkflowList(const std::string& nymID) const
        -> ObjectList final;
    auto PaymentWorkflowLookup(
        const std::string& nymID,
        const std::string& sourceID) const -> std::string final;
    auto PaymentWorkflowsByAccount(
        const std::string& nymID,
        const std::string& accountID) const -> std::pmr::set<std::string> final;
    auto PaymentWorkflowsByState(
        const std::string& nymID,
        const otx::client::PaymentWorkflowType type,
        const otx::client::PaymentWorkflowState state) const
        -> std::pmr::set<std::string> final;
    auto PaymentWorkflowsByUnit(
        const std::string& nymID,
        const std::string& unitID) const -> std::pmr::set<std::string> final;
    auto PaymentWorkflowState(
        const std::string& nymID,
        const std::string& workflowID) const
        -> std::pair<
            otx::client::PaymentWorkflowType,
            otx::client::PaymentWorkflowState> final;
    auto RelabelThread(const std::string& threadID, const std::string& label)
        const -> bool final;
    auto RemoveBlockchainThreadItem(
        const identifier::Nym& nym,
        const Identifier& thread,
        const opentxs::blockchain::Type chain,
        const Data& txid) const noexcept -> bool final;
    auto RemoveNymBoxItem(
        const std::string& nymID,
        const StorageBox box,
        const std::string& itemID) const -> bool final;
    auto RemoveServer(const std::string& id) const -> bool final;
    auto RemoveThreadItem(
        const identifier::Nym& nym,
        const Identifier& thread,
        const std::string& id) const -> bool final;
    auto RemoveUnitDefinition(const std::string& id) const -> bool final;
    auto RenameThread(
        const std::string& nymId,
        const std::string& threadId,
        const std::string& newID) const -> bool final;
    void RunGC() const final;
    auto SeedList() const -> ObjectList final;
    auto ServerAlias(const std::string& id) const -> std::string final;
    auto ServerList() const -> ObjectList final;
    auto SetAccountAlias(const std::string& id, const std::string& alias) const
        -> bool final;
    auto SetContactAlias(const std::string& id, const std::string& alias) const
        -> bool final;
    auto SetDefaultSeed(const std::string& id) const -> bool final;
    auto SetNymAlias(const identifier::Nym& id, const std::string& alias) const
        -> bool final;
    auto SetPeerRequestTime(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box) const -> bool final;
    auto SetReadState(
        const std::string& nymId,
        const std::string& threadId,
        const std::string& itemId,
        const bool unread) const -> bool final;
    auto SetSeedAlias(const std::string& id, const std::string& alias) const
        -> bool final;
    auto SetServerAlias(const identifier::Server& id, const std::string& alias)
        const -> bool final;
    auto SetThreadAlias(
        const std::string& nymId,
        const std::string& threadId,
        const std::string& alias) const -> bool final;
    auto SetUnitDefinitionAlias(
        const identifier::UnitDefinition& id,
        const std::string& alias) const -> bool final;
    auto Store(
        const std::string& accountID,
        const std::string& data,
        const std::string& alias,
        const identifier::Nym& ownerNym,
        const identifier::Nym& signerNym,
        const identifier::Nym& issuerNym,
        const identifier::Server& server,
        const identifier::UnitDefinition& contract,
        const core::UnitType unit) const -> bool final;
    auto Store(
        const std::string& nymID,
        const contact::ClaimType type,
        const proto::HDAccount& data) const -> bool final;
    auto Store(
        const identifier::Nym& nymID,
        const Identifier& channelID,
        const proto::Bip47Channel& data) const -> bool final;
    auto Store(const proto::Contact& data) const -> bool final;
    auto Store(const proto::Context& data) const -> bool final;
    auto Store(const proto::Credential& data) const -> bool final;
    auto Store(
        const proto::Nym& data,
        const std::string& alias = std::string("")) const -> bool final;
    auto Store(const ReadView& data, const std::string& alias = std::string(""))
        const -> bool final;
    auto Store(const std::string& nymID, const proto::Issuer& data) const
        -> bool final;
    auto Store(const std::string& nymID, const proto::PaymentWorkflow& data)
        const -> bool final;
    auto Store(
        const std::string& nymid,
        const std::string& threadid,
        const std::string& itemid,
        const std::uint64_t time,
        const std::string& alias,
        const std::string& data,
        const StorageBox box,
        const std::string& account = std::string("")) const -> bool final;
    auto Store(
        const identifier::Nym& nym,
        const Identifier& thread,
        const opentxs::blockchain::Type chain,
        const Data& txid,
        const Time time) const noexcept -> bool final;
    auto Store(
        const proto::PeerReply& data,
        const std::string& nymid,
        const StorageBox box) const -> bool final;
    auto Store(
        const proto::PeerRequest& data,
        const std::string& nymid,
        const StorageBox box) const -> bool final;
    auto Store(const identifier::Nym& nym, const proto::Purse& purse) const
        -> bool final;
    auto Store(
        const proto::Seed& data,
        const std::string& alias = std::string("")) const -> bool final;
    auto Store(
        const proto::ServerContract& data,
        const std::string& alias = std::string("")) const -> bool final;
    auto Store(const proto::Ciphertext& serialized) const -> bool final;
    auto Store(
        const proto::UnitDefinition& data,
        const std::string& alias = std::string("")) const -> bool final;
    auto ThreadList(const std::string& nymID, const bool unreadOnly) const
        -> ObjectList final;
    auto ThreadAlias(const std::string& nymID, const std::string& threadID)
        const -> std::string final;
    auto UnaffiliatedBlockchainTransaction(
        const identifier::Nym& recipient,
        const Data& txid) const noexcept -> bool final;
    auto UnitDefinitionAlias(const std::string& id) const -> std::string final;
    auto UnitDefinitionList() const -> ObjectList final;
    auto UnreadCount(const std::string& nymId, const std::string& threadId)
        const -> std::size_t final;
    void UpgradeNyms() final;

    Storage(
        const api::Crypto& crypto,
        const network::Asio& asio,
        const Flag& running,
        const opentxs::storage::Config& config);

    ~Storage() final;

private:
    static const std::uint32_t HASH_TYPE;

    const api::Crypto& crypto_;
    const network::Asio& asio_;
    const Flag& running_;
    std::int64_t gc_interval_;
    mutable std::mutex write_lock_;
    mutable std::unique_ptr<opentxs::storage::Root> root_;
    mutable OTFlag primary_bucket_;
    const opentxs::storage::Config config_;
    std::unique_ptr<opentxs::storage::driver::internal::Multiplex> multiplex_p_;
    opentxs::storage::driver::internal::Multiplex& multiplex_;

    auto root() const -> opentxs::storage::Root*;
    auto Root() const -> const opentxs::storage::Root&;
    auto verify_write_lock(const Lock& lock) const -> bool;

    auto blockchain_thread_item_id(
        const opentxs::blockchain::Type chain,
        const Data& txid) const noexcept -> std::string;
    void Cleanup();
    void Cleanup_Storage();
    void CollectGarbage() const;
    void InitBackup() final;
    void InitEncryptedBackup(opentxs::crypto::key::Symmetric& key) final;
    void InitPlugins();
    auto mutable_Root() const -> Editor<opentxs::storage::Root>;
    void RunMapPublicNyms(NymLambda lambda) const;
    void RunMapServers(ServerLambda lambda) const;
    void RunMapUnits(UnitLambda lambda) const;
    void save(opentxs::storage::Root* in, const Lock& lock) const;
    void start() final;

    Storage(const Storage&) = delete;
    Storage(Storage&&) = delete;
    auto operator=(const Storage&) -> Storage& = delete;
    auto operator=(Storage&&) -> Storage& = delete;
};
}  // namespace opentxs::api::session::imp

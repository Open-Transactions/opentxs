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

#ifndef OPENTXS_API_STORAGE_IMPLEMENTATION_STORAGE_HPP
#define OPENTXS_API_STORAGE_IMPLEMENTATION_STORAGE_HPP

#include "Internal.hpp"

namespace opentxs::api::storage::implementation
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
class Storage : public opentxs::api::storage::StorageInternal
{
public:
    ObjectList AccountList() const override;
    OTIdentifier AccountContract(const Identifier& accountID) const override;
    OTIdentifier AccountIssuer(const Identifier& accountID) const override;
    OTIdentifier AccountOwner(const Identifier& accountID) const override;
    OTIdentifier AccountServer(const Identifier& accountID) const override;
    OTIdentifier AccountSigner(const Identifier& accountID) const override;
    proto::ContactItemType AccountUnit(
        const Identifier& accountID) const override;
    std::set<OTIdentifier> AccountsByContract(
        const Identifier& contract) const override;
    std::set<OTIdentifier> AccountsByIssuer(
        const Identifier& issuerNym) const override;
    std::set<OTIdentifier> AccountsByOwner(
        const Identifier& ownerNym) const override;
    std::set<OTIdentifier> AccountsByServer(
        const Identifier& server) const override;
    std::set<OTIdentifier> AccountsByUnit(
        const proto::ContactItemType unit) const override;
    std::set<proto::ContactItemType> Bip47ChainList(
        const std::string& nymID) const override;
    std::set<Bip47ChannelID> Bip47ChannelList(
        const std::string& nymID,
        const std::string& contactID,
        const proto::ContactItemType chain) const override;
    std::set<std::string> Bip47ContactList(
        const std::string& nymID,
        const proto::ContactItemType chain) const override;
    std::set<std::string> BlockchainAccountList(
        const std::string& nymID,
        const proto::ContactItemType type) const override;
    std::string BlockchainAddressOwner(
        proto::ContactItemType chain,
        std::string address) const override;
    ObjectList BlockchainTransactionList() const override;
    std::string ContactAlias(const std::string& id) const override;
    ObjectList ContactList() const override;
    ObjectList ContextList(const std::string& nymID) const override;
    std::string ContactOwnerNym(const std::string& nymID) const override;
    void ContactSaveIndices() const override;
    std::uint32_t ContactUpgradeLevel() const override;
    bool CreateThread(
        const std::string& nymID,
        const std::string& threadID,
        const std::set<std::string>& participants) const override;
    bool DeleteAccount(const std::string& id) const override;
    std::string DefaultSeed() const override;
    bool DeleteContact(const std::string& id) const override;
    bool DeletePaymentWorkflow(
        const std::string& nymID,
        const std::string& workflowID) const override;
    std::uint32_t HashType() const override;
    ObjectList IssuerList(const std::string& nymID) const override;
    bool Load(
        const std::string& accountID,
        std::string& output,
        std::string& alias,
        const bool checking = false) const override;
    bool Load(
        const std::string& nymID,
        const std::string& accountID,
        std::shared_ptr<proto::Bip44Account>& output,
        const bool checking = false) const override;
    bool Load(
        const std::string& nymID,
        const std::string& paymentCode,
        std::shared_ptr<proto::Bip47Context>& output,
        const bool checking = false) const override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::BlockchainTransaction>& transaction,
        const bool checking = false) const override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Contact>& contact,
        const bool checking = false) const override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Contact>& contact,
        std::string& alias,
        const bool checking = false) const override;
    bool Load(
        const std::string& nym,
        const std::string& id,
        std::shared_ptr<proto::Context>& context,
        const bool checking = false) const override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Credential>& cred,
        const bool checking = false) const override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::CredentialIndex>& nym,
        const bool checking = false) const override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::CredentialIndex>& nym,
        std::string& alias,
        const bool checking = false) const override;
    bool Load(
        const std::string& nymID,
        const std::string& id,
        std::shared_ptr<proto::Issuer>& issuer,
        const bool checking = false) const override;
    bool Load(
        const std::string& nymID,
        const std::string& workflowID,
        std::shared_ptr<proto::PaymentWorkflow>& workflow,
        const bool checking = false) const override;
    bool Load(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box,
        std::string& output,
        std::string& alias,
        const bool checking = false) const override;
    bool Load(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box,
        std::shared_ptr<proto::PeerReply>& request,
        const bool checking = false) const override;
    bool Load(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box,
        std::shared_ptr<proto::PeerRequest>& request,
        std::time_t& time,
        const bool checking = false) const override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Seed>& seed,
        const bool checking = false) const override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Seed>& seed,
        std::string& alias,
        const bool checking = false) const override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::ServerContract>& contract,
        const bool checking = false) const override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::ServerContract>& contract,
        std::string& alias,
        const bool checking = false) const override;
    bool Load(
        const std::string& nymId,
        const std::string& threadId,
        std::shared_ptr<proto::StorageThread>& thread) const override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::UnitDefinition>& contract,
        const bool checking = false) const override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::UnitDefinition>& contract,
        std::string& alias,
        const bool checking = false) const override;
    const std::set<std::string> LocalNyms() const override;
    void MapPublicNyms(NymLambda& lambda) const override;
    void MapServers(ServerLambda& lambda) const override;
    void MapUnitDefinitions(UnitLambda& lambda) const override;
    bool MoveThreadItem(
        const std::string& nymId,
        const std::string& fromThreadID,
        const std::string& toThreadID,
        const std::string& itemID) const override;
    ObjectList NymBoxList(const std::string& nymID, const StorageBox box)
        const override;
    ObjectList NymList() const override;
    ObjectList PaymentWorkflowList(const std::string& nymID) const override;
    std::string PaymentWorkflowLookup(
        const std::string& nymID,
        const std::string& sourceID) const override;
    std::set<std::string> PaymentWorkflowsByAccount(
        const std::string& nymID,
        const std::string& accountID) const override;
    std::set<std::string> PaymentWorkflowsByState(
        const std::string& nymID,
        const proto::PaymentWorkflowType type,
        const proto::PaymentWorkflowState state) const override;
    std::set<std::string> PaymentWorkflowsByUnit(
        const std::string& nymID,
        const std::string& unitID) const override;
    std::pair<proto::PaymentWorkflowType, proto::PaymentWorkflowState>
    PaymentWorkflowState(
        const std::string& nymID,
        const std::string& workflowID) const override;
    bool RelabelThread(const std::string& threadID, const std::string& label)
        const override;
    bool RemoveNymBoxItem(
        const std::string& nymID,
        const StorageBox box,
        const std::string& itemID) const override;
    bool RemoveServer(const std::string& id) const override;
    bool RemoveUnitDefinition(const std::string& id) const override;
    bool RenameThread(
        const std::string& nymId,
        const std::string& threadId,
        const std::string& newID) const override;
    void RunGC() const override;
    std::string ServerAlias(const std::string& id) const override;
    ObjectList ServerList() const override;
    bool SetContactAlias(const std::string& id, const std::string& alias)
        const override;
    bool SetDefaultSeed(const std::string& id) const override;
    bool SetNymAlias(const std::string& id, const std::string& alias)
        const override;
    bool SetPeerRequestTime(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box) const override;
    bool SetReadState(
        const std::string& nymId,
        const std::string& threadId,
        const std::string& itemId,
        const bool unread) const override;
    bool SetSeedAlias(const std::string& id, const std::string& alias)
        const override;
    bool SetServerAlias(const std::string& id, const std::string& alias)
        const override;
    bool SetThreadAlias(
        const std::string& nymId,
        const std::string& threadId,
        const std::string& alias) const override;
    bool SetUnitDefinitionAlias(const std::string& id, const std::string& alias)
        const override;
    bool Store(
        const std::string& accountID,
        const std::string& data,
        const std::string& alias,
        const Identifier& ownerNym,
        const Identifier& signerNym,
        const Identifier& issuerNym,
        const Identifier& server,
        const Identifier& contract,
        const proto::ContactItemType unit) const override;
    bool Store(
        const std::string& nymID,
        const proto::ContactItemType type,
        const proto::Bip44Account& data) const override;
    bool Store(const std::string& nymID, const proto::Bip47Context& data)
        const override;
    bool Store(const proto::BlockchainTransaction& data) const override;
    bool Store(const proto::Contact& data) const override;
    bool Store(const proto::Context& data) const override;
    bool Store(const proto::Credential& data) const override;
    bool Store(
        const proto::CredentialIndex& data,
        const std::string& alias = std::string("")) const override;
    bool Store(const std::string& nymID, const proto::Issuer& data)
        const override;
    bool Store(const std::string& nymID, const proto::PaymentWorkflow& data)
        const override;
    bool Store(
        const std::string& nymid,
        const std::string& threadid,
        const std::string& itemid,
        const std::uint64_t time,
        const std::string& alias,
        const std::string& data,
        const StorageBox box,
        const std::string& account = std::string("")) const override;
    bool Store(
        const proto::PeerReply& data,
        const std::string& nymid,
        const StorageBox box) const override;
    bool Store(
        const proto::PeerRequest& data,
        const std::string& nymid,
        const StorageBox box) const override;
    bool Store(
        const proto::Seed& data,
        const std::string& alias = std::string("")) const override;
    bool Store(
        const proto::ServerContract& data,
        const std::string& alias = std::string("")) const override;
    bool Store(
        const proto::UnitDefinition& data,
        const std::string& alias = std::string("")) const override;
    ObjectList ThreadList(const std::string& nymID, const bool unreadOnly)
        const override;
    std::string ThreadAlias(
        const std::string& nymID,
        const std::string& threadID) const override;
    std::string UnitDefinitionAlias(const std::string& id) const override;
    ObjectList UnitDefinitionList() const override;
    std::size_t UnreadCount(
        const std::string& nymId,
        const std::string& threadId) const override;
    void UpgradeNyms() override;

    ~Storage();

private:
    friend Factory;

    static const std::uint32_t HASH_TYPE;

    const Flag& running_;
    std::int64_t gc_interval_{std::numeric_limits<std::int64_t>::max()};
    mutable std::mutex write_lock_;
    mutable std::unique_ptr<opentxs::storage::Root> root_;
    mutable OTFlag primary_bucket_;
    std::vector<std::thread> background_threads_;
    const StorageConfig config_;
    std::unique_ptr<StorageMultiplex> multiplex_p_;
    StorageMultiplex& multiplex_;

    opentxs::storage::Root* root() const;
    const opentxs::storage::Root& Root() const;
    bool verify_write_lock(const Lock& lock) const;

    void Cleanup();
    void Cleanup_Storage();
    void CollectGarbage() const;
    void InitBackup() override;
    void InitEncryptedBackup(std::unique_ptr<SymmetricKey>& key) override;
    void InitPlugins();
    Editor<opentxs::storage::Root> mutable_Root() const;
    void RunMapPublicNyms(NymLambda lambda) const;
    void RunMapServers(ServerLambda lambda) const;
    void RunMapUnits(UnitLambda lambda) const;
    void save(opentxs::storage::Root* in, const Lock& lock) const;
    void start() override;

    Storage(
        const Flag& running,
        const StorageConfig& config,
        const String& primary,
        const bool migrate,
        const String& previous,
        const Digest& hash,
        const Random& random);
    Storage(const Storage&) = delete;
    Storage(Storage&&) = delete;
    Storage& operator=(const Storage&) = delete;
    Storage& operator=(Storage&&) = delete;
};
}  // namespace opentxs::api::storage::implementation
#endif  // OPENTXS_API_STORAGE_IMPLEMENTATION_STORAGE_HPP

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstdint>
#include <ctime>
#include <memory>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/storage/Types.internal.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Session;
}  // namespace internal

class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class TransactionHash;
}  // namespace block
}  // namespace blockchain

namespace crypto
{
namespace symmetric
{
class Key;
}  // namespace symmetric
}  // namespace crypto

namespace identifier
{
class Account;
class Generic;
class Nym;
class Notary;
class UnitDefinition;
}  // namespace identifier

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

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::internal
{
using opentxs::storage::ErrorReporting;

class Storage : virtual public session::Storage
{
public:
    using Bip47ChannelList = UnallocatedSet<identifier::Account>;

    virtual auto AccountAlias(const identifier::Account& accountID)
        const noexcept -> UnallocatedCString = 0;
    virtual auto AccountList() const noexcept -> ObjectList = 0;
    virtual auto AccountContract(const identifier::Account& accountID)
        const noexcept -> identifier::UnitDefinition = 0;
    virtual auto AccountIssuer(const identifier::Account& accountID)
        const noexcept -> identifier::Nym = 0;
    virtual auto AccountOwner(const identifier::Account& accountID)
        const noexcept -> identifier::Nym = 0;
    virtual auto AccountServer(const identifier::Account& accountID)
        const noexcept -> identifier::Notary = 0;
    virtual auto AccountSigner(const identifier::Account& accountID)
        const noexcept -> identifier::Nym = 0;
    virtual auto AccountUnit(
        const identifier::Account& accountID) const noexcept -> UnitType = 0;
    virtual auto AccountsByContract(const identifier::UnitDefinition& contract)
        const noexcept -> UnallocatedSet<identifier::Account> = 0;
    virtual auto AccountsByIssuer(const identifier::Nym& issuerNym)
        const noexcept -> UnallocatedSet<identifier::Account> = 0;
    virtual auto AccountsByOwner(const identifier::Nym& ownerNym) const noexcept
        -> UnallocatedSet<identifier::Account> = 0;
    virtual auto AccountsByServer(const identifier::Notary& server)
        const noexcept -> UnallocatedSet<identifier::Account> = 0;
    virtual auto AccountsByUnit(const UnitType unit) const noexcept
        -> UnallocatedSet<identifier::Account> = 0;
    virtual auto Bip47Chain(
        const identifier::Nym& nymID,
        const identifier::Account& channelID) const noexcept -> UnitType = 0;
    virtual auto Bip47ChannelsByChain(
        const identifier::Nym& nymID,
        const UnitType chain) const noexcept -> Bip47ChannelList = 0;
    virtual auto BlockchainAccountList(
        const identifier::Nym& nymID,
        const UnitType type) const noexcept
        -> UnallocatedSet<identifier::Account> = 0;
    virtual auto BlockchainEthereumAccountList(
        const identifier::Nym& nymID,
        const UnitType type) const noexcept
        -> UnallocatedSet<identifier::Account> = 0;
    virtual auto BlockchainSubaccountAccountType(
        const identifier::Nym& owner,
        const identifier::Account& id) const noexcept -> UnitType = 0;
    virtual auto BlockchainThreadMap(
        const identifier::Nym& nym,
        const opentxs::blockchain::block::TransactionHash& txid) const noexcept
        -> UnallocatedVector<identifier::Generic> = 0;
    virtual auto BlockchainTransactionList(const identifier::Nym& nym)
        const noexcept -> UnallocatedVector<ByteArray> = 0;
    virtual auto CheckTokenSpent(
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit,
        const std::uint64_t series,
        const UnallocatedCString& key) const noexcept -> bool = 0;
    virtual auto ContactAlias(const identifier::Generic& id) const noexcept
        -> UnallocatedCString = 0;
    virtual auto ContactList() const noexcept -> ObjectList = 0;
    virtual auto ContactOwnerNym(const identifier::Nym& nym) const noexcept
        -> identifier::Generic = 0;
    virtual auto ContactSaveIndices() const noexcept -> void = 0;
    virtual auto ContactUpgradeLevel() const noexcept -> VersionNumber = 0;
    virtual auto ContextList(const identifier::Nym& nymID) const noexcept
        -> ObjectList = 0;
    virtual auto CreateThread(
        const identifier::Nym& nymID,
        const identifier::Generic& threadID,
        const UnallocatedSet<identifier::Generic>& participants) const noexcept
        -> bool = 0;
    virtual auto DeleteAccount(const identifier::Account& id) const noexcept
        -> bool = 0;
    virtual auto DeleteContact(const identifier::Generic& id) const noexcept
        -> bool = 0;
    virtual auto DeletePaymentWorkflow(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID) const noexcept -> bool = 0;
    virtual auto InitBackup() -> void = 0;
    virtual auto InitEncryptedBackup(opentxs::crypto::symmetric::Key& key)
        -> void = 0;
    auto Internal() const noexcept -> const internal::Storage& final
    {
        return *this;
    }
    virtual auto IssuerList(const identifier::Nym& nymID) const noexcept
        -> ObjectList = 0;
    virtual auto Load(
        const identifier::Account& accountID,
        UnallocatedCString& output,
        UnallocatedCString& alias,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nymID,
        const identifier::Account& channelID,
        proto::Bip47Channel& output,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nymID,
        const identifier::Account& accountID,
        proto::BlockchainEthereumAccountData& output,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        proto::Ciphertext& output,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const identifier::Generic& id,
        proto::Contact& contact,
        UnallocatedCString& alias,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const identifier::Generic& id,
        proto::Contact& contact,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const identifier::Nym& localNym,
        const identifier::Nym& remoteNym,
        proto::Context& context,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const identifier::Generic& id,
        proto::Credential& cred,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nymID,
        const identifier::Account& accountID,
        proto::HDAccount& output,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nymID,
        const identifier::Nym& id,
        proto::Issuer& issuer,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nymID,
        const identifier::Generic& id,
        const otx::client::StorageBox box,
        UnallocatedCString& output,
        UnallocatedCString& alias,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const identifier::Nym& id,
        proto::Nym& nym,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const identifier::Nym& id,
        proto::Nym& nym,
        UnallocatedCString& alias,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID,
        proto::PaymentWorkflow& workflow,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nymID,
        const identifier::Generic& id,
        const otx::client::StorageBox box,
        proto::PeerReply& request,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nymID,
        const identifier::Generic& id,
        const otx::client::StorageBox box,
        proto::PeerRequest& request,
        Time& time,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nym,
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit,
        proto::Purse& output,
        ErrorReporting checking) const noexcept -> bool = 0;
    virtual auto Load(
        const opentxs::crypto::SeedID& id,
        proto::Seed& seed,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const opentxs::crypto::SeedID& id,
        proto::Seed& seed,
        UnallocatedCString& alias,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const identifier::Notary& id,
        proto::ServerContract& contract,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const identifier::Notary& id,
        proto::ServerContract& contract,
        UnallocatedCString& alias,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nymId,
        const identifier::Generic& threadId,
        proto::StorageThread& thread) const noexcept -> bool = 0;
    virtual auto Load(
        const identifier::UnitDefinition& id,
        proto::UnitDefinition& contract,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto Load(
        const identifier::UnitDefinition& id,
        proto::UnitDefinition& contract,
        UnallocatedCString& alias,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto LoadNym(
        const identifier::Nym& id,
        Writer&& destination,
        ErrorReporting checking = ErrorReporting::silent) const noexcept
        -> bool = 0;
    virtual auto MarkTokenSpent(
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit,
        const std::uint64_t series,
        const UnallocatedCString& key) const noexcept -> bool = 0;
    virtual auto MoveThreadItem(
        const identifier::Nym& nymId,
        const identifier::Generic& fromThreadID,
        const identifier::Generic& toThreadID,
        const identifier::Generic& itemID) const noexcept -> bool = 0;
    virtual auto NymBoxList(
        const identifier::Nym& nymID,
        const otx::client::StorageBox box) const noexcept -> ObjectList = 0;
    virtual auto PaymentWorkflowList(
        const identifier::Nym& nymID) const noexcept -> ObjectList = 0;
    virtual auto PaymentWorkflowLookup(
        const identifier::Nym& nymID,
        const identifier::Generic& sourceID) const noexcept
        -> identifier::Generic = 0;
    virtual auto PaymentWorkflowsByAccount(
        const identifier::Nym& nymID,
        const identifier::Account& accountID) const noexcept
        -> UnallocatedSet<identifier::Generic> = 0;
    virtual auto PaymentWorkflowsByState(
        const identifier::Nym& nymID,
        const otx::client::PaymentWorkflowType type,
        const otx::client::PaymentWorkflowState state) const noexcept
        -> UnallocatedSet<identifier::Generic> = 0;
    virtual auto PaymentWorkflowsByUnit(
        const identifier::Nym& nymID,
        const identifier::UnitDefinition& unitID) const noexcept
        -> UnallocatedSet<identifier::Generic> = 0;
    virtual auto PaymentWorkflowState(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID) const noexcept
        -> std::pair<
            otx::client::PaymentWorkflowType,
            otx::client::PaymentWorkflowState> = 0;
    virtual auto RelabelThread(
        const identifier::Generic& threadID,
        const UnallocatedCString& label) const noexcept -> bool = 0;
    virtual auto RemoveBlockchainThreadItem(
        const identifier::Nym& nym,
        const identifier::Generic& thread,
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::TransactionHash& txid) const noexcept
        -> bool = 0;
    virtual auto RemoveNymBoxItem(
        const identifier::Nym& nymID,
        const otx::client::StorageBox box,
        const identifier::Generic& itemID) const noexcept -> bool = 0;
    virtual auto RemoveServer(const identifier::Notary& id) const noexcept
        -> bool = 0;
    virtual auto RemoveThreadItem(
        const identifier::Nym& nym,
        const identifier::Generic& thread,
        const identifier::Generic& id) const noexcept -> bool = 0;
    virtual auto RemoveUnitDefinition(
        const identifier::UnitDefinition& id) const noexcept -> bool = 0;
    virtual auto RenameThread(
        const identifier::Nym& nymId,
        const identifier::Generic& threadId,
        const identifier::Generic& newID) const noexcept -> bool = 0;
    virtual auto SetAccountAlias(
        const identifier::Account& id,
        std::string_view alias) const noexcept -> bool = 0;
    virtual auto SetContactAlias(
        const identifier::Generic& id,
        std::string_view alias) const noexcept -> bool = 0;
    virtual auto SetDefaultNym(const identifier::Nym& id) const noexcept
        -> bool = 0;
    virtual auto SetDefaultSeed(
        const opentxs::crypto::SeedID& id) const noexcept -> bool = 0;
    virtual auto SetNymAlias(const identifier::Nym& id, std::string_view alias)
        const noexcept -> bool = 0;
    virtual auto SetPeerRequestTime(
        const identifier::Nym& nymID,
        const identifier::Generic& id,
        const otx::client::StorageBox box) const noexcept -> bool = 0;
    virtual auto SetReadState(
        const identifier::Nym& nymId,
        const identifier::Generic& threadId,
        const identifier::Generic& itemId,
        const bool unread) const noexcept -> bool = 0;
    virtual auto SetSeedAlias(
        const opentxs::crypto::SeedID& id,
        std::string_view alias) const noexcept -> bool = 0;
    virtual auto SetServerAlias(
        const identifier::Notary& id,
        std::string_view alias) const noexcept -> bool = 0;
    virtual auto SetThreadAlias(
        const identifier::Nym& nymId,
        const identifier::Generic& threadId,
        std::string_view alias) const noexcept -> bool = 0;
    virtual auto SetUnitDefinitionAlias(
        const identifier::UnitDefinition& id,
        std::string_view alias) const noexcept -> bool = 0;
    virtual auto start() -> void = 0;
    virtual auto Store(
        const identifier::Account& accountID,
        const UnallocatedCString& data,
        std::string_view alias,
        const identifier::Nym& ownerNym,
        const identifier::Nym& signerNym,
        const identifier::Nym& issuerNym,
        const identifier::Notary& server,
        const identifier::UnitDefinition& contract,
        const UnitType unit) const noexcept -> bool = 0;
    virtual auto Store(
        const identifier::Nym& nymID,
        const identifier::Account& channelID,
        const proto::Bip47Channel& data) const noexcept -> bool = 0;
    virtual auto Store(
        const identifier::Nym& nymID,
        const UnitType type,
        const proto::BlockchainEthereumAccountData& data) const noexcept
        -> bool = 0;
    virtual auto Store(const proto::Ciphertext& serialized) const noexcept
        -> bool = 0;
    virtual auto Store(const proto::Contact& data) const noexcept -> bool = 0;
    virtual auto Store(const proto::Context& data) const noexcept -> bool = 0;
    virtual auto Store(const proto::Credential& data) const noexcept
        -> bool = 0;
    virtual auto Store(
        const identifier::Nym& nymID,
        const UnitType type,
        const proto::HDAccount& data) const noexcept -> bool = 0;
    virtual auto Store(const identifier::Nym& nymID, const proto::Issuer& data)
        const noexcept -> bool = 0;
    virtual auto Store(const proto::Nym& data, std::string_view alias = {})
        const noexcept -> bool = 0;
    virtual auto Store(const ReadView& data, std::string_view alias = {})
        const noexcept -> bool = 0;
    virtual auto Store(
        const identifier::Nym& nymid,
        const identifier::Generic& threadid,
        const identifier::Generic& itemid,
        const std::uint64_t time,
        std::string_view alias,
        const UnallocatedCString& data,
        const otx::client::StorageBox box,
        const identifier::Generic& workflow) const noexcept -> bool = 0;
    virtual auto Store(
        const identifier::Nym& nym,
        const identifier::Generic& thread,
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::TransactionHash& txid,
        const Time time) const noexcept -> bool = 0;
    virtual auto Store(
        const identifier::Nym& nymID,
        const proto::PaymentWorkflow& data) const noexcept -> bool = 0;
    virtual auto Store(
        const proto::PeerReply& data,
        const identifier::Nym& nymid,
        const otx::client::StorageBox box) const noexcept -> bool = 0;
    virtual auto Store(
        const proto::PeerRequest& data,
        const identifier::Nym& nymid,
        const otx::client::StorageBox box) const noexcept -> bool = 0;
    virtual auto Store(const identifier::Nym& nym, const proto::Purse& purse)
        const noexcept -> bool = 0;
    virtual auto Store(
        const opentxs::crypto::SeedID& id,
        const proto::Seed& data) const noexcept -> bool = 0;
    virtual auto Store(
        const proto::ServerContract& data,
        std::string_view alias = {}) const noexcept -> bool = 0;
    virtual auto Store(
        const proto::UnitDefinition& data,
        std::string_view alias = {}) const noexcept -> bool = 0;
    virtual auto ThreadList(const identifier::Nym& nymID, const bool unreadOnly)
        const noexcept -> ObjectList = 0;
    virtual auto ThreadAlias(
        const identifier::Nym& nymID,
        const identifier::Generic& threadID) const noexcept
        -> UnallocatedCString = 0;
    virtual auto UnaffiliatedBlockchainTransaction(
        const identifier::Nym& recipient,
        const opentxs::blockchain::block::TransactionHash& txid) const noexcept
        -> bool = 0;
    virtual auto UnreadCount(
        const identifier::Nym& nymId,
        const identifier::Generic& threadId) const noexcept -> std::size_t = 0;

    auto Internal() noexcept -> internal::Storage& final { return *this; }
    virtual auto Start(
        std::shared_ptr<const api::internal::Session> api) noexcept -> void = 0;
    virtual auto Upgrade() noexcept -> void = 0;

    Storage(const Storage&) = delete;
    Storage(Storage&&) = delete;
    auto operator=(const Storage&) -> Storage& = delete;
    auto operator=(Storage&&) -> Storage& = delete;

    ~Storage() override = default;

protected:
    Storage() = default;
};
}  // namespace opentxs::api::session::internal

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstdint>
#include <ctime>
#include <functional>
#include <memory>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace internal
{
class Storage;
}  // namespace internal
}  // namespace session
}  // namespace api

namespace blockchain
{
namespace block
{
class TransactionHash;
}  // namespace block
}  // namespace blockchain

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

using NymLambda = std::function<void(const proto::Nym&)>;
using ServerLambda = std::function<void(const proto::ServerContract&)>;
using UnitLambda = std::function<void(const proto::UnitDefinition&)>;

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session
{
class Storage
{
public:
    using Bip47ChannelList = UnallocatedSet<identifier::Account>;

    virtual auto AccountAlias(const identifier::Account& accountID) const
        -> UnallocatedCString = 0;
    virtual auto AccountList() const -> ObjectList = 0;
    virtual auto AccountContract(const identifier::Account& accountID) const
        -> identifier::UnitDefinition = 0;
    virtual auto AccountIssuer(const identifier::Account& accountID) const
        -> identifier::Nym = 0;
    virtual auto AccountOwner(const identifier::Account& accountID) const
        -> identifier::Nym = 0;
    virtual auto AccountServer(const identifier::Account& accountID) const
        -> identifier::Notary = 0;
    virtual auto AccountSigner(const identifier::Account& accountID) const
        -> identifier::Nym = 0;
    virtual auto AccountUnit(const identifier::Account& accountID) const
        -> UnitType = 0;
    virtual auto AccountsByContract(const identifier::UnitDefinition& contract)
        const -> UnallocatedSet<identifier::Account> = 0;
    virtual auto AccountsByIssuer(const identifier::Nym& issuerNym) const
        -> UnallocatedSet<identifier::Account> = 0;
    virtual auto AccountsByOwner(const identifier::Nym& ownerNym) const
        -> UnallocatedSet<identifier::Account> = 0;
    virtual auto AccountsByServer(const identifier::Notary& server) const
        -> UnallocatedSet<identifier::Account> = 0;
    virtual auto AccountsByUnit(const UnitType unit) const
        -> UnallocatedSet<identifier::Account> = 0;
    virtual auto Bip47Chain(
        const identifier::Nym& nymID,
        const identifier::Account& channelID) const -> UnitType = 0;
    virtual auto Bip47ChannelsByChain(
        const identifier::Nym& nymID,
        const UnitType chain) const -> Bip47ChannelList = 0;
    virtual auto BlockchainAccountList(
        const identifier::Nym& nymID,
        const UnitType type) const -> UnallocatedSet<identifier::Account> = 0;
    virtual auto BlockchainSubaccountAccountType(
        const identifier::Nym& owner,
        const identifier::Account& id) const -> UnitType = 0;
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
        const UnallocatedCString& key) const -> bool = 0;
    virtual auto ContactAlias(const UnallocatedCString& id) const
        -> UnallocatedCString = 0;
    virtual auto ContactList() const -> ObjectList = 0;
    virtual auto ContextList(const identifier::Nym& nymID) const
        -> ObjectList = 0;
    virtual auto ContactOwnerNym(const identifier::Nym& nym) const
        -> identifier::Generic = 0;
    virtual void ContactSaveIndices() const = 0;
    virtual auto ContactUpgradeLevel() const -> VersionNumber = 0;
    virtual auto CreateThread(
        const identifier::Nym& nymID,
        const UnallocatedCString& threadID,
        const UnallocatedSet<UnallocatedCString>& participants) const
        -> bool = 0;
    virtual auto DeleteAccount(const UnallocatedCString& id) const -> bool = 0;
    virtual auto DefaultNym() const -> identifier::Nym = 0;
    virtual auto DefaultSeed() const -> opentxs::crypto::SeedID = 0;
    virtual auto DeleteContact(const UnallocatedCString& id) const -> bool = 0;
    virtual auto DeletePaymentWorkflow(
        const identifier::Nym& nymID,
        const UnallocatedCString& workflowID) const -> bool = 0;
    virtual auto HashType() const -> std::uint32_t = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Storage& = 0;
    virtual auto IssuerList(const identifier::Nym& nymID) const
        -> ObjectList = 0;
    virtual auto Load(
        const UnallocatedCString& accountID,
        UnallocatedCString& output,
        UnallocatedCString& alias,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nymID,
        const identifier::Account& accountID,
        proto::HDAccount& output,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nymID,
        const identifier::Account& channelID,
        proto::Bip47Channel& output,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const UnallocatedCString& id,
        proto::Contact& contact,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const UnallocatedCString& id,
        proto::Contact& contact,
        UnallocatedCString& alias,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::Nym& localNym,
        const identifier::Nym& remoteNym,
        proto::Context& context,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::Generic& id,
        proto::Credential& cred,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::Nym& id,
        proto::Nym& nym,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::Nym& id,
        proto::Nym& nym,
        UnallocatedCString& alias,
        const bool checking = false) const -> bool = 0;
    virtual auto LoadNym(
        const identifier::Nym& id,
        Writer&& destination,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nymID,
        const UnallocatedCString& id,
        proto::Issuer& issuer,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nymID,
        const UnallocatedCString& workflowID,
        proto::PaymentWorkflow& workflow,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nymID,
        const UnallocatedCString& id,
        const otx::client::StorageBox box,
        UnallocatedCString& output,
        UnallocatedCString& alias,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nymID,
        const identifier::Generic& id,
        const otx::client::StorageBox box,
        proto::PeerReply& request,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nymID,
        const identifier::Generic& id,
        const otx::client::StorageBox box,
        proto::PeerRequest& request,
        Time& time,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nym,
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit,
        proto::Purse& output,
        const bool checking) const -> bool = 0;
    virtual auto Load(
        const opentxs::crypto::SeedID& id,
        proto::Seed& seed,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const opentxs::crypto::SeedID& id,
        proto::Seed& seed,
        UnallocatedCString& alias,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::Notary& id,
        proto::ServerContract& contract,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::Notary& id,
        proto::ServerContract& contract,
        UnallocatedCString& alias,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nymId,
        const UnallocatedCString& threadId,
        proto::StorageThread& thread) const -> bool = 0;
    virtual auto Load(proto::Ciphertext& output, const bool checking = false)
        const -> bool = 0;
    virtual auto Load(
        const identifier::UnitDefinition& id,
        proto::UnitDefinition& contract,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::UnitDefinition& id,
        proto::UnitDefinition& contract,
        UnallocatedCString& alias,
        const bool checking = false) const -> bool = 0;
    virtual auto LocalNyms() const -> Set<identifier::Nym> = 0;
    virtual void MapPublicNyms(NymLambda& lambda) const = 0;
    virtual void MapServers(ServerLambda& lambda) const = 0;
    virtual void MapUnitDefinitions(UnitLambda& lambda) const = 0;
    virtual auto MarkTokenSpent(
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit,
        const std::uint64_t series,
        const UnallocatedCString& key) const -> bool = 0;
    virtual auto MoveThreadItem(
        const identifier::Nym& nymId,
        const UnallocatedCString& fromThreadID,
        const UnallocatedCString& toThreadID,
        const UnallocatedCString& itemID) const -> bool = 0;
    virtual auto NymBoxList(
        const identifier::Nym& nymID,
        const otx::client::StorageBox box) const -> ObjectList = 0;
    virtual auto NymList() const -> ObjectList = 0;
    virtual auto PaymentWorkflowList(const identifier::Nym& nymID) const
        -> ObjectList = 0;
    virtual auto PaymentWorkflowLookup(
        const identifier::Nym& nymID,
        const UnallocatedCString& sourceID) const -> UnallocatedCString = 0;
    virtual auto PaymentWorkflowsByAccount(
        const identifier::Nym& nymID,
        const UnallocatedCString& accountID) const
        -> UnallocatedSet<UnallocatedCString> = 0;
    virtual auto PaymentWorkflowsByState(
        const identifier::Nym& nymID,
        const otx::client::PaymentWorkflowType type,
        const otx::client::PaymentWorkflowState state) const
        -> UnallocatedSet<UnallocatedCString> = 0;
    virtual auto PaymentWorkflowsByUnit(
        const identifier::Nym& nymID,
        const UnallocatedCString& unitID) const
        -> UnallocatedSet<UnallocatedCString> = 0;
    virtual auto PaymentWorkflowState(
        const identifier::Nym& nymID,
        const UnallocatedCString& workflowID) const
        -> std::pair<
            otx::client::PaymentWorkflowType,
            otx::client::PaymentWorkflowState> = 0;
    virtual auto RelabelThread(
        const UnallocatedCString& threadID,
        const UnallocatedCString& label) const -> bool = 0;
    virtual auto RemoveBlockchainThreadItem(
        const identifier::Nym& nym,
        const identifier::Generic& thread,
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::TransactionHash& txid) const noexcept
        -> bool = 0;
    virtual auto RemoveNymBoxItem(
        const identifier::Nym& nymID,
        const otx::client::StorageBox box,
        const identifier::Generic& itemID) const -> bool = 0;
    virtual auto RemoveServer(const identifier::Notary& id) const -> bool = 0;
    virtual auto RemoveThreadItem(
        const identifier::Nym& nym,
        const identifier::Generic& thread,
        const UnallocatedCString& id) const -> bool = 0;
    virtual auto RemoveUnitDefinition(
        const identifier::UnitDefinition& id) const -> bool = 0;
    virtual auto RenameThread(
        const identifier::Nym& nymId,
        const UnallocatedCString& threadId,
        const UnallocatedCString& newID) const -> bool = 0;
    virtual void RunGC() const = 0;
    virtual auto ServerAlias(const identifier::Notary& id) const
        -> UnallocatedCString = 0;
    virtual auto ServerList() const -> ObjectList = 0;
    virtual auto SeedList() const -> ObjectList = 0;
    virtual auto SetAccountAlias(
        const UnallocatedCString& id,
        std::string_view alias) const -> bool = 0;
    virtual auto SetContactAlias(
        const UnallocatedCString& id,
        std::string_view alias) const -> bool = 0;
    virtual auto SetDefaultNym(const identifier::Nym& id) const -> bool = 0;
    virtual auto SetDefaultSeed(const opentxs::crypto::SeedID& id) const
        -> bool = 0;
    virtual auto SetNymAlias(const identifier::Nym& id, std::string_view alias)
        const -> bool = 0;
    virtual auto SetPeerRequestTime(
        const identifier::Nym& nymID,
        const identifier::Generic& id,
        const otx::client::StorageBox box) const -> bool = 0;
    virtual auto SetReadState(
        const identifier::Nym& nymId,
        const UnallocatedCString& threadId,
        const UnallocatedCString& itemId,
        const bool unread) const -> bool = 0;
    virtual auto SetSeedAlias(
        const opentxs::crypto::SeedID& id,
        std::string_view alias) const -> bool = 0;
    virtual auto SetServerAlias(
        const identifier::Notary& id,
        std::string_view alias) const -> bool = 0;
    virtual auto SetThreadAlias(
        const identifier::Nym& nymId,
        const UnallocatedCString& threadId,
        std::string_view alias) const -> bool = 0;
    virtual auto SetUnitDefinitionAlias(
        const identifier::UnitDefinition& id,
        std::string_view alias) const -> bool = 0;
    virtual auto Store(
        const UnallocatedCString& accountID,
        const UnallocatedCString& data,
        std::string_view alias,
        const identifier::Nym& ownerNym,
        const identifier::Nym& signerNym,
        const identifier::Nym& issuerNym,
        const identifier::Notary& server,
        const identifier::UnitDefinition& contract,
        const UnitType unit) const -> bool = 0;
    virtual auto Store(
        const identifier::Nym& nymID,
        const opentxs::identity::wot::claim::ClaimType type,
        const proto::HDAccount& data) const -> bool = 0;
    virtual auto Store(
        const identifier::Nym& nymID,
        const identifier::Account& channelID,
        const proto::Bip47Channel& data) const -> bool = 0;
    virtual auto Store(const proto::Contact& data) const -> bool = 0;
    virtual auto Store(const proto::Context& data) const -> bool = 0;
    virtual auto Store(const proto::Credential& data) const -> bool = 0;
    virtual auto Store(const proto::Nym& data, std::string_view alias = {})
        const -> bool = 0;
    virtual auto Store(const ReadView& data, std::string_view alias = {}) const
        -> bool = 0;
    virtual auto Store(const identifier::Nym& nymID, const proto::Issuer& data)
        const -> bool = 0;
    virtual auto Store(
        const identifier::Nym& nymID,
        const proto::PaymentWorkflow& data) const -> bool = 0;
    virtual auto Store(
        const identifier::Nym& nymid,
        const UnallocatedCString& threadid,
        const UnallocatedCString& itemid,
        const std::uint64_t time,
        std::string_view alias,
        const UnallocatedCString& data,
        const otx::client::StorageBox box,
        const UnallocatedCString& account = {}) const -> bool = 0;
    virtual auto Store(
        const identifier::Nym& nym,
        const identifier::Generic& thread,
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::TransactionHash& txid,
        const Time time) const noexcept -> bool = 0;
    virtual auto Store(
        const proto::PeerReply& data,
        const identifier::Nym& nymid,
        const otx::client::StorageBox box) const -> bool = 0;
    virtual auto Store(
        const proto::PeerRequest& data,
        const identifier::Nym& nymid,
        const otx::client::StorageBox box) const -> bool = 0;
    virtual auto Store(const identifier::Nym& nym, const proto::Purse& purse)
        const -> bool = 0;
    virtual auto Store(
        const opentxs::crypto::SeedID& id,
        const proto::Seed& data) const -> bool = 0;
    virtual auto Store(
        const proto::ServerContract& data,
        std::string_view alias = {}) const -> bool = 0;
    virtual auto Store(const proto::Ciphertext& serialized) const -> bool = 0;
    virtual auto Store(
        const proto::UnitDefinition& data,
        std::string_view alias = {}) const -> bool = 0;
    virtual auto ThreadList(const identifier::Nym& nymID, const bool unreadOnly)
        const -> ObjectList = 0;
    virtual auto ThreadAlias(
        const identifier::Nym& nymID,
        const UnallocatedCString& threadID) const -> UnallocatedCString = 0;
    virtual auto UnaffiliatedBlockchainTransaction(
        const identifier::Nym& recipient,
        const opentxs::blockchain::block::TransactionHash& txid) const noexcept
        -> bool = 0;
    virtual auto UnitDefinitionAlias(const identifier::UnitDefinition& id) const
        -> UnallocatedCString = 0;
    virtual auto UnitDefinitionList() const -> ObjectList = 0;
    virtual auto UnreadCount(
        const identifier::Nym& nymId,
        const UnallocatedCString& threadId) const -> std::size_t = 0;
    virtual auto Upgrade() noexcept -> void = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::Storage& = 0;

    Storage(const Storage&) = delete;
    Storage(Storage&&) = delete;
    auto operator=(const Storage&) -> Storage& = delete;
    auto operator=(Storage&&) -> Storage& = delete;

    OPENTXS_NO_EXPORT virtual ~Storage() = default;

protected:
    Storage() = default;
};
}  // namespace opentxs::api::session

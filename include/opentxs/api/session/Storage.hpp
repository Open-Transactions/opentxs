// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>
#include <ctime>
#include <functional>
#include <memory>

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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

namespace identifier
{
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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session
{
class Storage
{
public:
    using Bip47ChannelList = UnallocatedSet<OTIdentifier>;

    virtual auto AccountAlias(const Identifier& accountID) const
        -> UnallocatedCString = 0;
    virtual auto AccountList() const -> ObjectList = 0;
    virtual auto AccountContract(const Identifier& accountID) const
        -> OTUnitID = 0;
    virtual auto AccountIssuer(const Identifier& accountID) const
        -> OTNymID = 0;
    virtual auto AccountOwner(const Identifier& accountID) const -> OTNymID = 0;
    virtual auto AccountServer(const Identifier& accountID) const
        -> OTNotaryID = 0;
    virtual auto AccountSigner(const Identifier& accountID) const
        -> OTNymID = 0;
    virtual auto AccountUnit(const Identifier& accountID) const -> UnitType = 0;
    virtual auto AccountsByContract(const identifier::UnitDefinition& contract)
        const -> UnallocatedSet<OTIdentifier> = 0;
    virtual auto AccountsByIssuer(const identifier::Nym& issuerNym) const
        -> UnallocatedSet<OTIdentifier> = 0;
    virtual auto AccountsByOwner(const identifier::Nym& ownerNym) const
        -> UnallocatedSet<OTIdentifier> = 0;
    virtual auto AccountsByServer(const identifier::Notary& server) const
        -> UnallocatedSet<OTIdentifier> = 0;
    virtual auto AccountsByUnit(const UnitType unit) const
        -> UnallocatedSet<OTIdentifier> = 0;
    virtual auto Bip47Chain(
        const identifier::Nym& nymID,
        const Identifier& channelID) const -> UnitType = 0;
    virtual auto Bip47ChannelsByChain(
        const identifier::Nym& nymID,
        const UnitType chain) const -> Bip47ChannelList = 0;
    virtual auto BlockchainAccountList(
        const UnallocatedCString& nymID,
        const UnitType type) const -> UnallocatedSet<UnallocatedCString> = 0;
    virtual auto BlockchainSubaccountAccountType(
        const identifier::Nym& owner,
        const Identifier& id) const -> UnitType = 0;
    virtual auto BlockchainThreadMap(
        const identifier::Nym& nym,
        const Data& txid) const noexcept -> UnallocatedVector<OTIdentifier> = 0;
    virtual auto BlockchainTransactionList(const identifier::Nym& nym)
        const noexcept -> UnallocatedVector<OTData> = 0;
    virtual auto CheckTokenSpent(
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit,
        const std::uint64_t series,
        const UnallocatedCString& key) const -> bool = 0;
    virtual auto ContactAlias(const UnallocatedCString& id) const
        -> UnallocatedCString = 0;
    virtual auto ContactList() const -> ObjectList = 0;
    virtual auto ContextList(const UnallocatedCString& nymID) const
        -> ObjectList = 0;
    virtual auto ContactOwnerNym(const UnallocatedCString& nymID) const
        -> UnallocatedCString = 0;
    virtual void ContactSaveIndices() const = 0;
    virtual auto ContactUpgradeLevel() const -> VersionNumber = 0;
    virtual auto CreateThread(
        const UnallocatedCString& nymID,
        const UnallocatedCString& threadID,
        const UnallocatedSet<UnallocatedCString>& participants) const
        -> bool = 0;
    virtual auto DeleteAccount(const UnallocatedCString& id) const -> bool = 0;
    virtual auto DefaultNym() const -> OTNymID = 0;
    virtual auto DefaultSeed() const -> UnallocatedCString = 0;
    virtual auto DeleteContact(const UnallocatedCString& id) const -> bool = 0;
    virtual auto DeletePaymentWorkflow(
        const UnallocatedCString& nymID,
        const UnallocatedCString& workflowID) const -> bool = 0;
    virtual auto HashType() const -> std::uint32_t = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Storage& = 0;
    virtual auto IssuerList(const UnallocatedCString& nymID) const
        -> ObjectList = 0;
    virtual auto Load(
        const UnallocatedCString& accountID,
        UnallocatedCString& output,
        UnallocatedCString& alias,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const UnallocatedCString& nymID,
        const UnallocatedCString& accountID,
        proto::HDAccount& output,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nymID,
        const Identifier& channelID,
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
        const UnallocatedCString& nym,
        const UnallocatedCString& id,
        proto::Context& context,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const UnallocatedCString& id,
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
        AllocateOutput destination,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const UnallocatedCString& nymID,
        const UnallocatedCString& id,
        proto::Issuer& issuer,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const UnallocatedCString& nymID,
        const UnallocatedCString& workflowID,
        proto::PaymentWorkflow& workflow,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const UnallocatedCString& nymID,
        const UnallocatedCString& id,
        const StorageBox box,
        UnallocatedCString& output,
        UnallocatedCString& alias,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const UnallocatedCString& nymID,
        const UnallocatedCString& id,
        const StorageBox box,
        proto::PeerReply& request,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const UnallocatedCString& nymID,
        const UnallocatedCString& id,
        const StorageBox box,
        proto::PeerRequest& request,
        std::time_t& time,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nym,
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit,
        proto::Purse& output,
        const bool checking) const -> bool = 0;
    virtual auto Load(
        const UnallocatedCString& id,
        proto::Seed& seed,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const UnallocatedCString& id,
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
        const UnallocatedCString& nymId,
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
    virtual auto LocalNyms() const
        -> const UnallocatedSet<UnallocatedCString> = 0;
    virtual void MapPublicNyms(NymLambda& lambda) const = 0;
    virtual void MapServers(ServerLambda& lambda) const = 0;
    virtual void MapUnitDefinitions(UnitLambda& lambda) const = 0;
    virtual auto MarkTokenSpent(
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit,
        const std::uint64_t series,
        const UnallocatedCString& key) const -> bool = 0;
    virtual auto MoveThreadItem(
        const UnallocatedCString& nymId,
        const UnallocatedCString& fromThreadID,
        const UnallocatedCString& toThreadID,
        const UnallocatedCString& itemID) const -> bool = 0;
    virtual auto NymBoxList(
        const UnallocatedCString& nymID,
        const StorageBox box) const -> ObjectList = 0;
    virtual auto NymList() const -> ObjectList = 0;
    virtual auto PaymentWorkflowList(const UnallocatedCString& nymID) const
        -> ObjectList = 0;
    virtual auto PaymentWorkflowLookup(
        const UnallocatedCString& nymID,
        const UnallocatedCString& sourceID) const -> UnallocatedCString = 0;
    virtual auto PaymentWorkflowsByAccount(
        const UnallocatedCString& nymID,
        const UnallocatedCString& accountID) const
        -> UnallocatedSet<UnallocatedCString> = 0;
    virtual auto PaymentWorkflowsByState(
        const UnallocatedCString& nymID,
        const otx::client::PaymentWorkflowType type,
        const otx::client::PaymentWorkflowState state) const
        -> UnallocatedSet<UnallocatedCString> = 0;
    virtual auto PaymentWorkflowsByUnit(
        const UnallocatedCString& nymID,
        const UnallocatedCString& unitID) const
        -> UnallocatedSet<UnallocatedCString> = 0;
    virtual auto PaymentWorkflowState(
        const UnallocatedCString& nymID,
        const UnallocatedCString& workflowID) const
        -> std::pair<
            otx::client::PaymentWorkflowType,
            otx::client::PaymentWorkflowState> = 0;
    virtual auto RelabelThread(
        const UnallocatedCString& threadID,
        const UnallocatedCString& label) const -> bool = 0;
    virtual auto RemoveBlockchainThreadItem(
        const identifier::Nym& nym,
        const Identifier& thread,
        const opentxs::blockchain::Type chain,
        const Data& txid) const noexcept -> bool = 0;
    virtual auto RemoveNymBoxItem(
        const UnallocatedCString& nymID,
        const StorageBox box,
        const UnallocatedCString& itemID) const -> bool = 0;
    virtual auto RemoveServer(const UnallocatedCString& id) const -> bool = 0;
    virtual auto RemoveThreadItem(
        const identifier::Nym& nym,
        const Identifier& thread,
        const UnallocatedCString& id) const -> bool = 0;
    virtual auto RemoveUnitDefinition(const UnallocatedCString& id) const
        -> bool = 0;
    virtual auto RenameThread(
        const UnallocatedCString& nymId,
        const UnallocatedCString& threadId,
        const UnallocatedCString& newID) const -> bool = 0;
    virtual void RunGC() const = 0;
    virtual auto ServerAlias(const UnallocatedCString& id) const
        -> UnallocatedCString = 0;
    virtual auto ServerList() const -> ObjectList = 0;
    virtual auto SeedList() const -> ObjectList = 0;
    virtual auto SetAccountAlias(
        const UnallocatedCString& id,
        const UnallocatedCString& alias) const -> bool = 0;
    virtual auto SetContactAlias(
        const UnallocatedCString& id,
        const UnallocatedCString& alias) const -> bool = 0;
    virtual auto SetDefaultNym(const identifier::Nym& id) const -> bool = 0;
    virtual auto SetDefaultSeed(const UnallocatedCString& id) const -> bool = 0;
    virtual auto SetNymAlias(
        const identifier::Nym& id,
        const UnallocatedCString& alias) const -> bool = 0;
    virtual auto SetPeerRequestTime(
        const UnallocatedCString& nymID,
        const UnallocatedCString& id,
        const StorageBox box) const -> bool = 0;
    virtual auto SetReadState(
        const UnallocatedCString& nymId,
        const UnallocatedCString& threadId,
        const UnallocatedCString& itemId,
        const bool unread) const -> bool = 0;
    virtual auto SetSeedAlias(
        const UnallocatedCString& id,
        const UnallocatedCString& alias) const -> bool = 0;
    virtual auto SetServerAlias(
        const identifier::Notary& id,
        const UnallocatedCString& alias) const -> bool = 0;
    virtual auto SetThreadAlias(
        const UnallocatedCString& nymId,
        const UnallocatedCString& threadId,
        const UnallocatedCString& alias) const -> bool = 0;
    virtual auto SetUnitDefinitionAlias(
        const identifier::UnitDefinition& id,
        const UnallocatedCString& alias) const -> bool = 0;
    virtual auto Store(
        const UnallocatedCString& accountID,
        const UnallocatedCString& data,
        const UnallocatedCString& alias,
        const identifier::Nym& ownerNym,
        const identifier::Nym& signerNym,
        const identifier::Nym& issuerNym,
        const identifier::Notary& server,
        const identifier::UnitDefinition& contract,
        const UnitType unit) const -> bool = 0;
    virtual auto Store(
        const UnallocatedCString& nymID,
        const opentxs::identity::wot::claim::ClaimType type,
        const proto::HDAccount& data) const -> bool = 0;
    virtual auto Store(
        const identifier::Nym& nymID,
        const Identifier& channelID,
        const proto::Bip47Channel& data) const -> bool = 0;
    virtual auto Store(const proto::Contact& data) const -> bool = 0;
    virtual auto Store(const proto::Context& data) const -> bool = 0;
    virtual auto Store(const proto::Credential& data) const -> bool = 0;
    virtual auto Store(
        const proto::Nym& data,
        const UnallocatedCString& alias = {}) const -> bool = 0;
    virtual auto Store(
        const ReadView& data,
        const UnallocatedCString& alias = {}) const -> bool = 0;
    virtual auto Store(
        const UnallocatedCString& nymID,
        const proto::Issuer& data) const -> bool = 0;
    virtual auto Store(
        const UnallocatedCString& nymID,
        const proto::PaymentWorkflow& data) const -> bool = 0;
    virtual auto Store(
        const UnallocatedCString& nymid,
        const UnallocatedCString& threadid,
        const UnallocatedCString& itemid,
        const std::uint64_t time,
        const UnallocatedCString& alias,
        const UnallocatedCString& data,
        const StorageBox box,
        const UnallocatedCString& account = {}) const -> bool = 0;
    virtual auto Store(
        const identifier::Nym& nym,
        const Identifier& thread,
        const opentxs::blockchain::Type chain,
        const Data& txid,
        const Time time) const noexcept -> bool = 0;
    virtual auto Store(
        const proto::PeerReply& data,
        const UnallocatedCString& nymid,
        const StorageBox box) const -> bool = 0;
    virtual auto Store(
        const proto::PeerRequest& data,
        const UnallocatedCString& nymid,
        const StorageBox box) const -> bool = 0;
    virtual auto Store(const identifier::Nym& nym, const proto::Purse& purse)
        const -> bool = 0;
    virtual auto Store(const proto::Seed& data) const -> bool = 0;
    virtual auto Store(
        const proto::ServerContract& data,
        const UnallocatedCString& alias = {}) const -> bool = 0;
    virtual auto Store(const proto::Ciphertext& serialized) const -> bool = 0;
    virtual auto Store(
        const proto::UnitDefinition& data,
        const UnallocatedCString& alias = {}) const -> bool = 0;
    virtual auto ThreadList(
        const UnallocatedCString& nymID,
        const bool unreadOnly) const -> ObjectList = 0;
    virtual auto ThreadAlias(
        const UnallocatedCString& nymID,
        const UnallocatedCString& threadID) const -> UnallocatedCString = 0;
    virtual auto UnaffiliatedBlockchainTransaction(
        const identifier::Nym& recipient,
        const Data& txid) const noexcept -> bool = 0;
    virtual auto UnitDefinitionAlias(const UnallocatedCString& id) const
        -> UnallocatedCString = 0;
    virtual auto UnitDefinitionList() const -> ObjectList = 0;
    virtual auto UnreadCount(
        const UnallocatedCString& nymId,
        const UnallocatedCString& threadId) const -> std::size_t = 0;
    virtual void UpgradeNyms() = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::Storage& = 0;

    OPENTXS_NO_EXPORT virtual ~Storage() = default;

protected:
    Storage() = default;

private:
    Storage(const Storage&) = delete;
    Storage(Storage&&) = delete;
    auto operator=(const Storage&) -> Storage& = delete;
    auto operator=(Storage&&) -> Storage& = delete;
};
}  // namespace opentxs::api::session

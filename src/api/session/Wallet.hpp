// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/contact/ClaimType.hpp"
// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <chrono>
#include <cstdint>
#include <ctime>
#include <functional>
#include <iosfwd>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "Proto.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/identity/Identity.hpp"
#include "internal/otx/consensus/Consensus.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Editor.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/contract/basket/BasketContract.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Request.hpp"
#include "opentxs/network/zeromq/socket/Sender.hpp"
#include "opentxs/otx/blind/CashType.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/NymEditor.hpp"
#include "serialization/protobuf/ContactEnums.pb.h"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
class Parameters;
}  // namespace crypto

namespace display
{
class Definition;
}  // namespace display

namespace identity
{
namespace internal
{
struct Nym;
}  // namespace internal

class Nym;
}  // namespace identity

namespace otx
{
namespace blind
{
class Purse;
}  // namespace blind

namespace client
{
class Issuer;
}  // namespace client

namespace context
{
namespace internal
{
struct Base;
}  // namespace internal

class Base;
class Client;
class Server;
}  // namespace context
}  // namespace otx

namespace proto
{
class Context;
class Credential;
class Issuer;
class Nym;
class PeerReply;
class PeerRequest;
class ServerContract;
class UnitDefinition;
}  // namespace proto

class Context;
class NymFile;
class PasswordPrompt;
class PeerObject;
class String;
}  // namespace opentxs

namespace opentxs::api::session::imp
{
class Wallet : virtual public internal::Wallet, public Lockable
{
public:
    auto Account(const Identifier& accountID) const -> SharedAccount final;
    auto AccountPartialMatch(const std::string& hint) const
        -> OTIdentifier final;
    auto CreateAccount(
        const identifier::Nym& ownerNymID,
        const identifier::Server& notaryID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const identity::Nym& signer,
        Account::AccountType type,
        TransactionNumber stash,
        const PasswordPrompt& reason) const -> ExclusiveAccount final;
    auto DeleteAccount(const Identifier& accountID) const -> bool final;
    auto IssuerAccount(const identifier::UnitDefinition& unitID) const
        -> SharedAccount final;
    auto mutable_Account(
        const Identifier& accountID,
        const PasswordPrompt& reason,
        const AccountCallback callback) const -> ExclusiveAccount final;
    auto UpdateAccount(
        const Identifier& accountID,
        const otx::context::Server&,
        const String& serialized,
        const PasswordPrompt& reason) const -> bool final;
    auto UpdateAccount(
        const Identifier& accountID,
        const otx::context::Server&,
        const String& serialized,
        const std::string& label,
        const PasswordPrompt& reason) const -> bool final;
    auto ImportAccount(std::unique_ptr<opentxs::Account>& imported) const
        -> bool final;
    auto ClientContext(const identifier::Nym& remoteNymID) const
        -> std::shared_ptr<const otx::context::Client> override;
    auto ServerContext(
        const identifier::Nym& localNymID,
        const Identifier& remoteID) const
        -> std::shared_ptr<const otx::context::Server> override;
    auto mutable_ClientContext(
        const identifier::Nym& remoteNymID,
        const PasswordPrompt& reason) const
        -> Editor<otx::context::Client> override;
    auto mutable_ServerContext(
        const identifier::Nym& localNymID,
        const Identifier& remoteID,
        const PasswordPrompt& reason) const
        -> Editor<otx::context::Server> override;
    auto IssuerList(const identifier::Nym& nymID) const
        -> std::pmr::set<OTNymID> final;
    auto Issuer(const identifier::Nym& nymID, const identifier::Nym& issuerID)
        const -> std::shared_ptr<const otx::client::Issuer> final;
    auto mutable_Issuer(
        const identifier::Nym& nymID,
        const identifier::Nym& issuerID) const
        -> Editor<otx::client::Issuer> final;
    auto IsLocalNym(const std::string& id) const -> bool final;
    auto LocalNymCount() const -> std::size_t final;
    auto LocalNyms() const -> std::pmr::set<OTNymID> final;
    auto Nym(
        const identifier::Nym& id,
        const std::chrono::milliseconds& timeout =
            std::chrono::milliseconds(0)) const -> Nym_p final;
    auto Nym(const proto::Nym& nym) const -> Nym_p final;
    auto Nym(const ReadView& bytes) const -> Nym_p final;
    auto Nym(
        const contact::ClaimType type,
        const PasswordPrompt& reason,
        const std::string& name) const -> Nym_p final;
    auto Nym(
        const opentxs::crypto::Parameters& parameters,
        const PasswordPrompt& reason,
        const std::string& name) const -> Nym_p final;
    auto Nym(const PasswordPrompt& reason, const std::string& name) const
        -> Nym_p final;
    auto Nym(
        const opentxs::crypto::Parameters& parameters,
        const contact::ClaimType type,
        const PasswordPrompt& reason,
        const std::string& name) const -> Nym_p final;
    auto mutable_Nym(const identifier::Nym& id, const PasswordPrompt& reason)
        const -> NymData final;
    auto Nymfile(const identifier::Nym& id, const PasswordPrompt& reason) const
        -> std::unique_ptr<const opentxs::NymFile> final;
    auto mutable_Nymfile(
        const identifier::Nym& id,
        const PasswordPrompt& reason) const -> Editor<opentxs::NymFile> final;
    auto NymByIDPartialMatch(const std::string& partialId) const -> Nym_p final;
    auto NymList() const -> ObjectList final;
    auto NymNameByIndex(const std::size_t index, String& name) const
        -> bool final;
    auto PeerReply(
        const identifier::Nym& nym,
        const Identifier& reply,
        const StorageBox& box,
        proto::PeerReply& serialized) const -> bool final;
    auto PeerReply(
        const identifier::Nym& nym,
        const Identifier& reply,
        const StorageBox& box,
        AllocateOutput destination) const -> bool final;
    auto PeerReplyComplete(
        const identifier::Nym& nym,
        const Identifier& replyOrRequest) const -> bool final;
    auto PeerReplyCreate(
        const identifier::Nym& nym,
        const proto::PeerRequest& request,
        const proto::PeerReply& reply) const -> bool final;
    auto PeerReplyCreateRollback(
        const identifier::Nym& nym,
        const Identifier& request,
        const Identifier& reply) const -> bool final;
    auto PeerReplySent(const identifier::Nym& nym) const -> ObjectList final;
    auto PeerReplyIncoming(const identifier::Nym& nym) const
        -> ObjectList final;
    auto PeerReplyFinished(const identifier::Nym& nym) const
        -> ObjectList final;
    auto PeerReplyProcessed(const identifier::Nym& nym) const
        -> ObjectList final;
    auto PeerReplyReceive(const identifier::Nym& nym, const PeerObject& reply)
        const -> bool final;
    auto PeerRequest(
        const identifier::Nym& nym,
        const Identifier& request,
        const StorageBox& box,
        std::time_t& time,
        proto::PeerRequest& serialized) const -> bool final;
    auto PeerRequest(
        const identifier::Nym& nym,
        const Identifier& request,
        const StorageBox& box,
        std::time_t& time,
        AllocateOutput destination) const -> bool final;
    auto PeerRequestComplete(
        const identifier::Nym& nym,
        const Identifier& reply) const -> bool final;
    auto PeerRequestCreate(
        const identifier::Nym& nym,
        const proto::PeerRequest& request) const -> bool final;
    auto PeerRequestCreateRollback(
        const identifier::Nym& nym,
        const Identifier& request) const -> bool final;
    auto PeerRequestDelete(
        const identifier::Nym& nym,
        const Identifier& request,
        const StorageBox& box) const -> bool final;
    auto PeerRequestSent(const identifier::Nym& nym) const -> ObjectList final;
    auto PeerRequestIncoming(const identifier::Nym& nym) const
        -> ObjectList final;
    auto PeerRequestFinished(const identifier::Nym& nym) const
        -> ObjectList final;
    auto PeerRequestProcessed(const identifier::Nym& nym) const
        -> ObjectList final;
    auto PeerRequestReceive(
        const identifier::Nym& nym,
        const PeerObject& request) const -> bool final;
    auto PeerRequestUpdate(
        const identifier::Nym& nym,
        const Identifier& request,
        const StorageBox& box) const -> bool final;
    auto Purse(
        const identifier::Nym& nym,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const bool checking) const -> const otx::blind::Purse& final;
    auto mutable_Purse(
        const identifier::Nym& nym,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const PasswordPrompt& reason,
        const otx::blind::CashType type) const
        -> Editor<otx::blind::Purse, std::shared_mutex> final;
    auto RemoveServer(const identifier::Server& id) const -> bool final;
    auto RemoveUnitDefinition(const identifier::UnitDefinition& id) const
        -> bool final;
    auto Server(
        const identifier::Server& id,
        const std::chrono::milliseconds& timeout =
            std::chrono::milliseconds(0)) const -> OTServerContract final;
    auto Server(const proto::ServerContract& contract) const
        -> OTServerContract final;
    auto Server(const ReadView& contract) const -> OTServerContract final;
    auto Server(
        const std::string& nymid,
        const std::string& name,
        const std::string& terms,
        const std::pmr::list<contract::Server::Endpoint>& endpoints,
        const PasswordPrompt& reason,
        const VersionNumber version) const -> OTServerContract final;
    auto ServerList() const -> ObjectList final;
    auto SetNymAlias(const identifier::Nym& id, const std::string& alias) const
        -> bool final;
    auto SetServerAlias(const identifier::Server& id, const std::string& alias)
        const -> bool final;
    auto SetUnitDefinitionAlias(
        const identifier::UnitDefinition& id,
        const std::string& alias) const -> bool final;
    auto UnitDefinitionList() const -> ObjectList final;
    auto UnitDefinition(
        const identifier::UnitDefinition& id,
        const std::chrono::milliseconds& timeout =
            std::chrono::milliseconds(0)) const -> OTUnitDefinition final;
    auto BasketContract(
        const identifier::UnitDefinition& id,
        const std::chrono::milliseconds& timeout = std::chrono::milliseconds(
            0)) const noexcept(false) -> OTBasketContract final;
    auto UnitDefinition(const proto::UnitDefinition& contract) const
        -> OTUnitDefinition final;
    auto UnitDefinition(const ReadView contract) const
        -> OTUnitDefinition final;
    auto CurrencyContract(
        const std::string& nymid,
        const std::string& shortname,
        const std::string& terms,
        const core::UnitType unitOfAccount,
        const Amount& redemptionIncrement,
        const PasswordPrompt& reason) const -> OTUnitDefinition final;
    auto CurrencyContract(
        const std::string& nymid,
        const std::string& shortname,
        const std::string& terms,
        const core::UnitType unitOfAccount,
        const Amount& redemptionIncrement,
        const display::Definition& displayDefinition,
        const PasswordPrompt& reason) const -> OTUnitDefinition final;
    auto CurrencyContract(
        const std::string& nymid,
        const std::string& shortname,
        const std::string& terms,
        const core::UnitType unitOfAccount,
        const Amount& redemptionIncrement,
        const VersionNumber version,
        const PasswordPrompt& reason) const -> OTUnitDefinition final;
    auto CurrencyContract(
        const std::string& nymid,
        const std::string& shortname,
        const std::string& terms,
        const core::UnitType unitOfAccount,
        const Amount& redemptionIncrement,
        const display::Definition& displayDefinition,
        const VersionNumber version,
        const PasswordPrompt& reason) const -> OTUnitDefinition final;
    auto SecurityContract(
        const std::string& nymid,
        const std::string& shortname,
        const std::string& terms,
        const core::UnitType unitOfAccount,
        const PasswordPrompt& reason,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement,
        const VersionNumber version = contract::Unit::DefaultVersion) const
        -> OTUnitDefinition final;
    auto CurrencyTypeBasedOnUnitType(
        const identifier::UnitDefinition& contractID) const
        -> core::UnitType final;

    auto LoadCredential(
        const std::string& id,
        std::shared_ptr<proto::Credential>& credential) const -> bool final;
    auto SaveCredential(const proto::Credential& credential) const
        -> bool final;

    ~Wallet() override = default;

protected:
    using AccountLock =
        std::pair<std::shared_mutex, std::unique_ptr<opentxs::Account>>;
    using ContextID = std::pair<std::string, std::string>;
    using ContextMap =
        std::pmr::map<ContextID, std::shared_ptr<otx::context::internal::Base>>;

    const api::Session& api_;
    mutable ContextMap context_map_;
    mutable std::mutex context_map_lock_;

    auto context(
        const identifier::Nym& localNymID,
        const identifier::Nym& remoteNymID) const
        -> std::shared_ptr<otx::context::Base>;
    auto extract_unit(const identifier::UnitDefinition& contractID) const
        -> core::UnitType;
    auto extract_unit(const contract::Unit& contract) const -> core::UnitType;
    void save(
        const PasswordPrompt& reason,
        otx::context::internal::Base* context) const;
    auto server_to_nym(Identifier& nymOrNotaryID) const -> OTNymID;

    Wallet(const api::Session& api);

private:
    using AccountMap = std::pmr::map<OTIdentifier, AccountLock>;
    using NymLock =
        std::pair<std::mutex, std::shared_ptr<identity::internal::Nym>>;
    using NymMap = std::pmr::map<OTNymID, NymLock>;
    using ServerMap =
        std::pmr::map<OTServerID, std::shared_ptr<contract::Server>>;
    using UnitMap = std::pmr::map<OTUnitID, std::shared_ptr<contract::Unit>>;
    using IssuerID = std::pair<OTIdentifier, OTIdentifier>;
    using IssuerLock =
        std::pair<std::mutex, std::shared_ptr<otx::client::Issuer>>;
    using IssuerMap = std::pmr::map<IssuerID, IssuerLock>;
    using PurseID = std::tuple<OTNymID, OTServerID, OTUnitID>;
    using PurseMap =
        std::pmr::map<PurseID, std::pair<std::shared_mutex, otx::blind::Purse>>;
    using UnitNameMap = std::pmr::map<std::string, proto::ContactItemType>;
    using UnitNameReverse = std::pmr::map<proto::ContactItemType, std::string>;

    mutable AccountMap account_map_;
    mutable NymMap nym_map_;
    mutable ServerMap server_map_;
    mutable UnitMap unit_map_;
    mutable IssuerMap issuer_map_;
    mutable std::mutex account_map_lock_;
    mutable std::mutex nym_map_lock_;
    mutable std::mutex server_map_lock_;
    mutable std::mutex unit_map_lock_;
    mutable std::mutex issuer_map_lock_;
    mutable std::mutex peer_map_lock_;
    mutable std::pmr::map<std::string, std::mutex> peer_lock_;
    mutable std::mutex nymfile_map_lock_;
    mutable std::pmr::map<OTIdentifier, std::mutex> nymfile_lock_;
    mutable std::mutex purse_lock_;
    mutable PurseMap purse_map_;
    OTZMQPublishSocket account_publisher_;
    OTZMQPublishSocket issuer_publisher_;
    OTZMQPublishSocket nym_publisher_;
    OTZMQPublishSocket nym_created_publisher_;
    OTZMQPublishSocket server_publisher_;
    OTZMQPublishSocket unit_publisher_;
    OTZMQPublishSocket peer_reply_publisher_;
    OTZMQPublishSocket peer_request_publisher_;
    OTZMQRequestSocket dht_nym_requester_;
    OTZMQRequestSocket dht_server_requester_;
    OTZMQRequestSocket dht_unit_requester_;
    OTZMQPushSocket find_nym_;

    static auto reverse_unit_map(const UnitNameMap& map) -> UnitNameReverse;

    auto account_alias(const std::string& accountID, const std::string& hint)
        const -> std::string;
    auto account_factory(
        const Identifier& accountID,
        const std::string& alias,
        const std::string& serialized) const -> opentxs::Account*;
    virtual void instantiate_client_context(
        const proto::Context& serialized,
        const Nym_p& localNym,
        const Nym_p& remoteNym,
        std::shared_ptr<otx::context::internal::Base>& output) const
    {
    }
    virtual void instantiate_server_context(
        const proto::Context& serialized,
        const Nym_p& localNym,
        const Nym_p& remoteNym,
        std::shared_ptr<otx::context::internal::Base>& output) const
    {
    }
    virtual auto load_legacy_account(
        const Identifier& accountID,
        const eLock& lock,
        AccountLock& row) const -> bool
    {
        return false;
    }
    auto mutable_nymfile(
        const Nym_p& targetNym,
        const Nym_p& signerNym,
        const identifier::Nym& id,
        const PasswordPrompt& reason) const -> Editor<opentxs::NymFile>;
    auto notify(const identifier::Nym& id) const noexcept -> void;
    virtual void nym_to_contact(
        [[maybe_unused]] const identity::Nym& nym,
        [[maybe_unused]] const std::string& name) const noexcept
    {
    }
    auto nymfile_lock(const identifier::Nym& nymID) const -> std::mutex&;
    auto peer_lock(const std::string& nymID) const -> std::mutex&;
    auto publish_server(const identifier::Server& id) const noexcept -> void;
    auto publish_unit(const identifier::UnitDefinition& id) const noexcept
        -> void;
    auto purse(
        const identifier::Nym& nym,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const bool checking) const -> PurseMap::mapped_type&;
    void save(
        const PasswordPrompt& reason,
        const std::string id,
        std::unique_ptr<opentxs::Account>& in,
        eLock& lock,
        bool success) const;
    void save(const Lock& lock, otx::client::Issuer* in) const;
    void save(const eLock& lock, const OTNymID nym, otx::blind::Purse* in)
        const;
    void save(NymData* nymData, const Lock& lock) const;
    void save(
        const PasswordPrompt& reason,
        opentxs::NymFile* nym,
        const Lock& lock) const;
    auto SaveCredentialIDs(const identity::Nym& nym) const -> bool;
    virtual auto signer_nym(const identifier::Nym& id) const -> Nym_p = 0;

    /* Throws std::out_of_range for missing accounts */
    auto account(
        const Lock& lock,
        const Identifier& accountID,
        const bool create) const -> AccountLock&;
    auto issuer(
        const identifier::Nym& nymID,
        const identifier::Nym& issuerID,
        const bool create) const -> IssuerLock&;

    auto server(std::unique_ptr<contract::Server> contract) const
        noexcept(false) -> OTServerContract;
    auto unit_definition(std::shared_ptr<contract::Unit>&& contract) const
        -> OTUnitDefinition;

    Wallet() = delete;
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;
};
}  // namespace opentxs::api::session::imp

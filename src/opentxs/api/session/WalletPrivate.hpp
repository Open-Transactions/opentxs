// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_deferred_guarded.h>
#include <cs_plain_guarded.h>
#include <chrono>
#include <ctime>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string_view>
#include <tuple>
#include <utility>

#include "internal/core/contract/BasketContract.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/network/zeromq/Handle.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/Push.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Lockable.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/contract/peer/Reply.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/storage/Types.internal.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/NymEditor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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
class Nym;
}  // namespace internal

class Nym;
}  // namespace identity

namespace network
{
namespace zeromq
{
namespace internal
{
class Batch;
class Thread;
}  // namespace internal

class ListenCallback;
class Message;
}  // namespace zeromq
}  // namespace network

namespace otx
{
namespace client
{
class Issuer;
}  // namespace client

namespace context
{
namespace internal
{
class Base;
}  // namespace internal

class Base;
class Client;
class Server;
}  // namespace context
}  // namespace otx

namespace protobuf
{
class Context;
class Credential;
class Nym;
class PeerReply;
class PeerRequest;
class ServerContract;
class UnitDefinition;
enum ContactItemType : int;
}  // namespace protobuf

class NymFile;
class PasswordPrompt;
class PeerObject;
class String;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session
{
using opentxs::storage::ErrorReporting;

class WalletPrivate : virtual public internal::Wallet, public Lockable
{
public:
    auto Account(const identifier::Account& accountID) const
        -> SharedAccount final;
    auto AccountPartialMatch(const UnallocatedCString& hint) const
        -> identifier::Generic final;
    auto BasketContract(
        const identifier::UnitDefinition& id,
        const std::chrono::milliseconds& timeout = std::chrono::milliseconds(
            0)) const noexcept(false) -> OTBasketContract final;
    auto ClientContext(const identifier::Nym& remoteNymID) const
        -> std::shared_ptr<const otx::context::Client> override;
    auto CreateAccount(
        const identifier::Nym& ownerNymID,
        const identifier::Notary& notaryID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const identity::Nym& signer,
        Account::AccountType type,
        TransactionNumber stash,
        const PasswordPrompt& reason) const -> ExclusiveAccount final;
    auto CurrencyContract(
        const UnallocatedCString& nymid,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const Amount& redemptionIncrement,
        const PasswordPrompt& reason) const -> OTUnitDefinition final;
    auto CurrencyContract(
        const UnallocatedCString& nymid,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const Amount& redemptionIncrement,
        const VersionNumber version,
        const PasswordPrompt& reason) const -> OTUnitDefinition final;
    auto CurrencyContract(
        const UnallocatedCString& nymid,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const Amount& redemptionIncrement,
        const display::Definition& displayDefinition,
        const PasswordPrompt& reason) const -> OTUnitDefinition final;
    auto CurrencyContract(
        const UnallocatedCString& nymid,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const Amount& redemptionIncrement,
        const display::Definition& displayDefinition,
        const VersionNumber version,
        const PasswordPrompt& reason) const -> OTUnitDefinition final;
    auto CurrencyTypeBasedOnUnitType(
        const identifier::UnitDefinition& contractID) const -> UnitType final;
    auto DefaultNym() const noexcept
        -> std::pair<identifier::Nym, std::size_t> final;
    auto DeleteAccount(const identifier::Account& accountID) const
        -> bool final;
    auto ImportAccount(std::unique_ptr<opentxs::Account>& imported) const
        -> bool final;
    auto IsLocalNym(const identifier::Nym& id) const -> bool final;
    auto IsLocalNym(const std::string_view id) const -> bool final;
    auto Issuer(const identifier::Nym& nymID, const identifier::Nym& issuerID)
        const -> std::shared_ptr<const otx::client::Issuer> final;
    auto IssuerAccount(const identifier::UnitDefinition& unitID) const
        -> SharedAccount final;
    auto IssuerList(const identifier::Nym& nymID) const
        -> UnallocatedSet<identifier::Nym> final;
    auto LoadCredential(
        const identifier::Generic& id,
        std::shared_ptr<protobuf::Credential>& credential) const -> bool final;
    auto LocalNymCount() const -> std::size_t final;
    auto LocalNyms() const -> Set<identifier::Nym> final;
    auto Nym(const PasswordPrompt& reason, const UnallocatedCString& name) const
        -> Nym_p final;
    auto Nym(const ReadView& bytes) const -> Nym_p final;
    auto Nym(
        const identifier::Nym& id,
        const std::chrono::milliseconds& timeout = 0ms) const -> Nym_p final;
    auto Nym(
        const identity::Type type,
        const PasswordPrompt& reason,
        const UnallocatedCString& name) const -> Nym_p final;
    auto Nym(
        const opentxs::crypto::Parameters& parameters,
        const PasswordPrompt& reason,
        const UnallocatedCString& name) const -> Nym_p final;
    auto Nym(
        const opentxs::crypto::Parameters& parameters,
        const identity::Type type,
        const PasswordPrompt& reason,
        const UnallocatedCString& name) const -> Nym_p final;
    auto Nym(const protobuf::Nym& nym) const -> Nym_p final;
    auto NymByIDPartialMatch(const UnallocatedCString& partialId) const
        -> Nym_p final;
    auto NymList() const -> ObjectList final;
    auto NymNameByIndex(const std::size_t index, String& name) const
        -> bool final;
    auto Nymfile(const identifier::Nym& id, const PasswordPrompt& reason) const
        -> std::unique_ptr<const opentxs::NymFile> final;
    auto PeerReply(
        const identifier::Nym& nym,
        const identifier::Generic& reply,
        otx::client::StorageBox box,
        alloc::Strategy alloc) const noexcept -> contract::peer::Reply final;
    auto PeerReplyComplete(
        const identifier::Nym& nym,
        const identifier::Generic& replyOrRequest) const -> bool final;
    auto PeerReplyCreate(
        const identifier::Nym& nym,
        const protobuf::PeerRequest& request,
        const protobuf::PeerReply& reply) const -> bool final;
    auto PeerReplyCreateRollback(
        const identifier::Nym& nym,
        const identifier::Generic& request,
        const identifier::Generic& reply) const -> bool final;
    auto PeerReplyFinished(const identifier::Nym& nym) const
        -> ObjectList final;
    auto PeerReplyIncoming(const identifier::Nym& nym) const
        -> ObjectList final;
    auto PeerReplyProcessed(const identifier::Nym& nym) const
        -> ObjectList final;
    auto PeerReplyReceive(const identifier::Nym& nym, const PeerObject& reply)
        const -> bool final;
    auto PeerReplySent(const identifier::Nym& nym) const -> ObjectList final;
    auto PeerRequest(
        const identifier::Nym& nym,
        const identifier::Generic& request,
        const otx::client::StorageBox& box,
        alloc::Strategy alloc) const noexcept -> contract::peer::Request final;
    auto PeerRequestComplete(
        const identifier::Nym& nym,
        const identifier::Generic& reply) const -> bool final;
    auto PeerRequestCreate(
        const identifier::Nym& nym,
        const protobuf::PeerRequest& request) const -> bool final;
    auto PeerRequestCreateRollback(
        const identifier::Nym& nym,
        const identifier::Generic& request) const -> bool final;
    auto PeerRequestDelete(
        const identifier::Nym& nym,
        const identifier::Generic& request,
        const otx::client::StorageBox& box) const -> bool final;
    auto PeerRequestFinished(const identifier::Nym& nym) const
        -> ObjectList final;
    auto PeerRequestIncoming(const identifier::Nym& nym) const
        -> ObjectList final;
    auto PeerRequestProcessed(const identifier::Nym& nym) const
        -> ObjectList final;
    auto PeerRequestReceive(
        const identifier::Nym& nym,
        const PeerObject& request) const -> bool final;
    auto PeerRequestSent(const identifier::Nym& nym) const -> ObjectList final;
    auto PeerRequestUpdate(
        const identifier::Nym& nym,
        const identifier::Generic& request,
        const otx::client::StorageBox& box) const -> bool final;
    auto PublishNotary(const identifier::Notary& id) const noexcept
        -> bool final;
    auto PublishNym(const identifier::Nym& id) const noexcept -> bool final;
    auto PublishUnit(const identifier::UnitDefinition& id) const noexcept
        -> bool final;
    auto Purse(
        const identifier::Nym& nym,
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        bool checking) const -> const otx::blind::Purse& final;
    auto RemoveServer(const identifier::Notary& id) const -> bool final;
    auto RemoveUnitDefinition(const identifier::UnitDefinition& id) const
        -> bool final;
    auto SaveCredential(const protobuf::Credential& credential) const
        -> bool final;
    auto SecurityContract(
        const UnallocatedCString& nymid,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const PasswordPrompt& reason,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement,
        const VersionNumber version = contract::Unit::DefaultVersion) const
        -> OTUnitDefinition final;
    auto Self() const noexcept -> const session::Wallet& final { return self_; }
    auto Server(const ReadView& contract) const -> OTServerContract final;
    auto Server(
        const UnallocatedCString& nymid,
        const UnallocatedCString& name,
        const UnallocatedCString& terms,
        const UnallocatedList<contract::Server::Endpoint>& endpoints,
        const PasswordPrompt& reason,
        const VersionNumber version) const -> OTServerContract final;
    auto Server(
        const identifier::Notary& id,
        const std::chrono::milliseconds& timeout = 0ms) const
        -> OTServerContract final;
    auto Server(const protobuf::ServerContract& contract) const
        -> OTServerContract final;
    auto ServerContext(
        const identifier::Nym& localNymID,
        const identifier::Generic& remoteID) const
        -> std::shared_ptr<const otx::context::Server> override;
    auto ServerList() const -> ObjectList final;
    auto SetDefaultNym(const identifier::Nym& id) const noexcept -> bool final;
    auto SetNymAlias(const identifier::Nym& id, std::string_view alias) const
        -> bool final;
    auto SetServerAlias(const identifier::Notary& id, std::string_view alias)
        const -> bool final;
    auto SetUnitDefinitionAlias(
        const identifier::UnitDefinition& id,
        std::string_view alias) const -> bool final;
    auto UnitDefinition(const ReadView contract) const
        -> OTUnitDefinition final;
    auto UnitDefinition(
        const identifier::UnitDefinition& id,
        const std::chrono::milliseconds& timeout = 0ms) const
        -> OTUnitDefinition final;
    auto UnitDefinition(const protobuf::UnitDefinition& contract) const
        -> OTUnitDefinition final;
    auto UnitDefinitionList() const -> ObjectList final;
    auto UpdateAccount(
        const identifier::Account& accountID,
        const otx::context::Server&,
        const String& serialized,
        const PasswordPrompt& reason) const -> bool final;
    auto UpdateAccount(
        const identifier::Account& accountID,
        const otx::context::Server&,
        const String& serialized,
        const UnallocatedCString& label,
        const PasswordPrompt& reason) const -> bool final;
    auto mutable_Account(
        const identifier::Account& accountID,
        const PasswordPrompt& reason,
        const AccountCallback callback) const -> ExclusiveAccount final;
    auto mutable_ClientContext(
        const identifier::Nym& remoteNymID,
        const PasswordPrompt& reason) const
        -> Editor<otx::context::Client> override;
    auto mutable_Issuer(
        const identifier::Nym& nymID,
        const identifier::Nym& issuerID) const
        -> Editor<otx::client::Issuer> final;
    auto mutable_Nym(const identifier::Nym& id, const PasswordPrompt& reason)
        const -> NymData final;
    auto mutable_Nymfile(
        const identifier::Nym& id,
        const PasswordPrompt& reason) const -> Editor<opentxs::NymFile> final;
    auto mutable_Purse(
        const identifier::Nym& nym,
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        const PasswordPrompt& reason,
        const otx::blind::CashType type) const
        -> Editor<otx::blind::Purse, std::shared_mutex> final;
    auto mutable_ServerContext(
        const identifier::Nym& localNymID,
        const identifier::Generic& remoteID,
        const PasswordPrompt& reason) const
        -> Editor<otx::context::Server> override;

    auto Self() noexcept -> session::Wallet& final { return self_; }

    WalletPrivate() = delete;
    WalletPrivate(const WalletPrivate&) = delete;
    WalletPrivate(WalletPrivate&&) = delete;
    auto operator=(const WalletPrivate&) -> WalletPrivate& = delete;
    auto operator=(WalletPrivate&&) -> WalletPrivate& = delete;

    ~WalletPrivate() override;

protected:
    using AccountLock =
        std::pair<std::shared_mutex, std::unique_ptr<opentxs::Account>>;
    using ContextID = std::pair<UnallocatedCString, UnallocatedCString>;
    using ContextMap = UnallocatedMap<
        ContextID,
        std::shared_ptr<otx::context::internal::Base>>;
    using GuardedContextMap = libguarded::plain_guarded<ContextMap>;

    session::Wallet self_;
    const api::Session& api_;
    mutable GuardedContextMap context_map_;

    auto context(
        const identifier::Nym& localNymID,
        const identifier::Nym& remoteNymID,
        ContextMap& map) const -> std::shared_ptr<otx::context::Base>;
    auto extract_unit(const identifier::UnitDefinition& contractID) const
        -> UnitType;
    auto extract_unit(const contract::Unit& contract) const -> UnitType;
    void save(
        const PasswordPrompt& reason,
        otx::context::internal::Base* context) const;
    auto server_to_nym(identifier::Generic& nymOrNotaryID) const
        -> identifier::Nym;

    WalletPrivate(const api::Session& api);

private:
    using AccountMap = UnallocatedMap<identifier::Generic, AccountLock>;
    using NymLock =
        std::pair<std::mutex, std::shared_ptr<identity::internal::Nym>>;
    using NymMap = UnallocatedMap<identifier::Nym, NymLock>;
    using ServerMap =
        UnallocatedMap<identifier::Notary, std::shared_ptr<contract::Server>>;
    using UnitMap = UnallocatedMap<
        identifier::UnitDefinition,
        std::shared_ptr<contract::Unit>>;
    using IssuerID = std::pair<identifier::Generic, identifier::Generic>;
    using IssuerLock =
        std::pair<std::mutex, std::shared_ptr<otx::client::Issuer>>;
    using IssuerMap = UnallocatedMap<IssuerID, IssuerLock>;
    using PurseID = std::
        tuple<identifier::Nym, identifier::Notary, identifier::UnitDefinition>;
    using PurseMap = UnallocatedMap<
        PurseID,
        std::pair<std::shared_mutex, otx::blind::Purse>>;
    using UnitNameMap =
        UnallocatedMap<UnallocatedCString, protobuf::ContactItemType>;
    using UnitNameReverse =
        UnallocatedMap<protobuf::ContactItemType, UnallocatedCString>;
    using GuardedSocket = libguarded::deferred_guarded<
        opentxs::network::zeromq::socket::Raw,
        std::shared_mutex>;

    mutable AccountMap account_map_;
    mutable NymMap nym_map_;
    mutable ServerMap server_map_;
    mutable UnitMap unit_map_;
    mutable IssuerMap issuer_map_;
    mutable std::mutex create_nym_lock_;
    mutable std::mutex account_map_lock_;
    mutable std::mutex nym_map_lock_;
    mutable std::mutex server_map_lock_;
    mutable std::mutex unit_map_lock_;
    mutable std::mutex issuer_map_lock_;
    mutable std::mutex peer_map_lock_;
    mutable UnallocatedMap<UnallocatedCString, std::mutex> peer_lock_;
    mutable std::mutex nymfile_map_lock_;
    mutable UnallocatedMap<identifier::Generic, std::mutex> nymfile_lock_;
    mutable std::mutex purse_lock_;
    mutable PurseMap purse_map_;
    OTZMQPublishSocket account_publisher_;
    OTZMQPublishSocket issuer_publisher_;
    OTZMQPublishSocket nym_publisher_;
    OTZMQPublishSocket nym_created_publisher_;
    OTZMQPublishSocket server_publisher_;
    OTZMQPublishSocket unit_publisher_;
    OTZMQPublishSocket peer_reply_publisher_;
    OTZMQPublishSocket peer_reply_new_publisher_;
    OTZMQPublishSocket peer_request_publisher_;
    OTZMQPublishSocket peer_request_new_publisher_;
    OTZMQPushSocket find_nym_;
    opentxs::network::zeromq::internal::Handle handle_;
    opentxs::network::zeromq::internal::Batch& batch_;
    opentxs::network::zeromq::ListenCallback& p2p_callback_;
    opentxs::network::zeromq::socket::Raw& p2p_socket_;
    opentxs::network::zeromq::socket::Raw& loopback_;
    mutable GuardedSocket to_loopback_;
    opentxs::network::zeromq::internal::Thread* thread_;

    static auto reverse_unit_map(const UnitNameMap& map) -> UnitNameReverse;

    auto account_alias(
        const UnallocatedCString& accountID,
        const UnallocatedCString& hint) const -> UnallocatedCString;
    auto account_factory(
        const identifier::Account& accountID,
        std::string_view alias,
        const UnallocatedCString& serialized) const -> opentxs::Account*;
    virtual void instantiate_client_context(
        const protobuf::Context& serialized,
        const Nym_p& localNym,
        const Nym_p& remoteNym,
        std::shared_ptr<otx::context::internal::Base>& output) const
    {
    }
    virtual void instantiate_server_context(
        const protobuf::Context& serialized,
        const Nym_p& localNym,
        const Nym_p& remoteNym,
        std::shared_ptr<otx::context::internal::Base>& output) const
    {
    }
    virtual auto load_legacy_account(
        const identifier::Account& accountID,
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
    auto notify_changed(const identifier::Nym& id) const noexcept -> void;
    auto notify_new(const identifier::Nym& id) const noexcept -> void;
    virtual void nym_to_contact(
        [[maybe_unused]] const identity::Nym& nym,
        [[maybe_unused]] const UnallocatedCString& name) const noexcept
    {
    }
    auto nymfile_lock(const identifier::Nym& nymID) const -> std::mutex&;
    auto peer_lock(const UnallocatedCString& nymID) const -> std::mutex&;
    auto process_p2p(opentxs::network::zeromq::Message&& msg) const noexcept
        -> void;
    auto process_p2p_query_contract(
        opentxs::network::zeromq::Message&& msg) const noexcept -> void;
    auto process_p2p_publish_contract(
        opentxs::network::zeromq::Message&& msg) const noexcept -> void;
    auto process_p2p_response(
        opentxs::network::zeromq::Message&& msg) const noexcept -> void;
    auto publish_server(const identifier::Notary& id) const noexcept -> void;
    auto publish_unit(const identifier::UnitDefinition& id) const noexcept
        -> void;
    auto purse(
        const identifier::Nym& nym,
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        ErrorReporting checking) const -> PurseMap::mapped_type&;
    void save(
        const PasswordPrompt& reason,
        const identifier::Account& id,
        std::unique_ptr<opentxs::Account>& in,
        eLock& lock,
        bool success) const;
    void save(const Lock& lock, otx::client::Issuer* in) const;
    void save(
        const eLock& lock,
        const identifier::Nym nym,
        otx::blind::Purse* in) const;
    void save(NymData* nymData, const Lock& lock) const;
    void save(
        const PasswordPrompt& reason,
        opentxs::NymFile* nym,
        const Lock& lock) const;
    auto SaveCredentialIDs(const identity::Nym& nym) const -> bool;
    auto search_notary(const identifier::Notary& id) const noexcept -> void;
    auto search_nym(const identifier::Nym& id) const noexcept -> void;
    auto search_unit(const identifier::UnitDefinition& id) const noexcept
        -> void;
    virtual auto signer_nym(const identifier::Nym& id) const -> Nym_p = 0;

    /* Throws std::out_of_range for missing accounts */
    auto account(
        const Lock& lock,
        const identifier::Account& accountID,
        const bool create) const -> AccountLock&;
    auto issuer(
        const identifier::Nym& nymID,
        const identifier::Nym& issuerID,
        const bool create) const -> IssuerLock&;

    auto server(std::unique_ptr<contract::Server> contract) const
        noexcept(false) -> OTServerContract;
    auto unit_definition(std::shared_ptr<contract::Unit>&& contract) const
        -> OTUnitDefinition;
};
}  // namespace opentxs::api::session

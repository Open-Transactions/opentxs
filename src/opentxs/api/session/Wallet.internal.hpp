// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <string_view>
#include <tuple>
#include <utility>

#include "internal/core/contract/BasketContract.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Exclusive.hpp"
#include "internal/util/Shared.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/otx/blind/CashType.hpp"  // IWYU pragma: keep
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace internal
{
class Wallet;  // IWYU pragma: keep
}  // namespace internal
}  // namespace session
}  // namespace api

namespace contract
{
namespace peer
{
class Reply;
class Request;
}  // namespace peer
}  // namespace contract

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
class Base;
class Client;
class Server;
}  // namespace context
}  // namespace otx

namespace proto
{
class Credential;
class Nym;
class PeerReply;
class PeerRequest;
class ServerContract;
class UnitDefinition;
}  // namespace proto

class NymFile;
class PasswordPrompt;
class PeerObject;
class String;

using ExclusiveAccount = Exclusive<Account>;
using SharedAccount = Shared<Account>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
/** AccountInfo: accountID, nymID, serverID, unitID*/
using AccountInfo = std::tuple<
    identifier::Generic,
    identifier::Nym,
    identifier::Notary,
    identifier::UnitDefinition>;
}  // namespace opentxs

class opentxs::api::session::internal::Wallet
{
public:
    using AccountCallback = std::function<void(const Account&)>;

    static auto Detach(session::Wallet& self) noexcept -> void;

    virtual auto Account(const identifier::Account& accountID) const
        -> SharedAccount = 0;
    virtual auto AccountPartialMatch(const UnallocatedCString& hint) const
        -> identifier::Generic = 0;
    virtual auto BasketContract(
        const identifier::UnitDefinition& id,
        const std::chrono::milliseconds& timeout = std::chrono::milliseconds(
            0)) const noexcept(false) -> OTBasketContract = 0;
    virtual auto ClientContext(const identifier::Nym& remoteNymID) const
        -> std::shared_ptr<const otx::context::Client> = 0;
    virtual auto Context(
        const identifier::Notary& notaryID,
        const identifier::Nym& clientNymID) const
        -> std::shared_ptr<const otx::context::Base> = 0;
    virtual auto CreateAccount(
        const identifier::Nym& ownerNymID,
        const identifier::Notary& notaryID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const identity::Nym& signer,
        Account::AccountType type,
        TransactionNumber stash,
        const PasswordPrompt& reason) const -> ExclusiveAccount = 0;
    virtual auto CurrencyContract(
        const UnallocatedCString& nymid,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const Amount& redemptionIncrement,
        const PasswordPrompt& reason) const noexcept(false)
        -> OTUnitDefinition = 0;
    virtual auto CurrencyContract(
        const UnallocatedCString& nymid,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const Amount& redemptionIncrement,
        const VersionNumber version,
        const PasswordPrompt& reason) const noexcept(false)
        -> OTUnitDefinition = 0;
    virtual auto CurrencyContract(
        const UnallocatedCString& nymid,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const Amount& redemptionIncrement,
        const display::Definition& displayDefinition,
        const PasswordPrompt& reason) const noexcept(false)
        -> OTUnitDefinition = 0;
    virtual auto CurrencyContract(
        const UnallocatedCString& nymid,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const Amount& redemptionIncrement,
        const display::Definition& displayDefinition,
        const VersionNumber version,
        const PasswordPrompt& reason) const noexcept(false)
        -> OTUnitDefinition = 0;
    virtual auto CurrencyTypeBasedOnUnitType(
        const identifier::UnitDefinition& contractID) const -> UnitType = 0;
    virtual auto DefaultNym() const noexcept
        -> std::pair<identifier::Nym, std::size_t> = 0;
    virtual auto DeleteAccount(const identifier::Account& accountID) const
        -> bool = 0;
    [[deprecated]] virtual auto ImportAccount(
        std::unique_ptr<opentxs::Account>& imported) const -> bool = 0;
    virtual auto IsLocalNym(const identifier::Nym& id) const -> bool = 0;
    virtual auto IsLocalNym(const std::string_view id) const -> bool = 0;
    virtual auto Issuer(
        const identifier::Nym& nymID,
        const identifier::Nym& issuerID) const
        -> std::shared_ptr<const otx::client::Issuer> = 0;
    virtual auto IssuerAccount(const identifier::UnitDefinition& unitID) const
        -> SharedAccount = 0;
    virtual auto IssuerList(const identifier::Nym& nymID) const
        -> UnallocatedSet<identifier::Nym> = 0;
    virtual auto LoadCredential(
        const identifier::Generic& id,
        std::shared_ptr<proto::Credential>& credential) const -> bool = 0;
    virtual auto LocalNymCount() const -> std::size_t = 0;
    virtual auto LocalNyms() const -> Set<identifier::Nym> = 0;
    virtual auto Nym(
        const PasswordPrompt& reason,
        const UnallocatedCString& name) const -> Nym_p = 0;
    virtual auto Nym(const ReadView& bytes) const -> Nym_p = 0;
    virtual auto Nym(
        const identifier::Nym& id,
        const std::chrono::milliseconds& timeout = 0ms) const -> Nym_p = 0;
    virtual auto Nym(
        const identity::Type type,
        const PasswordPrompt& reason,
        const UnallocatedCString& name) const -> Nym_p = 0;
    virtual auto Nym(
        const opentxs::crypto::Parameters& parameters,
        const PasswordPrompt& reason,
        const UnallocatedCString& name) const -> Nym_p = 0;
    virtual auto Nym(
        const opentxs::crypto::Parameters& parameters,
        const identity::Type type,
        const PasswordPrompt& reason,
        const UnallocatedCString& name) const -> Nym_p = 0;
    virtual auto Nym(const proto::Nym& nym) const -> Nym_p = 0;
    virtual auto NymByIDPartialMatch(const UnallocatedCString& partialId) const
        -> Nym_p = 0;
    virtual auto NymList() const -> ObjectList = 0;
    virtual auto NymNameByIndex(const std::size_t index, String& name) const
        -> bool = 0;
    virtual auto Nymfile(
        const identifier::Nym& id,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<const opentxs::NymFile> = 0;
    virtual auto PeerReply(
        const identifier::Nym& nym,
        const identifier::Generic& reply,
        otx::client::StorageBox box,
        alloc::Strategy alloc = {}) const noexcept -> contract::peer::Reply = 0;
    virtual auto PeerReplyComplete(
        const identifier::Nym& nym,
        const identifier::Generic& replyOrRequest) const -> bool = 0;
    virtual auto PeerReplyCreate(
        const identifier::Nym& nym,
        const proto::PeerRequest& request,
        const proto::PeerReply& reply) const -> bool = 0;
    virtual auto PeerReplyCreateRollback(
        const identifier::Nym& nym,
        const identifier::Generic& request,
        const identifier::Generic& reply) const -> bool = 0;
    virtual auto PeerReplyFinished(const identifier::Nym& nym) const
        -> ObjectList = 0;
    virtual auto PeerReplyIncoming(const identifier::Nym& nym) const
        -> ObjectList = 0;
    virtual auto PeerReplyProcessed(const identifier::Nym& nym) const
        -> ObjectList = 0;
    virtual auto PeerReplyReceive(
        const identifier::Nym& nym,
        const PeerObject& reply) const -> bool = 0;
    virtual auto PeerReplySent(const identifier::Nym& nym) const
        -> ObjectList = 0;
    virtual auto PeerRequest(
        const identifier::Nym& nym,
        const identifier::Generic& request,
        const otx::client::StorageBox& box,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::Request = 0;
    virtual auto PeerRequestComplete(
        const identifier::Nym& nym,
        const identifier::Generic& reply) const -> bool = 0;
    virtual auto PeerRequestCreate(
        const identifier::Nym& nym,
        const proto::PeerRequest& request) const -> bool = 0;
    virtual auto PeerRequestCreateRollback(
        const identifier::Nym& nym,
        const identifier::Generic& request) const -> bool = 0;
    virtual auto PeerRequestDelete(
        const identifier::Nym& nym,
        const identifier::Generic& request,
        const otx::client::StorageBox& box) const -> bool = 0;
    virtual auto PeerRequestFinished(const identifier::Nym& nym) const
        -> ObjectList = 0;
    virtual auto PeerRequestIncoming(const identifier::Nym& nym) const
        -> ObjectList = 0;
    virtual auto PeerRequestProcessed(const identifier::Nym& nym) const
        -> ObjectList = 0;
    virtual auto PeerRequestReceive(
        const identifier::Nym& nym,
        const PeerObject& request) const -> bool = 0;
    virtual auto PeerRequestSent(const identifier::Nym& nym) const
        -> ObjectList = 0;
    virtual auto PeerRequestUpdate(
        const identifier::Nym& nym,
        const identifier::Generic& request,
        const otx::client::StorageBox& box) const -> bool = 0;
    virtual auto PublishNotary(const identifier::Notary& id) const noexcept
        -> bool = 0;
    virtual auto PublishNym(const identifier::Nym& id) const noexcept
        -> bool = 0;
    virtual auto PublishUnit(
        const identifier::UnitDefinition& id) const noexcept -> bool = 0;
    virtual auto Purse(
        const identifier::Nym& nym,
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        const bool checking) const -> const otx::blind::Purse& = 0;
    virtual auto RemoveServer(const identifier::Notary& id) const -> bool = 0;
    virtual auto RemoveUnitDefinition(
        const identifier::UnitDefinition& id) const -> bool = 0;
    virtual auto SaveCredential(const proto::Credential& credential) const
        -> bool = 0;
    virtual auto SecurityContract(
        const UnallocatedCString& nymid,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const PasswordPrompt& reason,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement,
        const VersionNumber version = contract::Unit::DefaultVersion) const
        noexcept(false) -> OTUnitDefinition = 0;
    virtual auto Self() const noexcept -> const session::Wallet& = 0;
    virtual auto Server(const ReadView& contract) const noexcept(false)
        -> OTServerContract = 0;
    virtual auto Server(
        const UnallocatedCString& nymid,
        const UnallocatedCString& name,
        const UnallocatedCString& terms,
        const UnallocatedList<contract::Server::Endpoint>& endpoints,
        const PasswordPrompt& reason,
        const VersionNumber version) const noexcept(false)
        -> OTServerContract = 0;
    virtual auto Server(
        const identifier::Notary& id,
        const std::chrono::milliseconds& timeout = std::chrono::milliseconds(
            0)) const noexcept(false) -> OTServerContract = 0;
    virtual auto Server(const proto::ServerContract& contract) const
        noexcept(false) -> OTServerContract = 0;
    virtual auto ServerContext(
        const identifier::Nym& localNymID,
        const identifier::Generic& remoteID) const
        -> std::shared_ptr<const otx::context::Server> = 0;
    virtual auto ServerList() const -> ObjectList = 0;
    virtual auto SetDefaultNym(const identifier::Nym& id) const noexcept
        -> bool = 0;
    virtual auto SetNymAlias(const identifier::Nym& id, std::string_view alias)
        const -> bool = 0;
    virtual auto SetServerAlias(
        const identifier::Notary& id,
        std::string_view alias) const -> bool = 0;
    virtual auto SetUnitDefinitionAlias(
        const identifier::UnitDefinition& id,
        std::string_view alias) const -> bool = 0;
    virtual auto UnitDefinition(const ReadView contract) const noexcept(false)
        -> OTUnitDefinition = 0;
    virtual auto UnitDefinition(
        const identifier::UnitDefinition& id,
        const std::chrono::milliseconds& timeout = std::chrono::milliseconds(
            0)) const noexcept(false) -> OTUnitDefinition = 0;
    virtual auto UnitDefinition(const proto::UnitDefinition& contract) const
        noexcept(false) -> OTUnitDefinition = 0;
    virtual auto UnitDefinitionList() const -> ObjectList = 0;
    virtual auto UpdateAccount(
        const identifier::Account& accountID,
        const otx::context::Server&,
        const String& serialized,
        const PasswordPrompt& reason) const -> bool = 0;
    virtual auto UpdateAccount(
        const identifier::Account& accountID,
        const otx::context::Server&,
        const String& serialized,
        const UnallocatedCString& label,
        const PasswordPrompt& reason) const -> bool = 0;
    virtual auto mutable_Account(
        const identifier::Account& accountID,
        const PasswordPrompt& reason,
        const AccountCallback callback = nullptr) const -> ExclusiveAccount = 0;
    virtual auto mutable_ClientContext(
        const identifier::Nym& remoteNymID,
        const PasswordPrompt& reason) const -> Editor<otx::context::Client> = 0;
    virtual auto mutable_Context(
        const identifier::Notary& notaryID,
        const identifier::Nym& clientNymID,
        const PasswordPrompt& reason) const -> Editor<otx::context::Base> = 0;
    virtual auto mutable_Issuer(
        const identifier::Nym& nymID,
        const identifier::Nym& issuerID) const
        -> Editor<otx::client::Issuer> = 0;
    virtual auto mutable_Nym(
        const identifier::Nym& id,
        const PasswordPrompt& reason) const -> NymData = 0;
    virtual auto mutable_Nymfile(
        const identifier::Nym& id,
        const PasswordPrompt& reason) const -> Editor<opentxs::NymFile> = 0;
    virtual auto mutable_Purse(
        const identifier::Nym& nym,
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        const PasswordPrompt& reason,
        const otx::blind::CashType = otx::blind::CashType::Lucre) const
        -> Editor<otx::blind::Purse, std::shared_mutex> = 0;
    virtual auto mutable_ServerContext(
        const identifier::Nym& localNymID,
        const identifier::Generic& remoteID,
        const PasswordPrompt& reason) const -> Editor<otx::context::Server> = 0;

    virtual auto Self() noexcept -> session::Wallet& = 0;

    virtual ~Wallet() = default;
};

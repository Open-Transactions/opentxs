// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>

#include "internal/core/contract/BasketContract.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Exclusive.hpp"
#include "internal/util/Shared.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/otx/blind/CashType.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace contract
{
namespace peer
{
class Reply;
class Request;
}  // namespace peer
}  // namespace contract

namespace display
{
class Definition;
}  // namespace display

namespace otx
{
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

class Account;
class NymFile;

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

namespace opentxs::api::session::internal
{
class Wallet : virtual public api::session::Wallet
{
public:
    using AccountCallback = std::function<void(const Account&)>;

    virtual auto Account(const identifier::Account& accountID) const
        -> SharedAccount = 0;
    virtual auto BasketContract(
        const identifier::UnitDefinition& id,
        const std::chrono::milliseconds& timeout = std::chrono::milliseconds(
            0)) const noexcept(false) -> OTBasketContract = 0;
    /**   Load a read-only copy of a ClientContext object
     *
     *    \param[in] remoteNymID context identifier (usually the other party's
     *                           nym id)
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    virtual auto ClientContext(const identifier::Nym& remoteNymID) const
        -> std::shared_ptr<const otx::context::Client> = 0;
    /**   Load a read-only copy of a Context object
     *
     *    This method should only be called if the specific client or server
     *    version is not available (such as by classes common to client and
     *    server).
     *
     *    \param[in] notaryID     remote notary
     *    \param[in] clientNymID  local nym
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
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
    /**   Create a new currency contract
     *
     *    \param[in] nymid the identifier of nym which will create the contract
     *    \param[in] shortname a short human-readable identifier for the
     *                         contract
     *    \param[in] terms human-readable terms and conditions
     *    \throw std::runtime_error the contract can not be created
     */
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
        const display::Definition& displayDefinition,
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
        const VersionNumber version,
        const PasswordPrompt& reason) const noexcept(false)
        -> OTUnitDefinition = 0;
    virtual auto CurrencyTypeBasedOnUnitType(
        const identifier::UnitDefinition& contractID) const -> UnitType = 0;
    [[deprecated]] virtual auto ImportAccount(
        std::unique_ptr<opentxs::Account>& imported) const -> bool = 0;
    auto Internal() const noexcept -> const Wallet& final { return *this; }
    virtual auto IssuerAccount(const identifier::UnitDefinition& unitID) const
        -> SharedAccount = 0;
    virtual auto LoadCredential(
        const identifier::Generic& id,
        std::shared_ptr<proto::Credential>& credential) const -> bool = 0;
    virtual auto mutable_Account(
        const identifier::Account& accountID,
        const PasswordPrompt& reason,
        const AccountCallback callback = nullptr) const -> ExclusiveAccount = 0;
    /**   Load an existing Context object
     *
     *    This method should only be called if the specific client or server
     *    version is not available (such as by classes common to client and
     *    server).
     *
     *    WARNING: The context being loaded via this function must exist or else
     *    the function will assert.
     *
     *    \param[in] notaryID the identifier of the nym who owns the context
     *    \param[in] clientNymID context identifier (usually the other party's
     *                           nym id)
     */
    virtual auto mutable_Context(
        const identifier::Notary& notaryID,
        const identifier::Nym& clientNymID,
        const PasswordPrompt& reason) const -> Editor<otx::context::Base> = 0;
    /**   Load or create a ClientContext object
     *
     *    \param[in] remoteNymID context identifier (usually the other party's
     *                           nym id)
     */
    virtual auto mutable_ClientContext(
        const identifier::Nym& remoteNymID,
        const PasswordPrompt& reason) const -> Editor<otx::context::Client> = 0;
    virtual auto Issuer(
        const identifier::Nym& nymID,
        const identifier::Nym& issuerID) const
        -> std::shared_ptr<const otx::client::Issuer> = 0;
    /**   Load or create an Issuer object
     *
     *    \param[in] nymID the identifier of the local nym
     *    \param[in] issuerID the identifier of the issuer nym
     */
    virtual auto mutable_Issuer(
        const identifier::Nym& nymID,
        const identifier::Nym& issuerID) const
        -> Editor<otx::client::Issuer> = 0;

    using session::Wallet::Nym;
    virtual auto Nym(const proto::Nym& nym) const -> Nym_p = 0;

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
    /**   Load or create a ServerContext object
     *
     *    \param[in] localNymID the identifier of the nym who owns the context
     *    \param[in] remoteID context identifier (usually the other party's nym
     *                        id)
     */
    virtual auto mutable_ServerContext(
        const identifier::Nym& localNymID,
        const identifier::Generic& remoteID,
        const PasswordPrompt& reason) const -> Editor<otx::context::Server> = 0;
    virtual auto Nymfile(
        const identifier::Nym& id,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<const opentxs::NymFile> = 0;
    /**   Store the recipient's copy of a peer reply
     *
     *    The peer reply is stored in the SendPeerReply box for the
     *    specified nym.
     *
     *    The corresponding request is moved from the nym's IncomingPeerRequest
     *    box to the ProcessedPeerRequest box.
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the corresponding request
     *    \param[in] reply the serialized peer reply object
     *    \returns true if the request is successfully stored
     */
    virtual auto PeerReplyCreate(
        const identifier::Nym& nym,
        const proto::PeerRequest& request,
        const proto::PeerReply& reply) const -> bool = 0;
    /**   Store the initiator's copy of a peer request
     *
     *    The peer request is stored in the SentPeerRequest box for the
     *    specified nym.
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the serialized peer request object
     *    \returns true if the request is successfully stored
     */
    virtual auto PeerRequestCreate(
        const identifier::Nym& nym,
        const proto::PeerRequest& request) const -> bool = 0;
    virtual auto PublishNotary(const identifier::Notary& id) const noexcept
        -> bool = 0;
    virtual auto PublishNym(const identifier::Nym& id) const noexcept
        -> bool = 0;
    virtual auto PublishUnit(
        const identifier::UnitDefinition& id) const noexcept -> bool = 0;
    virtual auto SaveCredential(const proto::Credential& credential) const
        -> bool = 0;
    /**   Create a new security contract
     *
     *    \param[in] nymid the identifier of nym which will create the contract
     *    \param[in] shortname a short human-readable identifier for the
     *                         contract
     *    \param[in] terms human-readable terms and conditions
     *    \throw std::runtime_error the contract can not be created
     */
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
    /**   Obtain an instantiated server contract.
     *
     *    If the caller is willing to accept a network lookup delay, it can
     *    specify a timeout to be used in the event that the contract can not
     *    be located in local storage and must be queried from a remote
     *    location.
     *
     *    If no timeout is specified, the remote query will still happen in the
     *    background, but this method will return immediately with a null
     *    result.
     *
     *    \param[in] id the identifier of the contract to be returned
     *    \param[in] timeout The caller can set a non-zero value here if it's
     *                       willing to wait for a network lookup. The default
     *                       value of 0 will return immediately.
     *    \throw std::runtime_error the specified contract does not exist in the
     *                              wallet
     */
    virtual auto Server(
        const identifier::Notary& id,
        const std::chrono::milliseconds& timeout = std::chrono::milliseconds(
            0)) const noexcept(false) -> OTServerContract = 0;

    /**   Instantiate a server contract from serialized form
     *
     *    \param[in] contract the serialized version of the contract
     *    \throw std::runtime_error the provided contract is not valid
     */
    virtual auto Server(const ReadView& contract) const noexcept(false)
        -> OTServerContract = 0;

    /**   Create a new server contract
     *
     *    \param[in] nymid      the identifier of nym which will create the
     *                          contract
     *    \param[in] name       the official name of the server
     *    \param[in] terms      human-readable server description & terms of use
     *    \param[in] endpoints  list of notary endpoints to include in the
     *                          contract
     *    \param[in] reason     password prompt data
     *    \param[in] version    contract version to create
     *    \throw std::runtime_error the contract can not be created
     */
    virtual auto Server(
        const UnallocatedCString& nymid,
        const UnallocatedCString& name,
        const UnallocatedCString& terms,
        const UnallocatedList<contract::Server::Endpoint>& endpoints,
        const PasswordPrompt& reason,
        const VersionNumber version) const noexcept(false)
        -> OTServerContract = 0;
    /**   Instantiate a server contract from serialized form
     *
     *    \param[in] contract the serialized version of the contract
     *    \throw std::runtime_error the provided contract is not valid
     */
    virtual auto Server(const proto::ServerContract& contract) const
        noexcept(false) -> OTServerContract = 0;
    /**   Load a read-only copy of a ServerContext object
     *
     *    \param[in] localNymID the identifier of the nym who owns the context
     *    \param[in] remoteID context identifier (usually the other party's nym
     *                       id)
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    virtual auto ServerContext(
        const identifier::Nym& localNymID,
        const identifier::Generic& remoteID) const
        -> std::shared_ptr<const otx::context::Server> = 0;
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
    /**   Obtain an instantiated unit definition contract.
     *
     *    If the caller is willing to accept a network lookup delay, it can
     *    specify a timeout to be used in the event that the contract can not
     *    be located in local storage and must be queried from a remote
     *    location.
     *
     *    If no timeout is specified, the remote query will still happen in the
     *    background, but this method will return immediately with a null
     *    result.
     *
     *    \param[in] id the identifier of the contract to be returned
     *    \param[in] timeout The caller can set a non-zero value here if it's
     *                     willing to wait for a network lookup. The default
     *                     value of 0 will return immediately.
     *    \throw std::runtime_error the specified contract does not exist in the
     *                              wallet
     */
    virtual auto UnitDefinition(
        const identifier::UnitDefinition& id,
        const std::chrono::milliseconds& timeout = std::chrono::milliseconds(
            0)) const noexcept(false) -> OTUnitDefinition = 0;
    /**   Instantiate a unit definition contract from serialized form
     *
     *    \param[in] contract the protobuf serialized version of the contract
     *    \throw std::runtime_error the provided contract is invalid
     */
    virtual auto UnitDefinition(const ReadView contract) const noexcept(false)
        -> OTUnitDefinition = 0;
    /**   Instantiate a unit definition contract from serialized form
     *
     *    \param[in] contract the serialized version of the contract
     *    \throw std::runtime_error the provided contract is invalid
     */
    virtual auto UnitDefinition(const proto::UnitDefinition& contract) const
        noexcept(false) -> OTUnitDefinition = 0;

    auto Internal() noexcept -> Wallet& final { return *this; }

    ~Wallet() override = default;
};
}  // namespace opentxs::api::session::internal

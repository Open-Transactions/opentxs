// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>

#include "internal/otx/common/Account.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Exclusive.hpp"
#include "internal/util/Shared.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/otx/blind/CashType.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace otx
{
namespace client
{
class Issuer;
}  // namespace client
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

using ExclusiveAccount = Exclusive<Account>;
using SharedAccount = Shared<Account>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::internal
{
class Wallet : virtual public api::session::Wallet
{
public:
    virtual auto Account(const identifier::Generic& accountID) const
        -> SharedAccount = 0;
    virtual auto CreateAccount(
        const identifier::Nym& ownerNymID,
        const identifier::Notary& notaryID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const identity::Nym& signer,
        Account::AccountType type,
        TransactionNumber stash,
        const PasswordPrompt& reason) const -> ExclusiveAccount = 0;
    auto Internal() const noexcept -> const Wallet& final { return *this; }
    virtual auto IssuerAccount(const identifier::UnitDefinition& unitID) const
        -> SharedAccount = 0;
    virtual auto LoadCredential(
        const UnallocatedCString& id,
        std::shared_ptr<proto::Credential>& credential) const -> bool = 0;
    virtual auto mutable_Account(
        const identifier::Generic& accountID,
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
    using session::Wallet::PeerReply;
    /**   Load a peer reply object
     *
     *    \param[in]  nym the identifier of the nym who owns the object
     *    \param[in]  reply the identifier of the peer reply object
     *    \param[in]  box the box from which to retrive the peer object
     *    \param[out] serialized the serialized form of the requested object
     *    \returns true if the object with the specified id was loaded
     */
    virtual auto PeerReply(
        const identifier::Nym& nym,
        const identifier::Generic& reply,
        const otx::client::StorageBox& box,
        proto::PeerReply& serialized) const -> bool = 0;
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
    using session::Wallet::PeerRequest;
    /**   Load a peer reply object
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the peer reply object
     *    \param[in] box the box from which to retrive the peer object
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    virtual auto PeerRequest(
        const identifier::Nym& nym,
        const identifier::Generic& request,
        const otx::client::StorageBox& box,
        std::time_t& time,
        proto::PeerRequest& serialized) const -> bool = 0;
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
    using session::Wallet::Server;
    /**   Instantiate a server contract from serialized form
     *
     *    \param[in] contract the serialized version of the contract
     *    \throw std::runtime_error the provided contract is not valid
     */
    virtual auto Server(const proto::ServerContract& contract) const
        noexcept(false) -> OTServerContract = 0;
    using session::Wallet::UnitDefinition;
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

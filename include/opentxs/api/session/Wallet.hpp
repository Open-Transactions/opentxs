// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/identity/IdentityType.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>
#include <ctime>
#include <memory>

#include "opentxs/Types.hpp"
#include "opentxs/core/contract/BasketContract.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/Unit.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Parameters;
}  // namespace crypto

namespace session
{
namespace internal
{
class Wallet;
}  // namespace internal
}  // namespace session
}  // namespace api

namespace display
{
class Definition;
}  // namespace display

namespace otx
{
namespace blind
{
class Purse;
}  // namespace blind

namespace context
{
class Base;
class Client;
class Server;
}  // namespace context
}  // namespace otx

class Account;
class NymData;
class PeerObject;
}  // namespace opentxs

namespace opentxs
{
/** AccountInfo: accountID, nymID, serverID, unitID*/
using AccountInfo = std::tuple<OTIdentifier, OTNymID, OTNotaryID, OTUnitID>;
}  // namespace opentxs

namespace opentxs::api::session
{
/** \brief This class manages instantiated contracts and provides easy access
 *  to them.
 *
 * \ingroup native
 *
 *  It includes functionality which was previously found in OTWallet, and adds
 *  new capabilities such as the ability to (optionally) automatically perform
 *  remote lookups for contracts which are not already present in the local
 *  database.
 */
class OPENTXS_EXPORT Wallet
{
public:
    using AccountCallback = std::function<void(const Account&)>;

    virtual auto AccountPartialMatch(const UnallocatedCString& hint) const
        -> OTIdentifier = 0;
    virtual auto DeleteAccount(const Identifier& accountID) const -> bool = 0;
    virtual auto UpdateAccount(
        const Identifier& accountID,
        const otx::context::Server&,
        const String& serialized,
        const PasswordPrompt& reason) const -> bool = 0;
    virtual auto UpdateAccount(
        const Identifier& accountID,
        const otx::context::Server&,
        const String& serialized,
        const UnallocatedCString& label,
        const PasswordPrompt& reason) const -> bool = 0;
    [[deprecated]] virtual auto ImportAccount(
        std::unique_ptr<opentxs::Account>& imported) const -> bool = 0;

    /**   Load a read-only copy of a Context object
     *
     *    This method should only be called if the specific client or server
     *    version is not available (such as by classes common to client and
     *    server).
     *
     *    \param[in] notaryID
     *    \param[in] clientNymID
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    virtual auto Context(
        const identifier::Notary& notaryID,
        const identifier::Nym& clientNymID) const
        -> std::shared_ptr<const otx::context::Base> = 0;

    /**   Load a read-only copy of a ClientContext object
     *
     *    \param[in] remoteNymID context identifier (usually the other party's
     *                           nym id)
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    virtual auto ClientContext(const identifier::Nym& remoteNymID) const
        -> std::shared_ptr<const otx::context::Client> = 0;

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
        const Identifier& remoteID) const
        -> std::shared_ptr<const otx::context::Server> = 0;

    /**   Returns a list of all issuers associated with a local nym */
    virtual auto IssuerList(const identifier::Nym& nymID) const
        -> UnallocatedSet<OTNymID> = 0;

    virtual auto IsLocalNym(const UnallocatedCString& id) const -> bool = 0;
    virtual auto IsLocalNym(const identifier::Nym& id) const -> bool = 0;

    virtual auto LocalNymCount() const -> std::size_t = 0;

    virtual auto LocalNyms() const -> UnallocatedSet<OTNymID> = 0;

    /**   Obtain a smart pointer to an instantiated nym.
     *
     *    The smart pointer will not be initialized if the object does not
     *    exist or is invalid.
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
     *    \param[in] id the identifier of the nym to be returned
     *    \param[in] timeout The caller can set a non-zero value here if it's
     *                       willing to wait for a network lookup. The default
     *                       value of 0 will return immediately.
     */
    virtual auto Nym(
        const identifier::Nym& id,
        const std::chrono::milliseconds& timeout =
            std::chrono::milliseconds(0)) const -> Nym_p = 0;

    /**   Instantiate a nym from serialized form
     *
     *    The smart pointer will not be initialized if the provided serialized
     *    contract is invalid.
     *
     *    \param[in] nym the serialized version of the contract
     */
    virtual auto Nym(const ReadView& bytes) const -> Nym_p = 0;

    virtual auto Nym(
        const identity::Type type,
        const PasswordPrompt& reason,
        const UnallocatedCString& name = {}) const -> Nym_p = 0;
    virtual auto Nym(
        const opentxs::crypto::Parameters& parameters,
        const PasswordPrompt& reason,
        const UnallocatedCString& name = {}) const -> Nym_p = 0;
    virtual auto Nym(
        const PasswordPrompt& reason,
        const UnallocatedCString& name = {}) const -> Nym_p = 0;
    virtual auto Nym(
        const opentxs::crypto::Parameters& parameters,
        const identity::Type type,
        const PasswordPrompt& reason,
        const UnallocatedCString& name = {}) const -> Nym_p = 0;

    virtual auto mutable_Nym(
        const identifier::Nym& id,
        const PasswordPrompt& reason) const -> NymData = 0;

    virtual auto Nymfile(
        const identifier::Nym& id,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<const opentxs::NymFile> = 0;

    virtual auto NymByIDPartialMatch(const UnallocatedCString& partialId) const
        -> Nym_p = 0;

    /**   Returns a list of all known nyms and their aliases
     */
    virtual auto NymList() const -> ObjectList = 0;

    virtual auto NymNameByIndex(const std::size_t index, String& name) const
        -> bool = 0;

    /**   Load a peer reply object
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the peer reply object
     *    \param[in] box the box from which to retrive the peer object
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    virtual auto PeerReply(
        const identifier::Nym& nym,
        const Identifier& reply,
        const StorageBox& box,
        AllocateOutput destination) const -> bool = 0;

    /**   Clean up the recipient's copy of a peer reply
     *
     *    The peer reply is moved from the nym's SentPeerReply
     *    box to the FinishedPeerReply box.
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] replyOrRequest the identifier of the peer reply object, or
     *               the id of its corresponding request
     *    \returns true if the request is successfully stored
     */
    virtual auto PeerReplyComplete(
        const identifier::Nym& nym,
        const Identifier& replyOrRequest) const -> bool = 0;

    /**   Rollback a PeerReplyCreate call
     *
     *    The original request is returned to IncomingPeerRequest box
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the corresponding request
     *    \param[in] reply the identifier of the peer reply object
     *    \returns true if the rollback is successful
     */
    virtual auto PeerReplyCreateRollback(
        const identifier::Nym& nym,
        const Identifier& request,
        const Identifier& reply) const -> bool = 0;

    /**   Obtain a list of sent peer replies
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    virtual auto PeerReplySent(const identifier::Nym& nym) const
        -> ObjectList = 0;

    /**   Obtain a list of incoming peer replies
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    virtual auto PeerReplyIncoming(const identifier::Nym& nym) const
        -> ObjectList = 0;

    /**   Obtain a list of finished peer replies
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    virtual auto PeerReplyFinished(const identifier::Nym& nym) const
        -> ObjectList = 0;

    /**   Obtain a list of processed peer replies
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    virtual auto PeerReplyProcessed(const identifier::Nym& nym) const
        -> ObjectList = 0;

    /**   Store the senders's copy of a peer reply
     *
     *    The peer reply is stored in the IncomingPeerReply box for the
     *    specified nym.
     *
     *    The corresponding request is moved from the nym's SentPeerRequest
     *    box to the FinishedPeerRequest box.
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the corresponding request
     *    \param[in] reply the serialized peer reply object
     *    \returns true if the request is successfully stored
     */
    virtual auto PeerReplyReceive(
        const identifier::Nym& nym,
        const PeerObject& reply) const -> bool = 0;

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
        const Identifier& request,
        const StorageBox& box,
        std::time_t& time,
        AllocateOutput destination) const -> bool = 0;

    /**   Clean up the sender's copy of a peer reply
     *
     *    The peer reply is moved from the nym's IncomingPeerReply
     *    box to the ProcessedPeerReply box.
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] reply the identifier of the peer reply object
     *    \returns true if the request is successfully moved
     */
    virtual auto PeerRequestComplete(
        const identifier::Nym& nym,
        const Identifier& reply) const -> bool = 0;

    /**   Rollback a PeerRequestCreate call
     *
     *    The request is deleted from to SentPeerRequest box
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the peer request
     *    \returns true if the rollback is successful
     */
    virtual auto PeerRequestCreateRollback(
        const identifier::Nym& nym,
        const Identifier& request) const -> bool = 0;

    /**   Delete a peer reply object
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the peer reply object
     *    \param[in] box the box from which the peer object will be deleted
     */
    virtual auto PeerRequestDelete(
        const identifier::Nym& nym,
        const Identifier& request,
        const StorageBox& box) const -> bool = 0;

    /**   Obtain a list of sent peer requests
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    virtual auto PeerRequestSent(const identifier::Nym& nym) const
        -> ObjectList = 0;

    /**   Obtain a list of incoming peer requests
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    virtual auto PeerRequestIncoming(const identifier::Nym& nym) const
        -> ObjectList = 0;

    /**   Obtain a list of finished peer requests
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    virtual auto PeerRequestFinished(const identifier::Nym& nym) const
        -> ObjectList = 0;

    /**   Obtain a list of processed peer requests
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    virtual auto PeerRequestProcessed(const identifier::Nym& nym) const
        -> ObjectList = 0;

    /**   Store the recipient's copy of a peer request
     *
     *    The peer request is stored in the IncomingPeerRequest box for the
     *    specified nym.
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the serialized peer request object
     *    \returns true if the request is successfully stored
     */
    virtual auto PeerRequestReceive(
        const identifier::Nym& nym,
        const PeerObject& request) const -> bool = 0;

    /**   Update the timestamp of a peer request object
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the peer request object
     *    \param[in] box the box from which the peer object will be deleted
     */
    virtual auto PeerRequestUpdate(
        const identifier::Nym& nym,
        const Identifier& request,
        const StorageBox& box) const -> bool = 0;

    virtual auto Purse(
        const identifier::Nym& nym,
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        const bool checking = false) const -> const otx::blind::Purse& = 0;

    /**   Unload and delete a server contract
     *
     *    This method destroys the contract object, removes it from the
     *    in-memory map, and deletes it from local storage.
     *    \param[in]  id the indentifier of the contract to be removed
     *    \returns true if successful, false if the contract did not exist
     *
     */
    virtual auto RemoveServer(const identifier::Notary& id) const -> bool = 0;

    /**   Unload and delete a unit definition contract
     *
     *    This method destroys the contract object, removes it from the
     *    in-memory map, and deletes it from local storage.
     *    \param[in]  id the indentifier of the contract to be removed
     *    \returns true if successful, false if the contract did not exist
     *
     */
    virtual auto RemoveUnitDefinition(
        const identifier::UnitDefinition& id) const -> bool = 0;

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
     *    \param[in] nymid the identifier of nym which will create the contract
     *    \param[in] name the official name of the server
     *    \param[in] terms human-readable server description & terms of use
     *    \param[in] url externally-reachable IP address or hostname
     *    \param[in] port externally-reachable listen port
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

    /**   Returns a list of all available server contracts and their aliases
     */
    virtual auto ServerList() const -> ObjectList = 0;

    /**   Updates the alias for the specified nym.
     *
     *    An alias is a local label which is not part of the nym credentials
     *    itself.
     *
     *    \param[in] id the identifier of the nym whose alias is to be set
     *    \param[in] alias the alias to set or update for the specified nym
     *    \returns true if successful, false if the nym can not be located
     */
    virtual auto SetNymAlias(
        const identifier::Nym& id,
        const UnallocatedCString& alias) const -> bool = 0;

    /**   Updates the alias for the specified server contract.
     *
     *    An alias is a local label which is not part of the server contract
     *    itself.
     *
     *    \param[in] id the identifier of the contract whose alias is to be set
     *    \param[in] alias the alias to set or update for the specified contract
     *    \returns true if successful, false if the contract can not be located
     */
    virtual auto SetServerAlias(
        const identifier::Notary& id,
        const UnallocatedCString& alias) const -> bool = 0;

    /**   Updates the alias for the specified unit definition contract.
     *
     *    An alias is a local label which is not part of the unit definition
     *    contract itself.
     *
     *    \param[in] id the identifier of the contract whose alias is to be set
     *    \param[in] alias the alias to set or update for the specified contract
     *    \returns true if successful, false if the contract can not be located
     */
    virtual auto SetUnitDefinitionAlias(
        const identifier::UnitDefinition& id,
        const UnallocatedCString& alias) const -> bool = 0;

    /**   Obtain a list of all available unit definition contracts and their
     *    aliases
     */
    virtual auto UnitDefinitionList() const -> ObjectList = 0;

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
    virtual auto BasketContract(
        const identifier::UnitDefinition& id,
        const std::chrono::milliseconds& timeout = std::chrono::milliseconds(
            0)) const noexcept(false) -> OTBasketContract = 0;

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

    virtual auto CurrencyTypeBasedOnUnitType(
        const identifier::UnitDefinition& contractID) const -> UnitType = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const session::internal::Wallet& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> session::internal::Wallet& = 0;

    OPENTXS_NO_EXPORT virtual ~Wallet() = default;

protected:
    Wallet() = default;

private:
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;
};
}  // namespace opentxs::api::session

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/identity/IdentityType.hpp"

#pragma once

#include <chrono>
#include <cstdint>
#include <ctime>
#include <memory>
#include <tuple>

#include "opentxs/Export.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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

namespace otx
{
namespace blind
{
class Purse;
}  // namespace blind
}  // namespace otx

class NymData;
class PeerObject;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

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
    virtual auto AccountPartialMatch(const UnallocatedCString& hint) const
        -> identifier::Generic = 0;
    virtual auto DeleteAccount(const identifier::Generic& accountID) const
        -> bool = 0;
    virtual auto DefaultNym() const noexcept
        -> std::pair<identifier::Nym, std::size_t> = 0;
    virtual auto Internal() const noexcept
        -> const session::internal::Wallet& = 0;
    /**   Returns a list of all issuers associated with a local nym */
    virtual auto IssuerList(const identifier::Nym& nymID) const
        -> UnallocatedSet<identifier::Nym> = 0;
    virtual auto IsLocalNym(const std::string_view id) const -> bool = 0;
    virtual auto IsLocalNym(const identifier::Nym& id) const -> bool = 0;
    virtual auto LocalNymCount() const -> std::size_t = 0;
    virtual auto LocalNyms() const -> Set<identifier::Nym> = 0;
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
        const std::chrono::milliseconds& timeout = 0ms) const -> Nym_p = 0;
    /**   Instantiate a nym from serialized form
     *
     *    The smart pointer will not be initialized if the provided serialized
     *    contract is invalid.
     *
     *    \param[in] bytes the serialized version of the contract
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
    virtual auto NymByIDPartialMatch(const UnallocatedCString& partialId) const
        -> Nym_p = 0;
    /**   Returns a list of all known nyms and their aliases
     */
    virtual auto NymList() const -> ObjectList = 0;
    virtual auto NymNameByIndex(const std::size_t index, String& name) const
        -> bool = 0;
    /**   Load a peer reply object
     *
     *    \param[in] nym    the identifier of the nym who owns the object
     *    \param[in] reply  the identifier of the peer reply object
     *    \param[in] box    the box from which to retrive the peer object
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    virtual auto PeerReply(
        const identifier::Nym& nym,
        const identifier::Generic& reply,
        const otx::client::StorageBox& box,
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
        const identifier::Generic& replyOrRequest) const -> bool = 0;
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
        const identifier::Generic& request,
        const identifier::Generic& reply) const -> bool = 0;
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
     *    \param[in] nym    the identifier of the nym who owns the object
     *    \param[in] reply  the serialized peer reply object
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
        const identifier::Generic& request,
        const otx::client::StorageBox& box,
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
        const identifier::Generic& reply) const -> bool = 0;
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
        const identifier::Generic& request) const -> bool = 0;
    /**   Delete a peer reply object
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the peer reply object
     *    \param[in] box the box from which the peer object will be deleted
     */
    virtual auto PeerRequestDelete(
        const identifier::Nym& nym,
        const identifier::Generic& request,
        const otx::client::StorageBox& box) const -> bool = 0;
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
        const identifier::Generic& request,
        const otx::client::StorageBox& box) const -> bool = 0;
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
    /**   Returns a list of all available server contracts and their aliases
     */
    virtual auto ServerList() const -> ObjectList = 0;
    virtual auto SetDefaultNym(const identifier::Nym& id) const noexcept
        -> bool = 0;
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

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> session::internal::Wallet& = 0;

    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;

    OPENTXS_NO_EXPORT virtual ~Wallet() = default;

protected:
    Wallet() = default;
};
}  // namespace opentxs::api::session

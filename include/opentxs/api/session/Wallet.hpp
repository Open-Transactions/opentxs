// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <ctime>
#include <string_view>
#include <utility>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace internal
{
class Wallet;
}  // namespace internal

class Wallet;  // IWYU pragma: keep
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

namespace identifier
{
class Account;
class Generic;
class Notary;
class Nym;
class UnitDefinition;
}  // namespace identifier

namespace otx
{
namespace blind
{
class Purse;
}  // namespace blind
}  // namespace otx

class NymData;
class PasswordPrompt;
class PeerObject;
class String;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

/** \brief This class manages instantiated contracts. */
class OPENTXS_EXPORT opentxs::api::session::Wallet
{
public:
    auto AccountPartialMatch(const UnallocatedCString& hint) const
        -> identifier::Generic;
    auto DeleteAccount(const identifier::Account& accountID) const -> bool;
    auto DefaultNym() const noexcept -> std::pair<identifier::Nym, std::size_t>;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::Wallet&;
    /**   Returns a list of all issuers associated with a local nym */
    auto IssuerList(const identifier::Nym& nymID) const
        -> UnallocatedSet<identifier::Nym>;
    auto IsLocalNym(const std::string_view id) const -> bool;
    auto IsLocalNym(const identifier::Nym& id) const -> bool;
    auto LocalNymCount() const -> std::size_t;
    auto LocalNyms() const -> Set<identifier::Nym>;
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
    auto Nym(const identifier::Nym& id, std::chrono::milliseconds timeout) const
        -> Nym_p;
    auto Nym(const identifier::Nym& id) const -> Nym_p;
    /**   Instantiate a nym from serialized form
     *
     *    The smart pointer will not be initialized if the provided serialized
     *    contract is invalid.
     *
     *    \param[in] bytes the serialized version of the contract
     */
    auto Nym(const ReadView& bytes) const -> Nym_p;
    auto Nym(
        const identity::Type type,
        const PasswordPrompt& reason,
        const UnallocatedCString& name = {}) const -> Nym_p;
    auto Nym(
        const opentxs::crypto::Parameters& parameters,
        const PasswordPrompt& reason,
        const UnallocatedCString& name = {}) const -> Nym_p;
    auto Nym(const PasswordPrompt& reason, const UnallocatedCString& name = {})
        const -> Nym_p;
    auto Nym(
        const opentxs::crypto::Parameters& parameters,
        const identity::Type type,
        const PasswordPrompt& reason,
        const UnallocatedCString& name = {}) const -> Nym_p;
    auto mutable_Nym(const identifier::Nym& id, const PasswordPrompt& reason)
        const -> NymData;
    auto NymByIDPartialMatch(const UnallocatedCString& partialId) const
        -> Nym_p;
    /**   Returns a list of all known nyms and their aliases
     */
    auto NymList() const -> ObjectList;
    auto NymNameByIndex(const std::size_t index, String& name) const -> bool;
    /**   Load a peer reply object
     *
     *    \param[in] nym    the identifier of the nym who owns the object
     *    \param[in] reply  the identifier of the peer reply object
     *    \param[in] box    the box from which to retrieve the peer object
     */
    auto PeerReply(
        const identifier::Nym& nym,
        const identifier::Generic& reply,
        otx::client::StorageBox box,
        alloc::Strategy alloc = {}) const noexcept -> contract::peer::Reply;
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
    auto PeerReplyComplete(
        const identifier::Nym& nym,
        const identifier::Generic& replyOrRequest) const -> bool;
    /**   Rollback a PeerReplyCreate call
     *
     *    The original request is returned to IncomingPeerRequest box
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the corresponding request
     *    \param[in] reply the identifier of the peer reply object
     *    \returns true if the rollback is successful
     */
    auto PeerReplyCreateRollback(
        const identifier::Nym& nym,
        const identifier::Generic& request,
        const identifier::Generic& reply) const -> bool;
    /**   Obtain a list of sent peer replies
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    auto PeerReplySent(const identifier::Nym& nym) const -> ObjectList;
    /**   Obtain a list of incoming peer replies
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    auto PeerReplyIncoming(const identifier::Nym& nym) const -> ObjectList;
    /**   Obtain a list of finished peer replies
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    auto PeerReplyFinished(const identifier::Nym& nym) const -> ObjectList;
    /**   Obtain a list of processed peer replies
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    auto PeerReplyProcessed(const identifier::Nym& nym) const -> ObjectList;
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
    auto PeerReplyReceive(const identifier::Nym& nym, const PeerObject& reply)
        const -> bool;
    /**   Load a peer reply object
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the peer reply object
     *    \param[in] box the box from which to retrieve the peer object
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    auto PeerRequest(
        const identifier::Nym& nym,
        const identifier::Generic& request,
        const otx::client::StorageBox& box,
        alloc::Strategy alloc = {}) const noexcept -> contract::peer::Request;
    /**   Clean up the sender's copy of a peer reply
     *
     *    The peer reply is moved from the nym's IncomingPeerReply
     *    box to the ProcessedPeerReply box.
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] reply the identifier of the peer reply object
     *    \returns true if the request is successfully moved
     */
    auto PeerRequestComplete(
        const identifier::Nym& nym,
        const identifier::Generic& reply) const -> bool;
    /**   Rollback a PeerRequestCreate call
     *
     *    The request is deleted from to SentPeerRequest box
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the peer request
     *    \returns true if the rollback is successful
     */
    auto PeerRequestCreateRollback(
        const identifier::Nym& nym,
        const identifier::Generic& request) const -> bool;
    /**   Delete a peer reply object
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the peer reply object
     *    \param[in] box the box from which the peer object will be deleted
     */
    auto PeerRequestDelete(
        const identifier::Nym& nym,
        const identifier::Generic& request,
        const otx::client::StorageBox& box) const -> bool;
    /**   Obtain a list of sent peer requests
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    auto PeerRequestSent(const identifier::Nym& nym) const -> ObjectList;
    /**   Obtain a list of incoming peer requests
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    auto PeerRequestIncoming(const identifier::Nym& nym) const -> ObjectList;
    /**   Obtain a list of finished peer requests
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    auto PeerRequestFinished(const identifier::Nym& nym) const -> ObjectList;
    /**   Obtain a list of processed peer requests
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    auto PeerRequestProcessed(const identifier::Nym& nym) const -> ObjectList;
    /**   Store the recipient's copy of a peer request
     *
     *    The peer request is stored in the IncomingPeerRequest box for the
     *    specified nym.
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the serialized peer request object
     *    \returns true if the request is successfully stored
     */
    auto PeerRequestReceive(
        const identifier::Nym& nym,
        const PeerObject& request) const -> bool;
    /**   Update the timestamp of a peer request object
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the peer request object
     *    \param[in] box the box from which the peer object will be deleted
     */
    auto PeerRequestUpdate(
        const identifier::Nym& nym,
        const identifier::Generic& request,
        const otx::client::StorageBox& box) const -> bool;
    auto Purse(
        const identifier::Nym& nym,
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        const bool checking = false) const -> const otx::blind::Purse&;
    /**   Unload and delete a server contract
     *
     *    This method destroys the contract object, removes it from the
     *    in-memory map, and deletes it from local storage.
     *    \param[in]  id the indentifier of the contract to be removed
     *    \returns true if successful, false if the contract did not exist
     *
     */
    auto RemoveServer(const identifier::Notary& id) const -> bool;
    /**   Unload and delete a unit definition contract
     *
     *    This method destroys the contract object, removes it from the
     *    in-memory map, and deletes it from local storage.
     *    \param[in]  id the indentifier of the contract to be removed
     *    \returns true if successful, false if the contract did not exist
     *
     */
    auto RemoveUnitDefinition(const identifier::UnitDefinition& id) const
        -> bool;
    /**   Returns a list of all available server contracts and their aliases
     */
    auto ServerList() const -> ObjectList;
    auto SetDefaultNym(const identifier::Nym& id) const noexcept -> bool;
    /**   Updates the alias for the specified nym.
     *
     *    An alias is a local label which is not part of the nym credentials
     *    itself.
     *
     *    \param[in] id the identifier of the nym whose alias is to be set
     *    \param[in] alias the alias to set or update for the specified nym
     *    \returns true if successful, false if the nym can not be located
     */
    auto SetNymAlias(const identifier::Nym& id, std::string_view alias) const
        -> bool;
    /**   Updates the alias for the specified server contract.
     *
     *    An alias is a local label which is not part of the server contract
     *    itself.
     *
     *    \param[in] id the identifier of the contract whose alias is to be set
     *    \param[in] alias the alias to set or update for the specified contract
     *    \returns true if successful, false if the contract can not be located
     */
    auto SetServerAlias(const identifier::Notary& id, std::string_view alias)
        const -> bool;
    /**   Updates the alias for the specified unit definition contract.
     *
     *    An alias is a local label which is not part of the unit definition
     *    contract itself.
     *
     *    \param[in] id the identifier of the contract whose alias is to be set
     *    \param[in] alias the alias to set or update for the specified contract
     *    \returns true if successful, false if the contract can not be located
     */
    auto SetUnitDefinitionAlias(
        const identifier::UnitDefinition& id,
        std::string_view alias) const -> bool;

    /**   Obtain a list of all available unit definition contracts and their
     *    aliases
     */
    auto UnitDefinitionList() const -> ObjectList;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Wallet&;

    OPENTXS_NO_EXPORT Wallet(internal::Wallet* imp) noexcept;
    Wallet() = delete;
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;

    OPENTXS_NO_EXPORT virtual ~Wallet();

private:
    friend internal::Wallet;

    internal::Wallet* imp_;
};

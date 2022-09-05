// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string_view>

#include "opentxs/blockchain/Types.hpp"

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
class Endpoints;
}  // namespace internal
}  // namespace session
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session
{
class OPENTXS_EXPORT Endpoints
{
public:
    /** Account balance update notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  AccountUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for all session types.
     */
    virtual auto AccountUpdate() const noexcept -> std::string_view = 0;

    /** Blockchain account creation notification
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainAccountCreated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto BlockchainAccountCreated() const noexcept
        -> std::string_view = 0;

    /** Blockchain balance notifications
     *
     *  A dealer socket can connect to this endpoint to send and receive
     *  BlockchainBalance tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto BlockchainBalance() const noexcept -> std::string_view = 0;

    /** Blockchain block available notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainBlockAvailable tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto BlockchainBlockAvailable() const noexcept
        -> std::string_view = 0;

    /** Blockchain block download queue notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainBlockDownloadQueue tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto BlockchainBlockDownloadQueue() const noexcept
        -> std::string_view = 0;

    /** Blockchain block oracle update notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainBlockOracleProgress tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto BlockchainBlockOracleProgress() const noexcept
        -> std::string_view = 0;

    /** Blockchain mempool updates
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainMempoolUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto BlockchainMempool() const noexcept -> std::string_view = 0;

    /** Blockchain filter oracle notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainNewFilter tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto BlockchainNewFilter() const noexcept -> std::string_view = 0;

    /** Blockchain peer connection ready
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainPeerAdded tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto BlockchainPeer() const noexcept -> std::string_view = 0;

    /** Blockchain peer connection initiated or lost
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainPeerConnected tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto BlockchainPeerConnection() const noexcept
        -> std::string_view = 0;

    /** Blockchain reorg and update notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainNewHeader and BlockchainReorg tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto BlockchainReorg() const noexcept -> std::string_view = 0;

    /** Blockchain wallet scan progress
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainWalletScanProgress tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto BlockchainScanProgress() const noexcept
        -> std::string_view = 0;

    /** Blockchain enabled state change
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainStateChange tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto BlockchainStateChange() const noexcept -> std::string_view = 0;

    /** Blockchain wallet sync progress
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainSyncProgress tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto BlockchainSyncProgress() const noexcept
        -> std::string_view = 0;

    /** Blockchain sync server indexing progress
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainSyncServerProgress tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto BlockchainSyncServerProgress() const noexcept
        -> std::string_view = 0;

    /** Blockchain sync server database changes
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  SyncServerUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto BlockchainSyncServerUpdated() const noexcept
        -> std::string_view = 0;

    /** Blockchain transaction notifications (global)
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainNewTransaction tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto BlockchainTransactions() const noexcept
        -> std::string_view = 0;

    /** Blockchain transaction notifications (per-nym)
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainNewTransaction tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto BlockchainTransactions(
        const identifier::Nym& nym) const noexcept -> std::string_view = 0;

    /** Blockchain wallet balance updates
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainWalletUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto BlockchainWalletUpdated() const noexcept
        -> std::string_view = 0;

    /** Connection state notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  OTXConnectionStatus tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto ConnectionStatus() const noexcept -> std::string_view = 0;

    /** Contact account creation notification
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  ContactUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto ContactUpdate() const noexcept -> std::string_view = 0;

    /** Search for a nym on known notaries
     *
     *  A push socket can connect to this endpoint to send
     *  OTXSearchNym tagged messages.
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto FindNym() const noexcept -> std::string_view = 0;

    /** Search for a notary contract on known notaries
     *
     *  A push socket can connect to this endpoint to send
     *  OTXSearchServer tagged messages.
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto FindServer() const noexcept -> std::string_view = 0;

    /** Search for a unit definition on known notaries
     *
     *  A push socket can connect to this endpoint to send
     *  OTXSearchUnit tagged messages.
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto FindUnitDefinition() const noexcept -> std::string_view = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const session::internal::Endpoints& = 0;

    /** Issuer update notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  IssuerUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto IssuerUpdate() const noexcept -> std::string_view = 0;

    /** Contact messagability status
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  OTXMessagability tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto Messagability() const noexcept -> std::string_view = 0;

    /** Message loaded
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  MessageLoaded tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto MessageLoaded() const noexcept -> std::string_view = 0;

    /** Nym created notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  NymCreated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for all session types.
     */
    virtual auto NymCreated() const noexcept -> std::string_view = 0;

    /** Nym update notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  NymUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for all session types.
     */
    virtual auto NymDownload() const noexcept -> std::string_view = 0;

    /** Node pairing event notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any peer message related to node pairing is received.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains a serialized proto::PairEvent message
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto PairEvent() const noexcept -> std::string_view = 0;

    /** Peer reply event notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any peer reply is received.
     *
     *  Messages bodies consist of two frame.
     *   * The first frame contains the recipient nym as a serialized string
     *   * The second frame contains a serialized proto::PeerReply message
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto PeerReplyUpdate() const noexcept -> std::string_view = 0;

    /** Peer request event notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any peer request is received.
     *
     *  Messages bodies consist of one frame.
     *   * The first frame contains the recipient nym as a serialized string
     *   * The second frame contains a serialized proto::PeerRequest message
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto PeerRequestUpdate() const noexcept -> std::string_view = 0;

    /** Pending bailment notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  a pending bailment peer request has been received.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains a serialized proto::PeerRequest message
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto PendingBailment() const noexcept -> std::string_view = 0;

    /** HD seed update notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  SeedUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for all session types.
     */
    virtual auto SeedUpdated() const noexcept -> std::string_view = 0;

    /** Server reply notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any server reply is received.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains of the message type as a string
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto ServerReplyReceived() const noexcept -> std::string_view = 0;

    /** Server request notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any request message is sent to a notary.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains of the message type as a string
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto ServerRequestSent() const noexcept -> std::string_view = 0;

    /** Server contract update notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  NotaryUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for all session types.
     */
    virtual auto ServerUpdate() const noexcept -> std::string_view = 0;

    /** Notification of context shutdown
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  Shutdown tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for all session types.
     */
    virtual auto Shutdown() const noexcept -> std::string_view = 0;

    /** Background task completion notification
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  OTXTaskComplete tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto TaskComplete() const noexcept -> std::string_view = 0;

    /** Activity thread update notification
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  ActivityThreadUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto ThreadUpdate(const std::string_view thread) const noexcept
        -> std::string_view = 0;

    /** Unit definition contract update notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  UnitDefinitionUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for all session types.
     */
    virtual auto UnitUpdate() const noexcept -> std::string_view = 0;

    /** UI widget update notification
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  UIModelUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto WidgetUpdate() const noexcept -> std::string_view = 0;

    /** Account update notification
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  WorkflowAccountUpdate tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    virtual auto WorkflowAccountUpdate() const noexcept -> std::string_view = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> session::internal::Endpoints& = 0;

    Endpoints(const Endpoints&) = delete;
    Endpoints(Endpoints&&) = delete;
    auto operator=(const Endpoints&) -> Endpoints& = delete;
    auto operator=(Endpoints&&) -> Endpoints& = delete;

    OPENTXS_NO_EXPORT virtual ~Endpoints() = default;

protected:
    Endpoints() = default;
};
}  // namespace opentxs::api::session

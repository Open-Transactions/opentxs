// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string_view>

namespace opentxs
{
using OTZMQWorkType = std::uint16_t;

// NOTE 16384 - 32767 are reserved for client applications
enum class WorkType : OTZMQWorkType {
    Shutdown = 0,
    NymCreated = 1,
    NymUpdated = 2,
    NotaryUpdated = 3,
    UnitDefinitionUpdated = 4,
    ContactUpdated = 5,
    AccountUpdated = 6,
    IssuerUpdated = 7,
    ActivityThreadUpdated = 8,
    UIModelUpdated = 9,
    WorkflowAccountUpdate = 10,
    MessageLoaded = 11,
    SeedUpdated = 12,
    BlockchainAccountCreated = 128,
    BlockchainBalance = 129,
    BlockchainNewHeader = 130,
    BlockchainNewTransaction = 131,
    BlockchainPeerAdded = 132,
    BlockchainReorg = 133,
    BlockchainStateChange = 134,
    BlockchainSyncProgress = 135,
    BlockchainWalletScanProgress = 136,
    BlockchainNewFilter = 137,
    BlockchainBlockDownloadQueue = 138,
    BlockchainPeerConnected = 139,
    BlockchainWalletUpdated = 140,
    SyncServerUpdated = 141,
    BlockchainMempoolUpdated = 142,
    BlockchainBlockAvailable = 143,
    BlockchainSyncServerProgress = 144,
    BlockchainBlockOracleProgress = 145,
    OTXConnectionStatus = 256,
    OTXTaskComplete = 257,
    OTXSearchNym = 258,
    OTXSearchServer = 259,
    OTXSearchUnit = 260,
    OTXMessagability = 261,
    reserved1 = 1021,
    reserved2 = 1022,
    reserved3 = 1023,
    P2PBlockchainSyncRequest = 1024,
    P2PBlockchainSyncAck = 1025,
    P2PBlockchainSyncReply = 1026,
    P2PBlockchainNewBlock = 1027,
    P2PBlockchainSyncQuery = 1028,
    P2PResponse = 1029,
    P2PPublishContract = 1030,
    P2PQueryContract = 1031,
    P2PPushTransaction = 1032,
    AsioRegister = 2048,
    AsioConnect = 2049,
    AsioDisconnect = 2050,
    AsioSendResult = 2051,
    AsioResolve = 2052,
    BlockchainOverZeroMQ = 3072,
    OTXRequest = 4096,
    OTXResponse = 4097,
    OTXPush = 4098,
    OTXLegacyXML = 4099,
    PeerRequest = 5120,
    PeerReply = 5121,
};  // IWYU pragma: export

constexpr auto value(const WorkType in) noexcept
{
    return static_cast<OTZMQWorkType>(in);
}

auto print(OTZMQWorkType in) noexcept -> std::string_view;

/*** Tagged Status Message Format
 *
 *   Messages using this taxonomy will always have the first body frame set to a
 *   WorkType value
 *
 *   Depending on the message type additional body frames may be present as
 *   described below
 *
 *   Shutdown: reports the pending shutdown of a client session or server
 *             session
 *
 *   NymCreated: reports the id of newly generated nyms
 *       * Additional frames:
 *          1: id as identifier::Nym (encoded as byte sequence)
 *
 *   NymUpdated: reports that (new or existing) nym has been modified
 *       * Additional frames:
 *          1: nym id as identifier::Nym (encoded as byte sequence)
 *
 *   NotaryUpdated: reports that a notary contract has been modified
 *       * Additional frames:
 *          1: id as identifier::Notary (encoded as byte sequence)
 *
 *   UnitDefinitionUpdated: reports that a unit definition contract has been
 *                          modified
 *       * Additional frames:
 *          1: id as identifier::UnitDefinition (encoded as byte sequence)
 *
 *   ContactUpdated: reports that (new or existing) contact has been updated
 *       * Additional frames:
 *          1: contact id as Identifier (encoded as byte sequence)
 *
 *   AccountUpdated: reports that a custodial account has been modified
 *       * Additional frames:
 *          1: account id (decode with api::Factory::AccountIDFromZMQ)
 *          2: account balance as Amount
 *
 *   IssuerUpdated: reports that an issuer has been updated
 *       * Additional frames:
 *          1: local nym id as identifier::Nym (encoded as byte sequence)
 *          1: issuer id as identifier::Nym (encoded as byte sequence)
 *
 *   ActivityThreadUpdated: reports that an activity thread has been updated
 *       * Additional frames:
 *          1: thread id as Identifier (encoded as byte sequence)
 *
 *   UIModelUpdated: reports that a ui model has changed
 *       * Additional frames:
 *          1: widget id as Identifier (encoded as byte sequence)
 *
 *   WorkflowAccountUpdate: reports that a workflow has been modified
 *       * Additional frames:
 *          1: account id (decode with api::Factory::AccountIDFromZMQ)
 *
 *   MessageLoaded: report that background decryption of a message is complete
 *       * Additional frames:
 *          1: recipient nym id as identifier::Nym (encoded as byte sequence)
 *          2: message id as Identifier (encoded as byte sequence)
 *          3: message type as StorageBox
 *          4: decrypted text as string
 *
 *   SeedUpdated: reports that an HD seed has been created or modified
 *       * Additional frames:
 *          1: seed id as Identifier (encoded as byte sequence)
 *
 *   BlockchainAccountCreated: reports the creation of a new blockchain account
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: account owner as identifier::Nym (encoded as byte sequence)
 *          3: account type as api::crypto::Blockchain::AccountType
 *          4: account id (decode with api::Factory::AccountIDFromZMQ)
 *
 *   BlockchainBalance: request and response messages for blockchain balance
 *                      information
 *       * Request message additional frames:
 *          1: chain type as blockchain::Type
 *          2: [optional] target nym as identifier::Nym (encoded as byte
 *                        sequence)
 *
 *       * Response message additional frames:
 *          1: chain type as blockchain::Type
 *          2: confirmed balance as Amount
 *          3: unconfirmed balance as Amount
 *          4: [optional] designated nym as identifier::Nym (encoded as byte
 *                        sequence)
 *
 *   BlockchainNewHeader: reports the receipt of a new blockchain header
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: new block hash (encoded as byte sequence)
 *          3: height of the new block as blockchain::block::Height
 *
 *   BlockchainNewTransaction: reports the receipt of a new blockchain
 *                             transaction
 *       * Additional frames:
 *          1: txid (encoded as byte sequence)
 *          2: chain type as blockchain::Type
 *
 *   BlockchainPeerAdded: reports when a new peer has reached the active state
 *                        for any active blockchain
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: peer address as string
 *          3: active peer count as std::size_t
 *
 *   BlockchainReorg: reports the receipt of a new blockchain header which
 *                    reorganizes the chain
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: ancestor block hash (encoded as byte sequence)
 *          3: height of the ancestor block as blockchain::block::Height
 *          4: new block hash (encoded as byte sequence)
 *          5: height of the new block as blockchain::block::Height
 *
 *   BlockchainStateChange: reports changes to the enabled state of a
 *                          blockchain
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: state as bool (enabled = true)
 *
 *   BlockchainSyncProgress: reports the cfilter sync progress
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: current progress as blockchain::block::Height
 *          3: target height as blockchain::block::Height
 *
 *   BlockchainWalletScanProgress: reports the blockchain wallet scan progress
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: account owner as identifier::Nym (encoded as byte sequence)
 *          3: account type as api::crypto::Blockchain::AccountType
 *          4: account id (decode with api::Factory::AccountIDFromZMQ)
 *          5: subchain type as blockchain::crypto::Subchain
 *          6: last scan height as blockchain::block::Height
 *          7: last scan hash as blockchain::block::Hash (encoded as byte
 *             sequence)
 *
 *   BlockchainNewFilter: reports the receipt of a new cfilter
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: filter type as blockchain::cfilter::Type
 *          3: corresponding height as blockchain::block::Height
 *          4: corresponding block hash as blockchain::block::Hash (encoded as
 *             byte sequence)
 *
 *   BlockchainBlockDownloadQueue: reports change to the state of the block
 *                                 download queue
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: queue size as std::size_t
 *
 *   BlockchainPeerConnected: reports when the number of open incoming or
 *                            outgoing peer connections has changed
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: peer count as std::size_t
 *
 *   BlockchainWalletUpdated: reports the blockchain wallet balance updates
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: confirmed balance as Amount
 *          3: unconfirmed balance as Amount
 *          4: [optional] account owner as identifier::Nym (encoded as byte
 *                        sequence)
 *
 *   SyncServerUpdated: reports that a blockchain sync server endpoint has been
 *                      added or removed from the database
 *          1: endpoint as a string
 *          2: operation as bool (added = true, removed = false)
 *
 *   BlockchainMempoolUpdated: reports a new blockchain transaction has entered
 *                             the mempool
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: txid as blockchain::block::Hash (encoded as byte sequence)
 *
 *   BlockchainBlockAvailable: reports a blockchain transaction which was
 *                             previously pending download has become available
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: block hash (encoded as byte sequence)
 *
 *   BlockchainSyncServerProgress: reports that a sync indexer has added a new
 *                                 block to its database
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: corresponding height as blockchain::block::Height
 *          3: corresponding block hash as blockchain::block::Hash (encoded as
 *             byte sequence)
 *
 *   BlockchainBlockOracleProgress: reports that a block oracle has downloaded a
 *                                  new block
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: corresponding height as blockchain::block::Height
 *          3: corresponding block hash as blockchain::block::Hash (encoded as
 *             byte sequence)
 *
 *   OTXConnectionStatus: reports state changes to notary connections
 *       * Additional frames:
 *          1: notary id as identifier::Notary (encoded as byte sequence)
 *          2: state as bool (active = true)
 *
 *   OTXTaskComplete: reports completion of OTX task
 *       * Additional frames:
 *          1: task is as api::session::OTX::TaskID
 *          2: resolution as bool (success = true)
 *
 *   OTXSearchNym: request messages for OTX nym search
 *       * Additional frames:
 *          1: target id as identifier::Nym (encoded as byte sequence)
 *
 *   OTXSearchServer: request messages for OTX notary contract search
 *       * Additional frames:
 *          1: target id as identifier::Notary (encoded as byte sequence)
 *
 *   OTXSearchUnit: request messages for OTX unit definition contract search
 *       * Additional frames:
 *          1: target id as identifier::UnitDefinition (encoded as byte
 *             sequence)
 *
 *   OTXMessagability: status reports regarding the ability of a local nym to
 *                     message a remote contact via a notary
 *       * Additional frames:
 *          1: local nym id as identifier::Nym (encoded as byte sequence)
 *          1: remote contact id as Identifier (encoded as byte sequence)
 *          2: status as opentxs::Messagability
 *
 *   AsioRegister: request and response messages for asio wrapper backend router
 *                 socket
 *       * Request message additional frames:
 *          none
 *
 *       * Response message additional frames:
 *          1: requestor's connection id from the perspective of the backend
 *             (encoded as byte sequence)
 *
 *   AsioConnect: reports a successful connection to a remote endpoint
 *       * Additional frames:
 *          1: the remote endpoint (encoded as ascii)
 *
 *   AsioDisconnect: reports a connection or receiving error
 *       * Additional frames:
 *          1: the remote endpoint (encoded as ascii)
 *
 *   AsioSendResult: reports outcome of boost::asio::async_write
 *       * Additional frames:
 *          1: number of bytes written as std::size_t
 *          2: send success value as std::byte. 0x00 indicates an error
 *          3: [optional] error string
 *
 *   PeerRequest: OTX structured peer request message
 *       * Additional frames:
 *          1: request id as identifier::Generic (encoded as protobuf)
 *          2: recipient id as identifier::Nym (encoded as protobuf)
 *          3: sender id as identifier::Nym (encoded as protobuf)
 *          4: type as contract::peer::PeerRequestType
 *          5: payload encoded as protobuf
 *
 *   PeerReply: OTX structured peer reply message
 *       * Additional frames:
 *          1: request id as identifier::Generic (encoded as protobuf)
 *          2: recipient id as identifier::Nym (encoded as protobuf)
 *          3: sender id as identifier::Nym (encoded as protobuf)
 *          4: type as contract::peer::PeerRequestType
 *          5: payload encoded as protobuf
 */
}  // namespace opentxs

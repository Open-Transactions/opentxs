// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/core/contract/ContractType.hpp"

#pragma once

#include <memory>
#include <string_view>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/contract/Types.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WorkType.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Transaction;
}  // namespace block
}  // namespace bitcoin
}  // namespace blockchain

namespace contract
{
class Server;
class Unit;
}  // namespace contract

namespace identifier
{
class Generic;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace network
{
namespace otdht
{
class Acknowledgement;
class Base;
class Data;
class PublishContract;
class PublishContractReply;
class PushTransaction;
class PushTransactionReply;
class Query;
class QueryContract;
class QueryContractReply;
class Request;
class State;
}  // namespace otdht

namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto BlockchainSyncAcknowledgement() noexcept
    -> network::otdht::Acknowledgement;
auto BlockchainSyncAcknowledgement(
    network::otdht::StateData in,
    std::string_view endpoint) noexcept -> network::otdht::Acknowledgement;
auto BlockchainSyncAcknowledgement_p(
    network::otdht::StateData in,
    std::string_view endpoint) noexcept
    -> std::unique_ptr<network::otdht::Acknowledgement>;
auto BlockchainSyncData() noexcept -> network::otdht::Data;
auto BlockchainSyncData(
    WorkType type,
    network::otdht::State state,
    network::otdht::SyncData blocks,
    ReadView cfheader) noexcept -> network::otdht::Data;
auto BlockchainSyncData_p(
    WorkType type,
    network::otdht::State state,
    network::otdht::SyncData blocks,
    ReadView cfheader) noexcept -> std::unique_ptr<network::otdht::Data>;
auto BlockchainSyncMessage(
    const api::Session& api,
    const network::zeromq::Message& in) noexcept
    -> std::unique_ptr<network::otdht::Base>;
auto BlockchainSyncPublishContract() noexcept
    -> network::otdht::PublishContract;
auto BlockchainSyncPublishContract(const identity::Nym& payload) noexcept
    -> network::otdht::PublishContract;
auto BlockchainSyncPublishContract(const contract::Server& payload) noexcept
    -> network::otdht::PublishContract;
auto BlockchainSyncPublishContract(const contract::Unit& payload) noexcept
    -> network::otdht::PublishContract;
auto BlockchainSyncPublishContract_p(
    const api::Session& api,
    const contract::Type type,
    const ReadView id,
    const ReadView payload) noexcept
    -> std::unique_ptr<network::otdht::PublishContract>;
auto BlockchainSyncPublishContractReply() noexcept
    -> network::otdht::PublishContractReply;
auto BlockchainSyncPublishContractReply(
    const identifier::Generic& id,
    const bool success) noexcept -> network::otdht::PublishContractReply;
auto BlockchainSyncPublishContractReply_p(
    const api::Session& api,
    const ReadView id,
    const ReadView success) noexcept
    -> std::unique_ptr<network::otdht::PublishContractReply>;
auto BlockchainSyncPushTransaction() noexcept
    -> network::otdht::PushTransaction;
auto BlockchainSyncPushTransaction(
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::bitcoin::block::Transaction& payload) noexcept
    -> network::otdht::PushTransaction;
auto BlockchainSyncPushTransaction_p(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    const ReadView id,
    const ReadView payload) noexcept
    -> std::unique_ptr<network::otdht::PushTransaction>;
auto BlockchainSyncPushTransactionReply() noexcept
    -> network::otdht::PushTransactionReply;
auto BlockchainSyncPushTransactionReply(
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::block::Txid& id,
    const bool success) noexcept -> network::otdht::PushTransactionReply;
auto BlockchainSyncPushTransactionReply_p(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    const ReadView id,
    const ReadView success) noexcept
    -> std::unique_ptr<network::otdht::PushTransactionReply>;
auto BlockchainSyncQuery() noexcept -> network::otdht::Query;
auto BlockchainSyncQuery(int) noexcept -> network::otdht::Query;
auto BlockchainSyncQuery_p(int) noexcept
    -> std::unique_ptr<network::otdht::Query>;
auto BlockchainSyncQueryContract() noexcept -> network::otdht::QueryContract;
auto BlockchainSyncQueryContract(const identifier::Generic& id) noexcept
    -> network::otdht::QueryContract;
auto BlockchainSyncQueryContract_p(
    const api::Session& api,
    const ReadView id) noexcept
    -> std::unique_ptr<network::otdht::QueryContract>;
auto BlockchainSyncQueryContractReply() noexcept
    -> network::otdht::QueryContractReply;
auto BlockchainSyncQueryContractReply(const identifier::Generic& id) noexcept
    -> network::otdht::QueryContractReply;
auto BlockchainSyncQueryContractReply(const identity::Nym& payload) noexcept
    -> network::otdht::QueryContractReply;
auto BlockchainSyncQueryContractReply(const contract::Server& payload) noexcept
    -> network::otdht::QueryContractReply;
auto BlockchainSyncQueryContractReply(const contract::Unit& payload) noexcept
    -> network::otdht::QueryContractReply;
auto BlockchainSyncQueryContractReply_p(
    const api::Session& api,
    const contract::Type type,
    const ReadView id,
    const ReadView payload) noexcept
    -> std::unique_ptr<network::otdht::QueryContractReply>;
auto BlockchainSyncRequest() noexcept -> network::otdht::Request;
auto BlockchainSyncRequest(network::otdht::StateData in) noexcept
    -> network::otdht::Request;
auto BlockchainSyncRequest_p(network::otdht::StateData in) noexcept
    -> std::unique_ptr<network::otdht::Request>;
}  // namespace opentxs::factory
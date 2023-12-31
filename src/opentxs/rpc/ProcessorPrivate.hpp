// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/protobuf/RPCEnums.pb.h>
#include <opentxs/protobuf/RPCResponse.pb.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <tuple>

#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/Pull.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/util/Lockable.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/rpc/Processor.internal.hpp"
#include "opentxs/rpc/Types.hpp"
#include "opentxs/rpc/request/Message.hpp"
#include "opentxs/rpc/response/Message.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
class Notary;
}  // namespace session

class Context;
class Session;
}  // namespace api

namespace identifier
{
class Notary;
class UnitDefinition;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

namespace protobuf
{
class APIArgument;
class RPCCommand;
class TaskComplete;
}  // namespace protobuf

namespace rpc
{
namespace request
{
class SendPayment;
}  // namespace request

class AccountData;
}  // namespace rpc

class Options;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace zmq = opentxs::network::zeromq;

namespace opentxs::rpc
{
class ProcessorPrivate final : virtual public Processor, Lockable
{
public:
    auto Process(const protobuf::RPCCommand& command) const noexcept
        -> protobuf::RPCResponse final;
    auto Process(const request::Message& command) const noexcept
        -> std::unique_ptr<response::Message> final;

    ProcessorPrivate(const api::Context& native);
    ProcessorPrivate() = delete;
    ProcessorPrivate(const ProcessorPrivate&) = delete;
    ProcessorPrivate(ProcessorPrivate&&) = delete;
    auto operator=(const ProcessorPrivate&) -> ProcessorPrivate& = delete;
    auto operator=(ProcessorPrivate&&) -> ProcessorPrivate& = delete;

    ~ProcessorPrivate() final;

private:
    using Args = const ::google::protobuf::RepeatedPtrField<
        ::opentxs::protobuf::APIArgument>;
    using TaskID = UnallocatedCString;
    using Future = api::session::OTX::Future;
    using Result = api::session::OTX::Result;
    using Finish = std::function<
        void(const Result& result, protobuf::TaskComplete& output)>;
    using TaskData = std::tuple<Future, Finish, identifier::Nym>;

    const api::Context& ot_;
    mutable std::mutex task_lock_;
    mutable UnallocatedMap<TaskID, TaskData> queued_tasks_;
    const OTZMQListenCallback task_callback_;
    const OTZMQListenCallback push_callback_;
    const OTZMQPullSocket push_receiver_;
    const OTZMQPublishSocket rpc_publisher_;
    const OTZMQSubscribeSocket task_subscriber_;

    static void add_output_status(
        protobuf::RPCResponse& output,
        protobuf::RPCResponseCode code);
    static void add_output_status(
        protobuf::TaskComplete& output,
        protobuf::RPCResponseCode code);
    static void add_output_identifier(
        const UnallocatedCString& id,
        protobuf::RPCResponse& output);
    static void add_output_identifier(
        const UnallocatedCString& id,
        protobuf::TaskComplete& output);
    static void add_output_task(
        protobuf::RPCResponse& output,
        const UnallocatedCString& taskid);
    static auto get_account_event_type(
        otx::client::StorageBox storagebox,
        Amount amount) noexcept -> rpc::AccountEventType;
    static auto get_args(const Args& serialized) -> Options;
    static auto get_index(std::int32_t instance) -> std::size_t;
    static auto init(const protobuf::RPCCommand& command)
        -> protobuf::RPCResponse;
    static auto invalid_command(const protobuf::RPCCommand& command)
        -> protobuf::RPCResponse;

    auto accept_pending_payments(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto add_claim(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto add_contact(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto client_session(const request::Message& command) const noexcept(false)
        -> const api::session::Client&;
    auto create_account(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto create_compatible_account(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto create_issuer_account(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto create_nym(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto create_unit_definition(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto delete_claim(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    void evaluate_deposit_payment(
        const api::session::Client& client,
        const api::session::OTX::Result& result,
        protobuf::TaskComplete& output) const;
    void evaluate_move_funds(
        const api::session::Client& client,
        const api::session::OTX::Result& result,
        protobuf::RPCResponse& output) const;
    template <typename T>
    void evaluate_register_account(
        const api::session::OTX::Result& result,
        T& output) const;
    template <typename T>
    void evaluate_register_nym(
        const api::session::OTX::Result& result,
        T& output) const;
    auto evaluate_send_payment_cheque(
        const api::session::OTX::Result& result,
        protobuf::TaskComplete& output) const noexcept -> void;
    auto evaluate_send_payment_transfer(
        const api::session::Client& api,
        const api::session::OTX::Result& result,
        protobuf::TaskComplete& output) const noexcept -> void;
    auto evaluate_transaction_reply(
        const api::session::Client& api,
        const Message& reply) const noexcept -> bool;
    template <typename T>
    void evaluate_transaction_reply(
        const api::session::Client& client,
        const Message& reply,
        T& output,
        const protobuf::RPCResponseCode code =
            protobuf::RPCRESPONSE_TRANSACTION_FAILED) const;
    auto get_client(std::int32_t instance) const -> const api::session::Client*;
    auto get_account_activity(const request::Message& command) const
        -> std::unique_ptr<response::Message>;
    auto get_account_balance(const request::Message& command) const noexcept
        -> std::unique_ptr<response::Message>;
    auto get_account_balance_blockchain(
        const request::Message& base,
        const std::size_t index,
        const identifier::Account& accountID,
        UnallocatedVector<AccountData>& balances,
        response::Message::Responses& codes) const noexcept -> void;
    auto get_account_balance_custodial(
        const api::Session& api,
        const std::size_t index,
        const identifier::Account& accountID,
        UnallocatedVector<AccountData>& balances,
        response::Message::Responses& codes) const noexcept -> void;
    auto get_compatible_accounts(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto get_nyms(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto get_pending_payments(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto get_seeds(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto get_server(std::int32_t instance) const -> const api::session::Notary*;
    auto get_server_admin_nym(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto get_server_contracts(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto get_server_password(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto get_session(std::int32_t instance) const -> const api::Session&;
    auto get_transaction_data(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto get_unit_definitions(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto get_workflow(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto immediate_create_account(
        const api::session::Client& client,
        const identifier::Nym& owner,
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit) const -> bool;
    auto immediate_register_issuer_account(
        const api::session::Client& client,
        const identifier::Nym& owner,
        const identifier::Notary& notary) const -> bool;
    auto immediate_register_nym(
        const api::session::Client& client,
        const identifier::Notary& notary) const -> bool;
    auto import_seed(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto import_server_contract(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto is_blockchain_account(
        const request::Message& base,
        const identifier::Account& id) const noexcept -> bool;
    auto is_client_session(std::int32_t instance) const -> bool;
    auto is_server_session(std::int32_t instance) const -> bool;
    auto is_session_valid(std::int32_t instance) const -> bool;
    auto list_accounts(const request::Message& command) const noexcept
        -> std::unique_ptr<response::Message>;
    auto list_contacts(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto list_client_sessions(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto list_seeds(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto list_nyms(const request::Message& command) const noexcept
        -> std::unique_ptr<response::Message>;
    auto list_server_contracts(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto list_server_sessions(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto list_unit_definitions(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto lookup_account_id(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto move_funds(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    [[deprecated]] auto queue_task(
        const identifier::Nym& nymID,
        const UnallocatedCString taskID,
        Finish&& finish,
        Future&& future,
        protobuf::RPCResponse& output) const -> void;
    auto queue_task(
        const api::Session& api,
        const identifier::Nym& nymID,
        const UnallocatedCString taskID,
        Finish&& finish,
        Future&& future) const noexcept -> UnallocatedCString;
    auto register_nym(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto rename_account(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto send_payment(const request::Message& command) const noexcept
        -> std::unique_ptr<response::Message>;
    auto send_payment_blockchain(
        const api::session::Client& api,
        const request::SendPayment& command) const noexcept
        -> std::unique_ptr<response::Message>;
    auto send_payment_custodial(
        const api::session::Client& api,
        const request::SendPayment& command) const noexcept
        -> std::unique_ptr<response::Message>;
    auto session(const request::Message& command) const noexcept(false)
        -> const api::Session&;
    auto start_client(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto start_server(const protobuf::RPCCommand& command) const
        -> protobuf::RPCResponse;
    auto status(const response::Message::Identifiers& ids) const noexcept
        -> ResponseCode;

    void task_handler(const zmq::Message& message);
};
}  // namespace opentxs::rpc

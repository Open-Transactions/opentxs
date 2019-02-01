// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/client/Pair.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/client/NymData.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/OTClient.hpp"
#include "opentxs/client/OTWallet.hpp"
#include "opentxs/client/Utility.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/UniqueQueue.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"
#include "opentxs/network/zeromq/PullSocket.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/otx/Reply.hpp"

#include "api/client/InternalClient.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <map>
#include <thread>
#include <tuple>

#include "OTX.hpp"

#define CONTACT_REFRESH_DAYS 1
#define CONTRACT_DOWNLOAD_MILLISECONDS 10000
#define NYM_REGISTRATION_MILLISECONDS 10000

#define SHUTDOWN()                                                             \
    {                                                                          \
        YIELD(50);                                                             \
    }

#define YIELD(a)                                                               \
    {                                                                          \
        if (!running_) { return; }                                             \
                                                                               \
        Log::Sleep(std::chrono::milliseconds(a));                              \
    }

#define CHECK_NYM(a)                                                           \
    {                                                                          \
        if (a.empty()) {                                                       \
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid ")(#a)(".").Flush(); \
                                                                               \
            return error_task();                                               \
        }                                                                      \
    }

#define CHECK_SERVER(a, b)                                                     \
    {                                                                          \
        CHECK_NYM(a)                                                           \
                                                                               \
        if (b.empty()) {                                                       \
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid ")(#b)(".").Flush(); \
                                                                               \
            return error_task();                                               \
        }                                                                      \
    }

#define CHECK_ARGS(a, b, c)                                                    \
    {                                                                          \
        CHECK_SERVER(a, b)                                                     \
                                                                               \
        if (c.empty()) {                                                       \
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid ")(#c)(".").Flush(); \
                                                                               \
            return error_task();                                               \
        }                                                                      \
    }

#define DO_OPERATION(a, ...)                                                   \
    auto started = op.a(__VA_ARGS__);                                          \
                                                                               \
    while (false == started) {                                                 \
        LogDebug(OT_METHOD)(__FUNCTION__)(": State machine is not ready")      \
            .Flush();                                                          \
        Log::Sleep(                                                            \
            std::chrono::milliseconds(STATE_MACHINE_READY_MILLISECONDS));      \
                                                                               \
        if (running_) {                                                        \
            started = op.a(__VA_ARGS__);                                       \
        } else {                                                               \
                                                                               \
            return false;                                                      \
        }                                                                      \
    }                                                                          \
                                                                               \
    Result result = op.GetFuture().get();                                      \
    const auto success =                                                       \
        proto::LASTREPLYSTATUS_MESSAGESUCCESS == std::get<0>(result);

#define DO_OPERATION_TASK_DONE(a, ...)                                         \
    auto started = op.a(__VA_ARGS__);                                          \
                                                                               \
    while (false == started) {                                                 \
        LogDebug(OT_METHOD)(__FUNCTION__)(": State machine is not ready")      \
            .Flush();                                                          \
        Log::Sleep(                                                            \
            std::chrono::milliseconds(STATE_MACHINE_READY_MILLISECONDS));      \
                                                                               \
        if (running_) {                                                        \
            started = op.a(__VA_ARGS__);                                       \
        } else {                                                               \
                                                                               \
            return task_done(false);                                           \
        }                                                                      \
    }                                                                          \
                                                                               \
    Result result = op.GetFuture().get();                                      \
    const auto success =                                                       \
        proto::LASTREPLYSTATUS_MESSAGESUCCESS == std::get<0>(result);

#define INTRODUCTION_SERVER_KEY "introduction_server_id"
#define MASTER_SECTION "Master"
#define STATE_MACHINE_READY_MILLISECONDS 100

#define OT_METHOD "opentxs::api::client::implementation::OTX::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs
{
api::client::OTX* Factory::OTX(
    const Flag& running,
    const api::client::Manager& client,
    OTClient& otclient,
    const ContextLockCallback& lockCallback)
{
    return new api::client::implementation::OTX(
        running, client, otclient, lockCallback);
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
const std::string OTX::DEFAULT_INTRODUCTION_SERVER =
    R"(-----BEGIN OT ARMORED SERVER CONTRACT-----
Version: Open Transactions 0.99.1-113-g2b3acf5
Comment: http://opentransactions.org

CAESI290b20xcHFmREJLTmJLR3RiN0NBa0ZodFRXVFVOTHFIRzIxGiNvdHVkd3p4cWF0UHh4
bmh4VFV3RUo3am5HenE2RkhGYTRraiIMU3Rhc2ggQ3J5cHRvKr8NCAESI290dWR3enhxYXRQ
eHhuaHhUVXdFSjdqbkd6cTZGSEZhNGtqGAIoATJTCAEQAiJNCAESIQI9MywLxxKfOtai26pj
JbxKtCCPhM/DbvX08iwbW2qYqhoga6Ccvp6CABGAFj/RdWNjtg5uzIRHT5Dn+fUzdAM9SUSA
AQCIAQA6vAwIARIjb3R1ZHd6eHFhdFB4eG5oeFRVd0VKN2puR3pxNkZIRmE0a2oaI290dXdo
ZzNwb2kxTXRRdVkzR3hwYWpOaXp5bmo0NjJ4Z2RIIAIymgQIARIjb3R1d2hnM3BvaTFNdFF1
WTNHeHBhak5penluajQ2MnhnZEgYAiABKAIyI290dWR3enhxYXRQeHhuaHhUVXdFSjdqbkd6
cTZGSEZhNGtqQl0IARJTCAEQAiJNCAESIQI9MywLxxKfOtai26pjJbxKtCCPhM/DbvX08iwb
W2qYqhoga6Ccvp6CABGAFj/RdWNjtg5uzIRHT5Dn+fUzdAM9SUSAAQCIAQAaBAgBEAJKiAEI
ARACGioIARAEGAIgASogZ6MtTp4aTEDLxFfhnsGo+Esp5B4hkgjWEejNPt5J6C0aKggBEAQY
AiACKiAhqJjWf2Ugqbg6z6ps59crAx9lHwTuT6Eq4x6JmkBlGBoqCAEQBBgCIAMqII2Vps1F
C2YUMbB4kE9XsHt1jrVY6pMPV6KWc5sH3VvTem0IARIjb3R1d2hnM3BvaTFNdFF1WTNHeHBh
ak5penluajQ2MnhnZEgYASAFKkDQLsszAol/Ih56MomuBKV8zpKaw5+ry7Kse1+5nPwJlP8f
72OAgTegBlmv31K4JgLVs52EKJTBpjnV+v0pxzUOem0IARIjb3R1ZHd6eHFhdFB4eG5oeFRV
d0VKN2puR3pxNkZIRmE0a2oYAyAFKkAJZ0LTVM+XBrGbRdiZsEQSbvwqg+mqGwHD5MQ+D4h0
fPQaUrdB6Pp/HM5veox02LBKg05hVNQ64tcU+LAxK+VHQuQDCAESI290clA2dDJXY2hYMjYz
ZVpiclRuVzZyY2FCZVNQb2VqSzJnGAIgAigCMiNvdHVkd3p4cWF0UHh4bmh4VFV3RUo3am5H
enE2RkhGYTRrajonCAESI290dXdoZzNwb2kxTXRRdVkzR3hwYWpOaXp5bmo0NjJ4Z2RISogB
CAEQAhoqCAEQBBgCIAEqIDpwlCrxHNWvvtFt6k8ocB5NBo7vjkGO/mRuSOQ/j/9WGioIARAE
GAIgAiog6Dw0+AWok4dENWWc/3qhykA7NNybWecqMGs5fL8KLLYaKggBEAQYAiADKiD+s/iq
37NrYI4/xdHOYtO/ocR0YqDVz09IaDNGVEdBtnptCAESI290clA2dDJXY2hYMjYzZVpiclRu
VzZyY2FCZVNQb2VqSzJnGAEgBSpATbHtakma53Na35Be+rGvW+z1H6EtkHlljv9Mo8wfies3
in9el1Ejb4BDbGCN5ABl3lQpfedZnR+VYv2X6Y1yBnptCAESI290dXdoZzNwb2kxTXRRdVkz
R3hwYWpOaXp5bmo0NjJ4Z2RIGAEgBSpAeptEmgdqgkGUcOJCqG0MsiChEREUdDzH/hRj877u
WDIHoRHsf/k5dCOHfDct4TDszasVhGFhRdNunpgQJcp0DULnAwgBEiNvdHd6ZWd1dTY3cENI
RnZhYjZyS2JYaEpXelNvdlNDTGl5URgCIAIoAjIjb3R1ZHd6eHFhdFB4eG5oeFRVd0VKN2pu
R3pxNkZIRmE0a2o6JwgBEiNvdHV3aGczcG9pMU10UXVZM0d4cGFqTml6eW5qNDYyeGdkSEqL
AQgBEAIaKwgBEAMYAiABKiEC5p36Ivxs4Wb6CjKTnDA1MFtX3Mx2UBlrmloSt+ffXz0aKwgB
EAMYAiACKiECtMkEo4xsefeevzrBb62ll98VYZy8PipgbrPWqGUNxQMaKwgBEAMYAiADKiED
W1j2DzOZemB9OOZ/pPrFroKDfgILYu2IOtiRFfi0vDB6bQgBEiNvdHd6ZWd1dTY3cENIRnZh
YjZyS2JYaEpXelNvdlNDTGl5URgBIAUqQJYd860/Ybh13GtW+grxWtWjjmzPifHE7bTlgUWl
3bX+ZuWNeEotA4yXQvFNog4PTAOF6dbvCr++BPGepBEUEEx6bQgBEiNvdHV3aGczcG9pMU10
UXVZM0d4cGFqTml6eW5qNDYyeGdkSBgBIAUqQH6GXnKCCDDgDvcSt8dLWuVMlr75zVkHy85t
tccoy2oLHNevDvKrLfUk/zuICyaSIvDy0Kb2ytOuh/O17yabxQ8yHQgBEAEYASISb3Quc3Rh
c2hjcnlwdG8ubmV0KK03MiEIARADGAEiFnQ1NGxreTJxM2w1ZGt3bnQub25pb24orTcyRwgB
EAQYASI8b3ZpcDZrNWVycXMzYm52cjU2cmgzZm5pZ2JuZjJrZWd1cm5tNWZpYnE1NWtqenNv
YW54YS5iMzIuaTJwKK03Op8BTWVzc2FnaW5nLW9ubHkgc2VydmVyIHByb3ZpZGVkIGZvciB0
aGUgY29udmllbmllbmNlIG9mIFN0YXNoIENyeXB0byB1c2Vycy4gU2VydmljZSBpcyBwcm92
aWRlZCBhcyBpcyB3aXRob3V0IHdhcnJhbnR5IG9mIGFueSBraW5kLCBlaXRoZXIgZXhwcmVz
c2VkIG9yIGltcGxpZWQuQiCK4L5cnecfUFz/DQyvAklKC2pTmWQtxt9olQS5/0hUHUptCAES
I290clA2dDJXY2hYMjYzZVpiclRuVzZyY2FCZVNQb2VqSzJnGAUgBSpA1/bep0NTbisZqYns
MCL/PCUJ6FIMhej+ROPk41604x1jeswkkRmXRNjzLlVdiJ/pQMxG4tJ0UQwpxHxrr0IaBA==
-----END OT ARMORED SERVER CONTRACT-----)";

OTX::OTX(
    const Flag& running,
    const api::client::Manager& client,
    OTClient& otclient,
    const ContextLockCallback& lockCallback)
    : lock_callback_(lockCallback)
    , running_(running)
    , client_(client)
    , ot_client_(otclient)
    , introduction_server_lock_()
    , nym_fetch_lock_()
    , task_status_lock_()
    , refresh_counter_(0)
    , operations_()
    , server_nym_fetch_()
    , missing_nyms_()
    , missing_servers_()
    , state_machines_()
    , introduction_server_id_()
    , task_status_()
    , task_message_id_()
    , account_subscriber_callback_(zmq::ListenCallback::Factory(
          [this](const zmq::Message& message) -> void {
              this->process_account(message);
          }))
    , account_subscriber_(
          client_.ZeroMQ().SubscribeSocket(account_subscriber_callback_.get()))
    , notification_listener_callback_(zmq::ListenCallback::Factory(
          [this](const zmq::Message& message) -> void {
              this->process_notification(message);
          }))
    , notification_listener_(client_.ZeroMQ().PullSocket(
          notification_listener_callback_,
          zmq::Socket::Direction::Bind))
    , task_finished_(client_.ZeroMQ().PublishSocket())
    , auto_process_inbox_(Flag::Factory(true))
    , next_task_id_(0)
{
    // WARNING: do not access client_.Wallet() during construction
    const auto endpoint = client_.Endpoints().AccountUpdate();
    LogDetail(OT_METHOD)(__FUNCTION__)(": Connecting to ")(endpoint).Flush();
    auto listening = account_subscriber_->Start(endpoint);

    OT_ASSERT(listening)

    listening = notification_listener_->Start(
        client_.Endpoints().InternalProcessPushNotification());

    OT_ASSERT(listening)

    const auto publishing =
        task_finished_->Start(client_.Endpoints().TaskComplete());

    OT_ASSERT(publishing)
}

OTX::OperationQueue::OperationQueue(
    const api::client::Manager& api,
    const ContextID& id)
    : op_(opentxs::Factory::Operation(
          api,
          identifier::Nym::Factory(std::get<0>(id)->str()),
          std::get<1>(id)))
    , check_nym_()
    , deposit_payment_()
    , download_contract_()
    , download_mint_()
    , download_nymbox_()
    , get_transaction_numbers_()
    , issue_unit_definition_()
    , register_account_()
    , register_nym_()
    , send_message_()
    , send_payment_()
#if OT_CASH
    , send_cash_()
    , withdraw_cash_()
#endif  // OT_CASH
    , send_transfer_()
    , publish_server_contract_()
    , process_inbox_()
    , send_cheque_()
    , peer_reply_()
    , peer_request_()
    , counter_(0)
    , lock_()
    , thread_(false)
    , continue_(false)
    , tasks_()
{
    // TODO convert nymID type
    OT_ASSERT(op_);
}

std::future<void> OTX::OperationQueue::add_task(const Lock& lock)
{
    OT_ASSERT(lock.owns_lock());

    tasks_.emplace_back(counter_.load() + 2, std::promise<void>{});
    auto& task = *tasks_.rbegin();
    auto& promise = task.second;

    return promise.get_future();
}

std::future<void> OTX::OperationQueue::check_future(
    Thread task,
    std::unique_ptr<std::thread>& thread)
{
    Lock lock(lock_);
    check_thread(lock, task, thread);

    return add_task(lock);
}

void OTX::OperationQueue::check_thread(
    const Lock& lock,
    Thread task,
    std::unique_ptr<std::thread>& thread)
{
    OT_ASSERT(lock.owns_lock());

    if (thread_) {
        continue_ = true;
    } else {
        if (thread && thread->joinable()) { thread->join(); }

        thread.reset(new std::thread(task));
        thread_ = bool(thread);
    }
}

void OTX::OperationQueue::check_thread(
    Thread task,
    std::unique_ptr<std::thread>& thread)
{
    Lock lock(lock_);
    check_thread(lock, task, thread);
}

void OTX::OperationQueue::cleanup(Lock& lock)
{
    // Ignore "already locked" errors
    try {
        lock.lock();
    } catch (...) {
    }

    if (lock.owns_lock()) {
        thread_ = false;
    } else {
        OT_FAIL;
    }

    lock.unlock();
}

void OTX::OperationQueue::increment_counter(
    const bool missing,
    Lock& lock,
    bool& run)
{
    lock.lock();
    ++counter_;

    if (missing) {
        run = true;
    } else if (continue_) {
        run = true;
        continue_ = false;
    } else {
        run = false;
    }

    for (auto i = tasks_.begin(); i < tasks_.end();) {
        // auto& [limit, future] = *i;
        auto& limit = std::get<0>(*i);
        auto& future = std::get<1>(*i);
        const bool erase = (false == run) || (counter_ >= limit);

        if (erase) {
            future.set_value();
            i = tasks_.erase(i);
        } else {
            ++i;
        }
    }

    if (run) { lock.unlock(); }
}

OTX::BackgroundTask OTX::AcknowledgeBailment(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& targetNymID,
    const Identifier& requestID,
    const std::string& instructions,
    const SetID setID) const
{
    CHECK_ARGS(localNymID, serverID, targetNymID)
    CHECK_NYM(requestID)

    start_introduction_server(localNymID);
    const auto nym = client_.Wallet().Nym(localNymID);
    std::shared_ptr<const PeerReply> peerreply{PeerReply::Create(
        client_.Wallet(),
        nym,
        proto::PEERREQUEST_BAILMENT,
        requestID,
        serverID,
        instructions)};

    if (false == bool(peerreply)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create reply.").Flush();

        return error_task();
    }

    if (setID) { setID(peerreply->ID()); }

    std::time_t time{0};
    auto serializedRequest = client_.Wallet().PeerRequest(
        nym->ID(), requestID, StorageBox::INCOMINGPEERREQUEST, time);

    if (false == bool(serializedRequest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load request.").Flush();

        return error_task();
    }

    auto recipientNym = client_.Wallet().Nym(targetNymID);
    std::shared_ptr<const PeerRequest> instantiatedRequest{PeerRequest::Factory(
        client_.Wallet(), recipientNym, *serializedRequest)};

    if (false == bool(instantiatedRequest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate request.")
            .Flush();

        return error_task();
    }

    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(
        taskID,
        queue.peer_reply_.Push(
            taskID, {targetNymID, peerreply, instantiatedRequest}));
}

OTX::BackgroundTask OTX::AcknowledgeConnection(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& recipientID,
    const Identifier& requestID,
    const bool ack,
    const std::string& url,
    const std::string& login,
    const std::string& password,
    const std::string& key,
    const SetID setID) const
{
    CHECK_ARGS(localNymID, serverID, recipientID)
    CHECK_NYM(requestID)

    start_introduction_server(localNymID);
    const auto nym = client_.Wallet().Nym(localNymID);

    if (ack) {
        if (url.empty()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Warning: url is empty.")
                .Flush();
        }

        if (login.empty()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Warning: login is empty.")
                .Flush();
        }

        if (password.empty()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Warning: password is empty.")
                .Flush();
        }

        if (key.empty()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Warning: key is empty.")
                .Flush();
        }
    }

    std::shared_ptr<const PeerReply> peerreply{PeerReply::Create(
        client_.Wallet(),
        nym,
        requestID,
        serverID,
        ack,
        url,
        login,
        password,
        key)};

    if (false == bool(peerreply)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create reply.").Flush();

        return error_task();
    }

    if (setID) { setID(peerreply->ID()); }

    std::time_t time{0};
    auto serializedRequest = client_.Wallet().PeerRequest(
        nym->ID(), requestID, StorageBox::INCOMINGPEERREQUEST, time);

    if (false == bool(serializedRequest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load request.").Flush();

        return error_task();
    }

    auto recipientNym = client_.Wallet().Nym(recipientID);
    std::shared_ptr<const PeerRequest> instantiatedRequest{PeerRequest::Factory(
        client_.Wallet(), recipientNym, *serializedRequest)};

    if (false == bool(instantiatedRequest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate request.")
            .Flush();

        return error_task();
    }

    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(
        taskID,
        queue.peer_reply_.Push(
            taskID, {recipientID, peerreply, instantiatedRequest}));
}

OTX::BackgroundTask OTX::AcknowledgeNotice(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& recipientID,
    const Identifier& requestID,
    const bool ack,
    const SetID setID) const
{
    CHECK_ARGS(localNymID, serverID, recipientID)
    CHECK_NYM(requestID)

    start_introduction_server(localNymID);
    const auto nym = client_.Wallet().Nym(localNymID);
    std::shared_ptr<const PeerReply> peerreply{
        PeerReply::Create(client_.Wallet(), nym, requestID, serverID, ack)};

    if (false == bool(peerreply)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create reply.").Flush();

        return error_task();
    }

    if (setID) { setID(peerreply->ID()); }

    std::time_t time{0};
    auto serializedRequest = client_.Wallet().PeerRequest(
        nym->ID(), requestID, StorageBox::INCOMINGPEERREQUEST, time);

    if (false == bool(serializedRequest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load request.").Flush();

        return error_task();
    }

    auto recipientNym = client_.Wallet().Nym(recipientID);
    std::shared_ptr<const PeerRequest> instantiatedRequest{PeerRequest::Factory(
        client_.Wallet(), recipientNym, *serializedRequest)};

    if (false == bool(instantiatedRequest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate request.")
            .Flush();

        return error_task();
    }

    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(
        taskID,
        queue.peer_reply_.Push(
            taskID, {recipientID, peerreply, instantiatedRequest}));
}

OTX::BackgroundTask OTX::AcknowledgeOutbailment(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& recipientID,
    const Identifier& requestID,
    const std::string& details,
    const SetID setID) const
{
    CHECK_ARGS(localNymID, serverID, recipientID)
    CHECK_NYM(requestID)

    start_introduction_server(localNymID);
    const auto nym = client_.Wallet().Nym(localNymID);
    std::shared_ptr<const PeerReply> peerreply{PeerReply::Create(
        client_.Wallet(),
        nym,
        proto::PEERREQUEST_OUTBAILMENT,
        requestID,
        serverID,
        details)};

    if (false == bool(peerreply)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create reply.").Flush();

        return error_task();
    }

    if (setID) { setID(peerreply->ID()); }

    std::time_t time{0};
    auto serializedRequest = client_.Wallet().PeerRequest(
        nym->ID(), requestID, StorageBox::INCOMINGPEERREQUEST, time);

    if (false == bool(serializedRequest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load request.").Flush();

        return error_task();
    }

    auto recipientNym = client_.Wallet().Nym(recipientID);
    std::shared_ptr<const PeerRequest> instantiatedRequest{PeerRequest::Factory(
        client_.Wallet(), recipientNym, *serializedRequest)};

    if (false == bool(instantiatedRequest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate request.")
            .Flush();

        return error_task();
    }

    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(
        taskID,
        queue.peer_reply_.Push(
            taskID, {recipientID, peerreply, instantiatedRequest}));
}

OTX::BackgroundTask OTX::add_task(
    const TaskID taskID,
    const ThreadStatus status) const
{
    Lock lock(task_status_lock_);

    if (0 != task_status_.count(taskID)) { return error_task(); }

    std::promise<Result> promise{};
    [[maybe_unused]] auto [it, added] = task_status_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(taskID),
        std::forward_as_tuple(status, std::move(promise)));

    OT_ASSERT(added);

    return {it->first, it->second.second.get_future()};
}

void OTX::associate_message_id(const Identifier& messageID, const TaskID taskID)
    const
{
    Lock lock(task_status_lock_);
    task_message_id_.emplace(taskID, messageID);
}

Depositability OTX::can_deposit(
    const OTPayment& payment,
    const identifier::Nym& recipient,
    const Identifier& accountIDHint,
    OTIdentifier& depositServer,
    OTIdentifier& depositAccount) const
{
    auto unitID = Identifier::Factory();
    auto nymID = identifier::Nym::Factory();

    if (false == extract_payment_data(payment, nymID, depositServer, unitID)) {

        return Depositability::INVALID_INSTRUMENT;
    }

    auto output = valid_recipient(payment, nymID, recipient);

    if (Depositability::READY != output) { return output; }

    const bool registered = client_.Exec().IsNym_RegisteredAtServer(
        recipient.str(), depositServer->str());

    if (false == registered) {
        schedule_download_nymbox(recipient, depositServer);
        LogOutput(OT_METHOD)(__FUNCTION__)(": Recipient nym ")(recipient)(
            " not registered on server ")(depositServer)(".")
            .Flush();

        return Depositability::NOT_REGISTERED;
    }

    output = valid_account(
        payment,
        recipient,
        depositServer,
        unitID,
        accountIDHint,
        depositAccount);

    switch (output) {
        case Depositability::ACCOUNT_NOT_SPECIFIED: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Multiple valid accounts exist. "
                "This payment can not be automatically deposited.")
                .Flush();
        } break;
        case Depositability::WRONG_ACCOUNT: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": The specified account is not valid for this payment.")
                .Flush();
        } break;
        case Depositability::NO_ACCOUNT: {

            LogOutput(OT_METHOD)(__FUNCTION__)(": Recipient ")(recipient)(
                " needs an account for ")(unitID)(" on server ")(depositServer)(
                ".")
                .Flush();
            schedule_register_account(recipient, depositServer, unitID, "");

        } break;
        case Depositability::READY: {
            LogDetail(OT_METHOD)(__FUNCTION__)(": Payment can be deposited.")
                .Flush();
        } break;
        default: {
            OT_FAIL
        }
    }

    return output;
}

Messagability OTX::can_message(
    const identifier::Nym& senderNymID,
    const Identifier& recipientContactID,
    OTNymID& recipientNymID,
    OTIdentifier& serverID) const
{
    auto senderNym = client_.Wallet().Nym(senderNymID);

    if (false == bool(senderNym)) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Unable to load sender nym ")(
            senderNymID)
            .Flush();

        return Messagability::MISSING_SENDER;
    }

    const bool canSign = senderNym->HasCapability(NymCapability::SIGN_MESSAGE);

    if (false == canSign) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Sender nym ")(senderNymID)(
            " can not sign messages (no private key).")
            .Flush();

        return Messagability::INVALID_SENDER;
    }

    const auto contact = client_.Contacts().Contact(recipientContactID);

    if (false == bool(contact)) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Recipient contact ")(
            recipientContactID)(" does not exist.")
            .Flush();

        return Messagability::MISSING_CONTACT;
    }

    const auto nyms = contact->Nyms();

    if (0 == nyms.size()) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Recipient contact ")(
            recipientContactID)(" does not have a nym.")
            .Flush();

        return Messagability::CONTACT_LACKS_NYM;
    }

    std::shared_ptr<const Nym> recipientNym{nullptr};

    for (const auto& it : nyms) {
        recipientNym = client_.Wallet().Nym(it);

        if (recipientNym) {
            recipientNymID = identifier::Nym::Factory(it->str());
            break;
        }
    }

    if (false == bool(recipientNym)) {
        for (const auto& id : nyms) {
            // TODO convert nym id type
            const auto nymID = identifier::Nym::Factory(id->str());
            missing_nyms_.Push(++next_task_id_, nymID);
        }

        LogDetail(OT_METHOD)(__FUNCTION__)(": Recipient contact ")(
            recipientContactID)(" credentials not available.")
            .Flush();

        return Messagability::MISSING_RECIPIENT;
    }

    OT_ASSERT(recipientNym)

    const auto& claims = recipientNym->Claims();
    serverID = Identifier::Factory(claims.PreferredOTServer());

    // TODO maybe some of the other nyms in this contact do specify a server
    if (serverID->empty()) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Recipient contact ")(
            recipientContactID)(", nym ")(recipientNymID)(
            ": credentials do not specify a server.")
            .Flush();
        missing_nyms_.Push(++next_task_id_, recipientNymID);

        return Messagability::NO_SERVER_CLAIM;
    }

    const bool registered = client_.Exec().IsNym_RegisteredAtServer(
        senderNymID.str(), serverID->str());

    if (false == registered) {
        schedule_download_nymbox(senderNymID, serverID);
        LogDetail(OT_METHOD)(__FUNCTION__)(": Sender nym ")(senderNymID)(
            " not registered on server ")(serverID)
            .Flush();

        return Messagability::UNREGISTERED;
    }

    return Messagability::READY;
}

Depositability OTX::CanDeposit(
    const identifier::Nym& recipientNymID,
    const OTPayment& payment) const
{
    auto accountHint = Identifier::Factory();

    return CanDeposit(recipientNymID, accountHint, payment);
}

Depositability OTX::CanDeposit(
    const identifier::Nym& recipientNymID,
    const Identifier& accountIDHint,
    const OTPayment& payment) const
{
    auto serverID = Identifier::Factory();
    auto accountID = Identifier::Factory();

    return can_deposit(
        payment, recipientNymID, accountIDHint, serverID, accountID);
}

Messagability OTX::CanMessage(
    const identifier::Nym& senderNymID,
    const Identifier& recipientContactID,
    const bool startIntroductionServer) const
{
    if (startIntroductionServer) { start_introduction_server(senderNymID); }

    if (senderNymID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid sender.").Flush();

        return Messagability::INVALID_SENDER;
    }

    if (recipientContactID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid recipient.").Flush();

        return Messagability::MISSING_CONTACT;
    }

    auto nymID = identifier::Nym::Factory();
    auto serverID = Identifier::Factory();

    return can_message(senderNymID, recipientContactID, nymID, serverID);
}

void OTX::check_nym_revision(
    const ServerContext& context,
    OperationQueue& queue) const
{
    if (context.StaleNym()) {
        const auto& nymID = context.Nym()->ID();
        LogDetail(OT_METHOD)(__FUNCTION__)(": Nym ")(nymID)(
            " has is newer than version last registered version on server ")(
            context.Server())(".")
            .Flush();
        queue.register_nym_.Push(++next_task_id_, true);
    }
}

bool OTX::check_registration(
    const identifier::Nym& nymID,
    const Identifier& serverID,
    api::client::internal::Operation& op,
    std::shared_ptr<const ServerContext>& context) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())

    context = client_.Wallet().ServerContext(nymID, serverID);
    RequestNumber request{0};

    if (context) {
        request = context->Request();
    } else {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Nym ")(nymID)(
            " has never registered on ")(serverID)
            .Flush();
    }

    if (0 != request) {
        OT_ASSERT(context)

        return true;
    }

    const auto output = register_nym(++next_task_id_, op, false);

    if (output) {
        context = client_.Wallet().ServerContext(nymID, serverID);

        OT_ASSERT(context)
    }

    return output;
}

bool OTX::check_server_contract(const Identifier& serverID) const
{
    OT_ASSERT(false == serverID.empty())

    const auto serverContract = client_.Wallet().Server(serverID);

    if (serverContract) { return true; }

    LogDetail(OT_METHOD)(__FUNCTION__)(": Server contract for ")(serverID)(
        " is not in the wallet.")
        .Flush();
    missing_servers_.Push(++next_task_id_, serverID);

    return false;
}

bool OTX::check_server_name(OperationQueue& queue, const ServerContext& context)
    const
{
    auto& op = *queue.op_;
    const auto server = client_.Wallet().Server(op.ServerID());

    OT_ASSERT(server)

    const auto myName = server->Alias();
    const auto hisName = server->EffectiveName();

    if (myName == hisName) { return true; }

    DO_OPERATION(
        AddClaim,
        proto::CONTACTSECTION_SCOPE,
        proto::CITEMTYPE_SERVER,
        String::Factory(myName),
        true);

    if (success) {
        // TODO nym id type
        queue.check_nym_.Push(
            ++next_task_id_,
            identifier::Nym::Factory(context.RemoteNym().ID().str()));
    }

    return success;
}

bool OTX::CheckTransactionNumbers(
    const identifier::Nym& nym,
    const Identifier& serverID,
    const std::size_t quantity) const
{
    auto context = client_.Wallet().ServerContext(nym, serverID);

    if (false == bool(context)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Nym is not registered").Flush();

        return false;
    }

    const auto available = context->AvailableNumbers();

    if (quantity <= available) { return true; }

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Asking server for more numbers.")
        .Flush();
    auto& queue = get_operations({nym, serverID});
    const auto taskID{++next_task_id_};
    start_task(taskID, queue.get_transaction_numbers_.Push(taskID, true));
    auto status = Status(taskID);

    while (ThreadStatus::RUNNING == status) {
        Log::Sleep(std::chrono::milliseconds(100));
        status = Status(taskID);
    }

    if (ThreadStatus::FINISHED_SUCCESS == status) { return true; }

    return false;
}

OTX::Finished OTX::ContextIdle(
    const identifier::Nym& nym,
    const identifier::Server& server) const
{
    const ContextID id{nym, server};

    return get_future(id);
}

bool OTX::deposit_cheque(
    const TaskID taskID,
    api::client::internal::Operation& op,
    const Identifier& accountID,
    const std::shared_ptr<const OTPayment>& payment,
    UniqueQueue<DepositPaymentTask>& retry) const
{
    OT_ASSERT(false == accountID.empty())
    OT_ASSERT(payment)

    if ((false == payment->IsCheque()) && (false == payment->IsVoucher())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unhandled payment type.").Flush();

        return finish_task(taskID, false, error_result());
    }

    std::shared_ptr<Cheque> cheque{client_.Factory().Cheque()};

    OT_ASSERT(cheque);

    const auto loaded = cheque->LoadContractFromString(payment->Payment());

    if (false == loaded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid cheque.").Flush();

        return finish_task(taskID, false, error_result());
    }

    DO_OPERATION(DepositCheque, accountID, cheque);

    if (success) {
        finish_task(taskID, success, std::move(result));
    } else {
        retry.Push(taskID, {accountID, payment});
    }

    return false;
}

std::size_t OTX::DepositCheques(const identifier::Nym& nymID) const
{
    std::size_t output{0};
    const auto workflows = client_.Workflow().List(
        nymID,
        proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE,
        proto::PAYMENTWORKFLOWSTATE_CONVEYED);

    for (const auto& id : workflows) {
        const auto chequeState =
            client_.Workflow().LoadChequeByWorkflow(nymID, id);
        const auto& [state, cheque] = chequeState;

        if (proto::PAYMENTWORKFLOWSTATE_CONVEYED != state) { continue; }

        OT_ASSERT(cheque)

        if (queue_cheque_deposit(nymID, *cheque)) { ++output; }
    }

    return output;
}

std::size_t OTX::DepositCheques(
    const identifier::Nym& nymID,
    const std::set<OTIdentifier>& chequeIDs) const
{
    std::size_t output{0};

    if (chequeIDs.empty()) { return DepositCheques(nymID); }

    for (const auto& id : chequeIDs) {
        const auto chequeState = client_.Workflow().LoadCheque(nymID, id);
        const auto& [state, cheque] = chequeState;

        if (proto::PAYMENTWORKFLOWSTATE_CONVEYED != state) { continue; }

        OT_ASSERT(cheque)

        if (queue_cheque_deposit(nymID, *cheque)) { ++output; }
    }

    return {};
}

OTX::BackgroundTask OTX::DepositPayment(
    const identifier::Nym& recipientNymID,
    const std::shared_ptr<const OTPayment>& payment) const
{
    auto notUsed = Identifier::Factory();

    return DepositPayment(recipientNymID, notUsed, payment);
}

OTX::BackgroundTask OTX::DepositPayment(
    const identifier::Nym& recipientNymID,
    const Identifier& accountIDHint,
    const std::shared_ptr<const OTPayment>& payment) const
{
    OT_ASSERT(payment)

    if (recipientNymID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid recipient.").Flush();

        return error_task();
    }

    auto serverID = Identifier::Factory();
    auto accountID = Identifier::Factory();
    const auto status = can_deposit(
        *payment, recipientNymID, accountIDHint, serverID, accountID);

    switch (status) {
        case Depositability::READY:
        case Depositability::NOT_REGISTERED:
        case Depositability::NO_ACCOUNT: {
            start_introduction_server(recipientNymID);
            auto& queue = get_operations({recipientNymID, serverID});
            const auto taskID{++next_task_id_};

            return start_task(
                taskID,
                queue.deposit_payment_.Push(taskID, {accountIDHint, payment}));
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unable to queue payment for download.")
                .Flush();
        }
    }

    return error_task();
}

void OTX::DisableAutoaccept() const { auto_process_inbox_->Off(); }

bool OTX::download_contract(
    const TaskID taskID,
    api::client::internal::Operation& op,
    const Identifier& contractID) const
{
    OT_ASSERT(false == contractID.empty())

    DO_OPERATION(DownloadContract, contractID);

    return finish_task(taskID, success, std::move(result));
}

#if OT_CASH
bool OTX::download_mint(
    const TaskID taskID,
    api::client::internal::Operation& op,
    const DownloadMintTask& task) const
{
    DO_OPERATION(Start, internal::Operation::Type::DownloadMint, task, {});

    return finish_task(taskID, success, std::move(result));
}
#endif

bool OTX::download_nym(
    const TaskID taskID,
    api::client::internal::Operation& op,
    const identifier::Nym& targetNymID) const
{
    OT_ASSERT(false == targetNymID.empty())

    ServerContext::ExtraArgs args{};

    DO_OPERATION(Start, internal::Operation::Type::CheckNym, targetNymID, args);

    return finish_task(taskID, success, std::move(result));
}

bool OTX::download_nymbox(
    const TaskID taskID,
    api::client::internal::Operation& op) const
{
    op.join();
    auto contextE =
        client_.Wallet().mutable_ServerContext(op.NymID(), op.ServerID());
    auto& context = contextE.It();
    context.Join();
    context.ResetThread();
    auto future = context.RefreshNymbox(client_);

    if (false == bool(future)) {

        return finish_task(taskID, false, error_result());
    }

    Result result{future->get()};
    const auto success =
        proto::LASTREPLYSTATUS_MESSAGESUCCESS == std::get<0>(result);

    return finish_task(taskID, success, std::move(result));
}

OTX::BackgroundTask OTX::DownloadContract(
    const identifier::Nym& localNymID,
    const Identifier& serverID,
    const Identifier& contractID) const
{
    CHECK_ARGS(localNymID, serverID, contractID)

    start_introduction_server(localNymID);
    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(
        taskID, queue.download_contract_.Push(taskID, contractID));
}

#if OT_CASH
OTX::BackgroundTask OTX::DownloadMint(
    const identifier::Nym& nym,
    const identifier::Server& server,
    const identifier::UnitDefinition& unit) const
{
    CHECK_ARGS(nym, server, unit);

    start_introduction_server(nym);
    auto& queue = get_operations({nym, server});
    const auto taskID{++next_task_id_};

    return start_task(taskID, queue.download_mint_.Push(taskID, unit));
}
#endif

OTX::BackgroundTask OTX::DownloadNym(
    const identifier::Nym& localNymID,
    const Identifier& serverID,
    const identifier::Nym& targetNymID) const
{
    CHECK_ARGS(localNymID, serverID, targetNymID)

    start_introduction_server(localNymID);
    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};
    // TODO convert nymid type

    return start_task(
        taskID,
        queue.check_nym_.Push(
            taskID, identifier::Nym::Factory(targetNymID.str())));
}

OTX::BackgroundTask OTX::DownloadNymbox(
    const identifier::Nym& localNymID,
    const Identifier& serverID) const
{
    return schedule_download_nymbox(localNymID, serverID);
}

bool OTX::extract_payment_data(
    const OTPayment& payment,
    OTNymID& nymID,
    OTIdentifier& serverID,
    OTIdentifier& unitID) const
{
    if (false == payment.GetRecipientNymID(nymID)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to load recipient nym from instrument.")
            .Flush();

        return false;
    }

    if (false == payment.GetNotaryID(serverID)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to load recipient nym from instrument.")
            .Flush();

        return false;
    }

    OT_ASSERT(false == serverID->empty())

    if (false == payment.GetInstrumentDefinitionID(unitID)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to load recipient nym from instrument.")
            .Flush();

        return false;
    }

    OT_ASSERT(false == unitID->empty())

    return true;
}

OTX::Result OTX::error_result()
{
    Result output{proto::LASTREPLYSTATUS_NOTSENT, nullptr};

    return output;
}

OTX::BackgroundTask OTX::error_task()
{
    BackgroundTask output{0, Future{}};

    return output;
}

bool OTX::find_nym(
    api::client::internal::Operation& op,
    const identifier::Nym& targetNymID) const
{
    OT_ASSERT(false == targetNymID.empty())

    const auto nym = client_.Wallet().Nym(targetNymID);

    if (nym) {
        missing_nyms_.CancelByValue(targetNymID);

        return true;
    }

    if (download_nym(++next_task_id_, op, targetNymID)) {
        missing_nyms_.CancelByValue(targetNymID);

        return true;
    }

    return false;
}

bool OTX::find_server(
    api::client::internal::Operation& op,
    const identifier::Nym& targetID) const
{
    OT_ASSERT(false == targetID.empty())

    const auto serverContract = client_.Wallet().Server(targetID);

    if (serverContract) {
        missing_servers_.CancelByValue(targetID);

        return true;
    }

    if (download_contract(++next_task_id_, op, targetID)) {
        missing_servers_.CancelByValue(targetID);

        return true;
    }

    return false;
}

OTX::BackgroundTask OTX::FindNym(const identifier::Nym& nymID) const
{
    CHECK_NYM(nymID)

    const auto taskID{++next_task_id_};
    // TODO convert nym id type
    const auto id = nymID.str();

    return start_task(
        taskID,
        missing_nyms_.Push(taskID, identifier::Nym::Factory(id.c_str())));
}

OTX::BackgroundTask OTX::FindNym(
    const identifier::Nym& nymID,
    const Identifier& serverIDHint) const
{
    CHECK_NYM(nymID)

    auto& serverQueue = get_nym_fetch(serverIDHint);
    const auto taskID{++next_task_id_};
    // TODO convert nym ID type

    return start_task(taskID, serverQueue.Push(taskID, nymID));
}

OTX::BackgroundTask OTX::FindServer(const Identifier& serverID) const
{
    CHECK_NYM(serverID)

    const auto taskID{++next_task_id_};

    return start_task(taskID, missing_servers_.Push(taskID, serverID));
}

bool OTX::finish_task(const TaskID taskID, const bool success, Result&& result)
    const
{
    if (success) {
        update_task(taskID, ThreadStatus::FINISHED_SUCCESS, std::move(result));
    } else {
        update_task(taskID, ThreadStatus::FINISHED_FAILED, std::move(result));
    }

    return success;
}

bool OTX::get_admin(
    const TaskID taskID,
    api::client::internal::Operation& op,
    const OTPassword& password) const
{
    DO_OPERATION(RequestAdmin, String::Factory(password.getPassword()));

    return finish_task(taskID, success, std::move(result));
}

std::future<void> OTX::get_future(const ContextID& id) const
{
    auto& queue = get_queue(id);

    return queue.check_future(
        [id, &queue, this]() { state_machine(id, queue); },
        state_machines_[id]);
}

OTIdentifier OTX::get_introduction_server(const Lock& lock) const
{
    OT_ASSERT(verify_lock(lock, introduction_server_lock_))

    bool keyFound = false;
    auto serverID = String::Factory();
    const bool config = client_.Config().Check_str(
        String::Factory(MASTER_SECTION),
        String::Factory(INTRODUCTION_SERVER_KEY),
        serverID,
        keyFound);

    if (!config || !keyFound || !serverID->Exists()) {

        return import_default_introduction_server(lock);
    }

    return Identifier::Factory(serverID);
}

UniqueQueue<OTNymID>& OTX::get_nym_fetch(const Identifier& serverID) const
{
    Lock lock(nym_fetch_lock_);

    return server_nym_fetch_[serverID];
}

OTX::OperationQueue& OTX::get_operations(const ContextID& id) const
{
    auto& queue = get_queue(id);
    queue.check_thread(
        [id, &queue, this]() { state_machine(id, queue); },
        state_machines_[id]);

    return queue;
}

OTX::OperationQueue& OTX::get_queue(const ContextID& id) const
{
    Lock lock(lock_);
    auto it = operations_.find(id);

    if (operations_.end() == it) {
        auto added = operations_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(id),
            std::forward_as_tuple(client_, id));
        it = std::get<0>(added);
    }

    return it->second;
}

bool OTX::get_transaction_numbers(
    const TaskID taskID,
    api::client::internal::Operation& op) const
{
    ServerContext::ExtraArgs args{};

    DO_OPERATION(Start, internal::Operation::Type::GetTransactionNumbers, args);

    return finish_task(taskID, success, std::move(result));
}

OTIdentifier OTX::import_default_introduction_server(const Lock& lock) const
{
    OT_ASSERT(verify_lock(lock, introduction_server_lock_))

    const auto serialized = proto::StringToProto<proto::ServerContract>(
        String::Factory(DEFAULT_INTRODUCTION_SERVER.c_str()));
    const auto instantiated = client_.Wallet().Server(serialized);

    OT_ASSERT(instantiated)

    return set_introduction_server(lock, *instantiated);
}

bool OTX::initiate_peer_reply(
    const TaskID taskID,
    api::client::internal::Operation& op,
    PeerReplyTask& task) const
{
    const auto [targetNymID, peerReply, peerRequest] = task;

    DO_OPERATION(SendPeerReply, targetNymID, peerReply, peerRequest);

    return finish_task(taskID, success, std::move(result));
}
bool OTX::initiate_peer_request(
    const TaskID taskID,
    api::client::internal::Operation& op,
    PeerRequestTask& task) const
{
    const auto& [targetNymID, peerRequest] = task;

    DO_OPERATION(SendPeerRequest, targetNymID, peerRequest);

    return finish_task(taskID, success, std::move(result));
}
OTX::BackgroundTask OTX::InitiateBailment(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& targetNymID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const SetID setID) const
{
    CHECK_ARGS(localNymID, serverID, instrumentDefinitionID)
    CHECK_NYM(targetNymID)

    start_introduction_server(localNymID);
    const auto nym = client_.Wallet().Nym(localNymID);
    std::shared_ptr<const PeerRequest> peerrequest{PeerRequest::Create(
        client_.Wallet(),
        nym,
        proto::PEERREQUEST_BAILMENT,
        instrumentDefinitionID,
        serverID)};

    if (false == bool(peerrequest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create request.")
            .Flush();
        return error_task();
    }

    if (setID) { setID(peerrequest->ID()); }

    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(
        taskID, queue.peer_request_.Push(taskID, {targetNymID, peerrequest}));
}

OTX::BackgroundTask OTX::InitiateOutbailment(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& targetNymID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const Amount amount,
    const std::string& message,
    const SetID setID) const
{
    CHECK_ARGS(localNymID, serverID, instrumentDefinitionID)
    CHECK_NYM(targetNymID)

    start_introduction_server(localNymID);
    const auto nym = client_.Wallet().Nym(localNymID);
    std::shared_ptr<const PeerRequest> peerrequest{PeerRequest::Create(
        client_.Wallet(),
        nym,
        proto::PEERREQUEST_OUTBAILMENT,
        instrumentDefinitionID,
        serverID,
        amount,
        message)};

    if (false == bool(peerrequest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create request.")
            .Flush();
        return error_task();
    }

    if (setID) { setID(peerrequest->ID()); }

    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(
        taskID, queue.peer_request_.Push(taskID, {targetNymID, peerrequest}));
}

OTX::BackgroundTask OTX::InitiateRequestConnection(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& targetNymID,
    const proto::ConnectionInfoType& type,
    const SetID setID) const
{
    CHECK_SERVER(localNymID, serverID)
    CHECK_NYM(targetNymID)

    start_introduction_server(localNymID);
    const auto nym = client_.Wallet().Nym(localNymID);
    std::shared_ptr<const PeerRequest> peerrequest{PeerRequest::Create(
        client_.Wallet(),
        nym,
        proto::PEERREQUEST_CONNECTIONINFO,
        type,
        targetNymID,
        serverID)};

    if (false == bool(peerrequest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create request.")
            .Flush();
        return error_task();
    }

    if (setID) { setID(peerrequest->ID()); }

    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(
        taskID, queue.peer_request_.Push(taskID, {targetNymID, peerrequest}));
}

OTX::BackgroundTask OTX::InitiateStoreSecret(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& targetNymID,
    const proto::SecretType& type,
    const std::string& primary,
    const std::string& secondary,
    const SetID setID) const
{
    CHECK_ARGS(localNymID, serverID, targetNymID)

    start_introduction_server(localNymID);
    const auto nym = client_.Wallet().Nym(localNymID);

    if (primary.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Warning: primary is empty.")
            .Flush();
    }

    if (secondary.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Warning: secondary is empty.")
            .Flush();
    }

    std::shared_ptr<const PeerRequest> peerrequest{PeerRequest::Create(
        client_.Wallet(),
        nym,
        proto::PEERREQUEST_STORESECRET,
        type,
        targetNymID,
        primary,
        secondary,
        serverID)};

    if (false == bool(peerrequest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create request.")
            .Flush();
        return error_task();
    }

    if (setID) { setID(peerrequest->ID()); }

    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(
        taskID, queue.peer_request_.Push(taskID, {targetNymID, peerrequest}));
}

const Identifier& OTX::IntroductionServer() const
{
    Lock lock(introduction_server_lock_);

    if (false == bool(introduction_server_id_)) {
        load_introduction_server(lock);
    }

    OT_ASSERT(introduction_server_id_)

    return *introduction_server_id_;
}

bool OTX::issue_unit_definition(
    const TaskID taskID,
    api::client::internal::Operation& op,
    const Identifier& unitID,
    const std::string& label) const
{
    auto unitDefinition = client_.Wallet().UnitDefinition(unitID);

    if (false == bool(unitDefinition)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unit definition not found.")
            .Flush();

        return false;
    }

    auto serialized = std::make_shared<proto::UnitDefinition>();

    OT_ASSERT(serialized);

    *serialized = unitDefinition->PublicContract();
    ServerContext::ExtraArgs args{label, false};

    DO_OPERATION(IssueUnitDefinition, serialized, args);

    return finish_task(taskID, success, std::move(result));
}

OTX::BackgroundTask OTX::IssueUnitDefinition(
    const identifier::Nym& localNymID,
    const Identifier& serverID,
    const Identifier& unitID,
    const std::string& label) const
{
    CHECK_ARGS(localNymID, serverID, unitID)

    start_introduction_server(localNymID);
    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(
        taskID, queue.issue_unit_definition_.Push(taskID, {unitID, label}));
}

void OTX::load_introduction_server(const Lock& lock) const
{
    OT_ASSERT(verify_lock(lock, introduction_server_lock_))

    introduction_server_id_.reset(
        new OTIdentifier(get_introduction_server(lock)));
}

bool OTX::message_nym(
    const TaskID taskID,
    api::client::internal::Operation& op,
    const MessageTask& task) const
{
    const auto& [recipient, text, setID] = task;
    auto messageID = Identifier::Factory();
    auto updateID = [&](const Identifier& in) -> void {
        messageID = in;
        auto& id = std::get<2>(task);

        if (id) { id(in); }
    };

    OT_ASSERT(false == recipient->empty())

    DO_OPERATION(SendMessage, recipient, String::Factory(text), updateID);

    if (success) {
        if (false == messageID->empty()) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Sent message: ")(messageID)
                .Flush();
            associate_message_id(messageID, taskID);
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message ID").Flush();
        }
    }

    return finish_task(taskID, success, std::move(result));
}

OTX::BackgroundTask OTX::MessageContact(
    const identifier::Nym& senderNymID,
    const Identifier& contactID,
    const std::string& message,
    const SetID setID) const
{
    CHECK_SERVER(senderNymID, contactID)

    start_introduction_server(senderNymID);
    auto serverID = Identifier::Factory();
    auto recipientNymID = identifier::Nym::Factory();
    const auto canMessage =
        can_message(senderNymID, contactID, recipientNymID, serverID);

    if (Messagability::READY != canMessage) { return error_task(); }

    OT_ASSERT(false == serverID->empty())
    OT_ASSERT(false == recipientNymID->empty())

    auto& queue = get_operations({senderNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(
        taskID,
        queue.send_message_.Push(taskID, {recipientNymID, message, setID}));
}

std::pair<ThreadStatus, OTX::MessageID> OTX::MessageStatus(
    const TaskID taskID) const
{
    std::pair<ThreadStatus, MessageID> output{{}, Identifier::Factory()};
    auto& [threadStatus, messageID] = output;
    Lock lock(task_status_lock_);
    threadStatus = status(lock, taskID);

    if (threadStatus == ThreadStatus::FINISHED_SUCCESS) {
        auto it = task_message_id_.find(taskID);

        if (task_message_id_.end() != it) {
            messageID = it->second;
            task_message_id_.erase(it);
        }
    }

    return output;
}

OTX::BackgroundTask OTX::NotifyBailment(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& targetNymID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const Identifier& requestID,
    const std::string& txid,
    const Amount amount,
    const SetID setID) const
{
    CHECK_ARGS(localNymID, serverID, instrumentDefinitionID)
    CHECK_NYM(targetNymID)
    CHECK_NYM(requestID)

    start_introduction_server(localNymID);
    const auto nym = client_.Wallet().Nym(localNymID);
    std::shared_ptr<const PeerRequest> peerrequest{PeerRequest::Create(
        client_.Wallet(),
        nym,
        proto::PEERREQUEST_PENDINGBAILMENT,
        instrumentDefinitionID,
        serverID,
        targetNymID,
        requestID,
        txid,
        amount)};

    if (false == bool(peerrequest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create request.")
            .Flush();
        return error_task();
    }

    if (setID) { setID(peerrequest->ID()); }

    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(
        taskID, queue.peer_request_.Push(taskID, {targetNymID, peerrequest}));
}

bool OTX::pay_nym(
    const TaskID taskID,
    api::client::internal::Operation& op,
    const identifier::Nym& recipient,
    std::shared_ptr<const OTPayment>& payment) const
{
    OT_ASSERT(false == recipient.empty())

    DO_OPERATION(ConveyPayment, recipient, payment);

    return finish_task(taskID, success, std::move(result));
}

#if OT_CASH
bool OTX::pay_nym_cash(
    const TaskID taskID,
    api::client::internal::Operation& op,
    const identifier::Nym& recipient,
    const Identifier& workflowID) const
{
    OT_ASSERT(false == recipient.empty())

    DO_OPERATION(SendCash, recipient, workflowID);

    return finish_task(taskID, success, std::move(result));
}
#endif  // OT_CASH

OTX::BackgroundTask OTX::PayContact(
    const identifier::Nym& senderNymID,
    const Identifier& contactID,
    std::shared_ptr<const OTPayment> payment) const
{
    CHECK_SERVER(senderNymID, contactID)

    start_introduction_server(senderNymID);
    auto serverID = Identifier::Factory();
    auto recipientNymID = identifier::Nym::Factory();
    const auto canMessage =
        can_message(senderNymID, contactID, recipientNymID, serverID);

    if (Messagability::READY != canMessage) { return error_task(); }

    OT_ASSERT(false == serverID->empty())
    OT_ASSERT(false == recipientNymID->empty())

    auto& queue = get_operations({senderNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(
        taskID,
        queue.send_payment_.Push(
            taskID,
            {recipientNymID, std::shared_ptr<const OTPayment>(payment)}));
}

#if OT_CASH
OTX::BackgroundTask OTX::PayContactCash(
    const identifier::Nym& senderNymID,
    const Identifier& contactID,
    const Identifier& workflowID) const
{
    CHECK_SERVER(senderNymID, contactID)

    start_introduction_server(senderNymID);
    auto serverID = Identifier::Factory();
    auto recipientNymID = identifier::Nym::Factory();
    const auto canMessage =
        can_message(senderNymID, contactID, recipientNymID, serverID);

    if (Messagability::READY != canMessage) { return error_task(); }

    OT_ASSERT(false == serverID->empty())
    OT_ASSERT(false == recipientNymID->empty())

    auto& queue = get_operations({senderNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(
        taskID, queue.send_cash_.Push(taskID, {recipientNymID, workflowID}));
}
#endif  // OT_CASH

OTX::BackgroundTask OTX::ProcessInbox(
    const identifier::Nym& localNymID,
    const Identifier& serverID,
    const Identifier& accountID) const
{
    CHECK_ARGS(localNymID, serverID, accountID)

    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(taskID, queue.process_inbox_.Push(taskID, accountID));
}

void OTX::process_account(const zmq::Message& message) const
{
    OT_ASSERT(2 == message.Body().size())

    const std::string id(*message.Body().begin());
    const auto& balance = message.Body().at(1);

    OT_ASSERT(balance.size() == sizeof(Amount))

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Account ")(id)(" balance: ")(
        *static_cast<const Amount*>(balance.data()))
        .Flush();
}

bool OTX::process_inbox(
    const TaskID taskID,
    api::client::internal::Operation& op,
    const Identifier& accountID) const
{
    OT_ASSERT(false == accountID.empty())

    DO_OPERATION(UpdateAccount, accountID);

    return finish_task(taskID, success, std::move(result));
}

void OTX::process_notification(const zmq::Message& message) const
{
    OT_ASSERT(0 < message.Body().size())

    const auto& frame = message.Body().at(0);
    const auto notification = otx::Reply::Factory(
        client_,
        proto::RawToProto<proto::ServerReply>(frame.data(), frame.size()));
    const auto& nymID = notification->Recipient();
    const auto& serverID = notification->Server();

    if (false ==
        valid_context(identifier::Nym::Factory(nymID.str()), serverID)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": No context available to handle notification.")
            .Flush();

        return;
    }

    auto context = client_.Wallet().mutable_ServerContext(
        identifier::Nym::Factory(nymID.str()), serverID);

    switch (notification->Type()) {
        case proto::SERVERREPLY_PUSH: {
            context.It().ProcessNotification(client_, notification);
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unsupported server reply type: ")(notification->Type())(".")
                .Flush();
        }
    }
}

bool OTX::publish_server_contract(
    const TaskID taskID,
    api::client::internal::Operation& op,
    const identifier::Server& serverID) const
{
    OT_ASSERT(false == serverID.empty())

    DO_OPERATION(PublishContract, serverID);

    return finish_task(taskID, success, std::move(result));
}

bool OTX::publish_server_registration(
    const identifier::Nym& nymID,
    const Identifier& serverID,
    const bool forcePrimary) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())

    auto nym = client_.Wallet().mutable_Nym(nymID);

    return nym.AddPreferredOTServer(serverID.str(), forcePrimary);
}

OTX::BackgroundTask OTX::PublishServerContract(
    const identifier::Nym& localNymID,
    const Identifier& serverID,
    const Identifier& contractID) const
{
    CHECK_ARGS(localNymID, serverID, contractID)

    start_introduction_server(localNymID);
    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};

    // TODO update server id type
    return start_task(
        taskID,
        queue.publish_server_contract_.Push(
            taskID, identifier::Server::Factory(contractID.str())));
}

bool OTX::queue_cheque_deposit(
    const identifier::Nym& nymID,
    const Cheque& cheque) const
{
    auto payment{client_.Factory().Payment(String::Factory(cheque))};

    OT_ASSERT(false != bool(payment));

    payment->SetTempValuesFromCheque(cheque);

    if (cheque.GetRecipientNymID().empty()) {
        payment->SetTempRecipientNymID(nymID);
    }

    std::shared_ptr<OTPayment> ppayment{payment.release()};
    const auto task = DepositPayment(nymID, ppayment);
    const auto taskID = std::get<0>(task);

    return (0 != taskID);
}

void OTX::Refresh() const
{
    client_.Pair().Update();
    refresh_accounts();

    SHUTDOWN()

    refresh_contacts();
    ++refresh_counter_;
}

std::uint64_t OTX::RefreshCount() const { return refresh_counter_.load(); }

void OTX::refresh_accounts() const
{
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Begin").Flush();
    const auto serverList = client_.Wallet().ServerList();
    const auto accounts = client_.Storage().AccountList();

    for (const auto server : serverList) {
        SHUTDOWN()

        const auto serverID = Identifier::Factory(server.first);
        LogDetail(OT_METHOD)(__FUNCTION__)(": Considering server ")(serverID)
            .Flush();

        for (const auto& nymID : client_.OTAPI().LocalNymList()) {
            SHUTDOWN()
            auto logStr = String::Factory(": Nym ");
            logStr->Concatenate("%s", nymID->str().c_str());
            const bool registered =
                client_.OTAPI().IsNym_RegisteredAtServer(nymID, serverID);

            if (registered) {
                logStr->Concatenate(" %s ", "is");
                auto& queue = get_operations({nymID, serverID});
                const auto taskID{++next_task_id_};
                queue.download_nymbox_.Push(taskID, true);
            } else {
                logStr->Concatenate(" %s ", "is not");
            }

            logStr->Concatenate("%s", " registered here.");
            LogDetail(OT_METHOD)(__FUNCTION__)(logStr).Flush();
        }
    }

    SHUTDOWN()

    for (const auto& it : accounts) {
        SHUTDOWN()
        const auto accountID = Identifier::Factory(it.first);
        const auto nymID = client_.Storage().AccountOwner(accountID);
        const auto serverID = client_.Storage().AccountServer(accountID);
        LogDetail(OT_METHOD)(__FUNCTION__)(": Account ")(accountID)(": ")(
            "  * Owned by nym: ")(nymID)("  * On server: ")(serverID)
            .Flush();
        auto& queue =
            get_operations({identifier::Nym::Factory(nymID->str()), serverID});
        const auto taskID{++next_task_id_};
        queue.process_inbox_.Push(taskID, accountID);
    }

    LogVerbose(OT_METHOD)(__FUNCTION__)(": End").Flush();
}

void OTX::refresh_contacts() const
{
    for (const auto& it : client_.Contacts().ContactList()) {
        SHUTDOWN()

        const auto& contactID = it.first;
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Considering contact: ")(
            contactID)
            .Flush();
        const auto contact =
            client_.Contacts().Contact(Identifier::Factory(contactID));

        OT_ASSERT(contact);

        const auto now = std::time(nullptr);
        const std::chrono::seconds interval(now - contact->LastUpdated());
        const std::chrono::hours limit(24 * CONTACT_REFRESH_DAYS);
        const auto nymList = contact->Nyms();

        if (nymList.empty()) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(
                ": No nyms associated with this contact.")
                .Flush();

            continue;
        }

        for (const auto& it : nymList) {
            SHUTDOWN()

            // TODO convert nym ID type
            const auto nymID = identifier::Nym::Factory(it->str());
            const auto nym = client_.Wallet().Nym(nymID);
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Considering nym: ")(nymID)
                .Flush();

            if (nym) {
                client_.Contacts().Update(nym->asPublicNym());
            } else {
                LogVerbose(OT_METHOD)(__FUNCTION__)(
                    ": We don't have credentials for this nym. "
                    " Will search on all servers.")
                    .Flush();
                const auto taskID{++next_task_id_};
                missing_nyms_.Push(taskID, nymID);

                continue;
            }

            if (interval > limit) {
                LogVerbose(OT_METHOD)(__FUNCTION__)(
                    ": Hours since last update (")(interval.count())(
                    ") exceeds the limit (")(limit.count())(")")
                    .Flush();
                // TODO add a method to Contact that returns the list of
                // servers
                const auto data = contact->Data();

                if (false == bool(data)) { continue; }

                const auto serverGroup = data->Group(
                    proto::CONTACTSECTION_COMMUNICATION,
                    proto::CITEMTYPE_OPENTXS);

                if (false == bool(serverGroup)) {

                    const auto taskID{++next_task_id_};
                    missing_nyms_.Push(taskID, nymID);
                    continue;
                }

                for (const auto& [claimID, item] : *serverGroup) {
                    SHUTDOWN()
                    OT_ASSERT(item)

                    const auto& notUsed [[maybe_unused]] = claimID;
                    const OTIdentifier serverID =
                        Identifier::Factory(item->Value());

                    if (serverID->empty()) { continue; }

                    LogVerbose(OT_METHOD)(__FUNCTION__)(": Will download nym ")(
                        nymID)(" from server ")(serverID)
                        .Flush();
                    auto& serverQueue = get_nym_fetch(serverID);
                    const auto taskID{++next_task_id_};
                    serverQueue.Push(taskID, nymID);
                }
            } else {
                LogVerbose(OT_METHOD)(__FUNCTION__)(
                    ": No need to update this nym.")
                    .Flush();
            }
        }
    }
}

bool OTX::register_account(
    const TaskID taskID,
    api::client::internal::Operation& op,
    const Identifier& unitID,
    const std::string& label) const
{
    OT_ASSERT(false == unitID.empty())

    auto contract = client_.Wallet().UnitDefinition(unitID);

    if (false == bool(contract)) {
        DO_OPERATION(DownloadContract, unitID, ContractType::UNIT);

        if (false == success) {
            return finish_task(taskID, success, std::move(result));
        }
    }

    ServerContext::ExtraArgs args{label, false};

    // TODO unit id type
    DO_OPERATION(
        Start,
        internal::Operation::Type::RegisterAccount,
        identifier::UnitDefinition::Factory(unitID.str()),
        {label, false});

    finish_task(taskID, success, std::move(result));

    if (success) { client_.Pair().Update(); }

    return success;
}

bool OTX::register_nym(
    const TaskID taskID,
    api::client::internal::Operation& op,
    const bool resync) const
{
    ServerContext::ExtraArgs args{};

    if (resync) { std::get<1>(args) = true; }

    DO_OPERATION(Start, internal::Operation::Type::RegisterNym, args);

    return finish_task(taskID, success, std::move(result));
}

OTX::BackgroundTask OTX::RegisterAccount(
    const identifier::Nym& localNymID,
    const Identifier& serverID,
    const Identifier& unitID,
    const std::string& label) const
{
    return schedule_register_account(localNymID, serverID, unitID, label);
}

OTX::BackgroundTask OTX::RegisterNym(
    const identifier::Nym& localNymID,
    const Identifier& serverID,
    const bool resync) const
{
    CHECK_SERVER(localNymID, serverID)

    start_introduction_server(localNymID);
    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(taskID, queue.register_nym_.Push(taskID, resync));
}

OTX::BackgroundTask OTX::RegisterNymPublic(
    const identifier::Nym& nymID,
    const Identifier& serverID,
    const bool setContactData,
    const bool forcePrimary,
    const bool resync) const
{
    CHECK_SERVER(nymID, serverID)

    start_introduction_server(nymID);

    if (setContactData) {
        publish_server_registration(nymID, serverID, forcePrimary);
    }

    return RegisterNym(nymID, serverID, resync);
}

OTIdentifier OTX::SetIntroductionServer(const ServerContract& contract) const
{
    Lock lock(introduction_server_lock_);

    return set_introduction_server(lock, contract);
}

OTX::BackgroundTask OTX::schedule_download_nymbox(
    const identifier::Nym& localNymID,
    const Identifier& serverID) const
{
    CHECK_SERVER(localNymID, serverID)

    start_introduction_server(localNymID);
    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(taskID, queue.download_nymbox_.Push(taskID, true));
}

OTX::BackgroundTask OTX::schedule_register_account(
    const identifier::Nym& localNymID,
    const Identifier& serverID,
    const Identifier& unitID,
    const std::string& label) const
{
    CHECK_ARGS(localNymID, serverID, unitID)

    start_introduction_server(localNymID);
    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(
        taskID, queue.register_account_.Push(taskID, {unitID, label}));
}

bool OTX::send_transfer(
    const TaskID taskID,
    api::client::internal::Operation& op,
    const Identifier& sourceAccountID,
    const Identifier& targetAccountID,
    const Amount value,
    const std::string& memo) const
{
    DO_OPERATION(
        SendTransfer,
        sourceAccountID,
        targetAccountID,
        value,
        String::Factory(memo));

    return finish_task(taskID, success, std::move(result));
}

OTX::BackgroundTask OTX::SendCheque(
    const identifier::Nym& localNymID,
    const Identifier& sourceAccountID,
    const Identifier& recipientContactID,
    const Amount value,
    const std::string& memo,
    const Time validFrom,
    const Time validTo) const
{
    CHECK_ARGS(localNymID, sourceAccountID, recipientContactID)

    start_introduction_server(localNymID);
    auto serverID = Identifier::Factory();
    auto recipientNymID = identifier::Nym::Factory();
    const auto canMessage =
        can_message(localNymID, recipientContactID, recipientNymID, serverID);

    if (Messagability::READY != canMessage) { return error_task(); }

    if (0 >= value) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid amount.").Flush();

        return error_task();
    }

    auto account = client_.Wallet().Account(sourceAccountID);

    if (false == bool(account)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid account.").Flush();

        return error_task();
    }

    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(
        taskID,
        queue.send_cheque_.Push(
            taskID,
            {sourceAccountID,
             recipientNymID,
             value,
             memo,
             validFrom,
             validTo}));
}

OTX::BackgroundTask OTX::SendExternalTransfer(
    const identifier::Nym& localNymID,
    const Identifier& serverID,
    const Identifier& sourceAccountID,
    const Identifier& targetAccountID,
    const std::int64_t value,
    const std::string& memo) const
{
    CHECK_ARGS(localNymID, serverID, targetAccountID)
    CHECK_NYM(sourceAccountID)

    auto sourceAccount = client_.Wallet().Account(sourceAccountID);

    if (false == bool(sourceAccount)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid source account.").Flush();

        return error_task();
    }

    if (sourceAccount.get().GetNymID().operator!=(localNymID)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong owner on source account.")
            .Flush();

        return error_task();
    }

    if (sourceAccount.get().GetRealNotaryID() != serverID) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong notary on source account.")
            .Flush();

        return error_task();
    }

    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(
        taskID,
        queue.send_transfer_.Push(
            taskID,
            {identifier::Nym::Factory(sourceAccountID.str()),
             targetAccountID,
             value,
             memo}));
}

OTX::BackgroundTask OTX::SendTransfer(
    const identifier::Nym& localNymID,
    const Identifier& serverID,
    const Identifier& sourceAccountID,
    const Identifier& targetAccountID,
    const std::int64_t value,
    const std::string& memo) const
{
    CHECK_ARGS(localNymID, serverID, targetAccountID)
    CHECK_NYM(sourceAccountID)

    auto sourceAccount = client_.Wallet().Account(sourceAccountID);

    if (false == bool(sourceAccount)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid source account.").Flush();

        return error_task();
    }

    if (sourceAccount.get().GetNymID().operator!=(localNymID)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong owner on source account.")
            .Flush();

        return error_task();
    }

    if (sourceAccount.get().GetRealNotaryID() != serverID) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong notary on source account.")
            .Flush();

        return error_task();
    }

    auto& queue = get_operations({localNymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(
        taskID,
        queue.send_transfer_.Push(
            taskID,
            {identifier::Nym::Factory(sourceAccountID.str()),
             targetAccountID,
             value,
             memo}));
}

void OTX::set_contact(const identifier::Nym& nymID, const Identifier& serverID)
    const
{
    auto nym = client_.Wallet().mutable_Nym(nymID);
    const auto server = nym.PreferredOTServer();

    if (server.empty()) { nym.AddPreferredOTServer(serverID.str(), true); }
}

OTIdentifier OTX::set_introduction_server(
    const Lock& lock,
    const ServerContract& contract) const
{
    OT_ASSERT(verify_lock(lock, introduction_server_lock_));

    auto instantiated = client_.Wallet().Server(contract.PublicContract());

    if (false == bool(instantiated)) { return Identifier::Factory(); }

    const auto& id = instantiated->ID();
    introduction_server_id_.reset(new OTIdentifier(id));

    OT_ASSERT(introduction_server_id_)

    bool dontCare = false;
    const bool set = client_.Config().Set_str(
        String::Factory(MASTER_SECTION),
        String::Factory(INTRODUCTION_SERVER_KEY),
        String::Factory(id),
        dontCare);

    OT_ASSERT(set)

    client_.Config().Save();

    return id;
}

void OTX::start_introduction_server(const identifier::Nym& nymID) const
{
    auto& serverID = IntroductionServer();

    if (serverID.empty()) { return; }

    auto& queue = get_operations({nymID, serverID});
    const auto taskID{++next_task_id_};
    start_task(taskID, queue.download_nymbox_.Push(taskID, true));
}

OTX::BackgroundTask OTX::start_task(const TaskID taskID, bool success) const
{
    if (0 == taskID) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Empty task ID").Flush();

        return error_task();
    }

    if (false == success) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Failed to start task").Flush();

        return error_task();
    }

    return add_task(taskID, ThreadStatus::RUNNING);
}

void OTX::StartIntroductionServer(const identifier::Nym& localNymID) const
{
    start_introduction_server(localNymID);
}

void OTX::state_machine(const ContextID id, OperationQueue& queue) const
{
    struct Cleanup {
        Lock& lock_;
        OperationQueue& queue_;

        Cleanup(Lock& lock, OperationQueue& queue)
            : lock_(lock)
            , queue_(queue)
        {
        }

        ~Cleanup() { queue_.cleanup(lock_); }
    };

    auto lock = queue.lock();
    Cleanup cleanup(lock, queue);
    const auto& [nymID, serverID] = id;

    // Make sure the server contract is available
    while (running_) {
        if (check_server_contract(serverID)) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Server contract ")(serverID)(
                " exists.")
                .Flush();

            break;
        }

        YIELD(CONTRACT_DOWNLOAD_MILLISECONDS);
    }

    SHUTDOWN()

    std::shared_ptr<const ServerContext> context{nullptr};

    // Make sure the nym has registered for the first time on the server
    while (running_) {
        if (check_registration(
                identifier::Nym::Factory(nymID->str()),
                serverID,
                *queue.op_,
                context)) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Nym ")(nymID)(
                " has registered on server ")(serverID)(" at least once.")
                .Flush();

            break;
        }

        YIELD(NYM_REGISTRATION_MILLISECONDS);
    }

    SHUTDOWN()
    OT_ASSERT(context)

    bool run{true};
    bool resync{false};
    bool needAdmin{false};
    bool registerNymQueued{false};
    bool downloadNymbox{false};
    bool getTransactionNumbers{false};
    TaskID taskID{0};
    auto accountID = Identifier::Factory();
    auto contractID = Identifier::Factory();
    auto targetNymID = identifier::Nym::Factory();
    auto targetServerID = identifier::Server::Factory();
    auto nullID = Identifier::Factory();
    auto unitID = identifier::UnitDefinition::Factory();
    OTPassword serverPassword;
    MessageTask message{identifier::Nym::Factory(), "", {}};
    PaymentTask payment{identifier::Nym::Factory(), {}};
    SendChequeTask sendCheque{Identifier::Factory(),
                              identifier::Nym::Factory(),
                              0,
                              "",
                              Clock::now(),
                              Clock::now()};
    IssueUnitDefinitionTask issueUnit{Identifier::Factory(), ""};
    RegisterAccountTask registerAccount{Identifier::Factory(), ""};
#if OT_CASH
    PayCashTask cash_payment{identifier::Nym::Factory(), Identifier::Factory()};
    WithdrawCashTask cash_withdrawal{Identifier::Factory(), {}};
#endif  // OT_CASH
    DepositPaymentTask deposit{Identifier::Factory(), {}};
    UniqueQueue<DepositPaymentTask> depositPaymentRetry;
    SendTransferTask transfer{
        identifier::Nym::Factory(), Identifier::Factory(), {}, {}};
#if OT_CASH
    WithdrawCashTask withdrawCash{accountID, 0};
#endif
    PeerReplyTask peerReply{identifier::Nym::Factory(), {}, {}};
    PeerRequestTask peerRequest{identifier::Nym::Factory(), {}};

    // Primary loop
    while (run) {
        SHUTDOWN()

        // If the local nym has updated since the last registernym operation,
        // schedule a registernym
        check_nym_revision(*context, queue);

        SHUTDOWN()

        // Register the nym, if scheduled. Keep trying until success
        registerNymQueued = queue.register_nym_.Pop(taskID, resync);

        if (registerNymQueued) {
            if (false == register_nym(taskID, *queue.op_, resync)) {
                queue.register_nym_.Push(++next_task_id_, resync);
            }
        }

        SHUTDOWN()

        // If this server was added by a pairing operation that included
        // a server password then request admin permissions on the server
        const auto haveAdmin = context->isAdmin();
        needAdmin = context->HaveAdminPassword() && (false == haveAdmin);

        if (needAdmin) {
            serverPassword.setPassword(context->AdminPassword());
            get_admin(++next_task_id_, *queue.op_, serverPassword);
        }

        SHUTDOWN()

        if (haveAdmin) { check_server_name(queue, *context); }

        SHUTDOWN()

        if (0 == queue.counter() % 100) {
            // download server nym in case it has been renamed
            // TODO convert nym id type
            queue.check_nym_.Push(
                ++next_task_id_,
                identifier::Nym::Factory(context->RemoteNym().ID().str()));
        }

        SHUTDOWN()

        // This is a list of servers for which we do not have a contract.
        // We ask all known servers on which we are registered to try to find
        // the contracts.
        const auto servers = missing_servers_.Copy();

        for (const auto& [targetID, taskID] : servers) {
            SHUTDOWN()

            if (targetID->empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": How did an empty serverID get in here?")
                    .Flush();

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(
                    ": Searching for server contract for ")(targetID)
                    .Flush();
            }

            const auto& notUsed [[maybe_unused]] = taskID;
            find_server(*queue.op_, identifier::Nym::Factory(targetID->str()));
        }

        // This is a list of contracts (server and unit definition) which a
        // user of this class has requested we download from this server.
        while (queue.download_contract_.Pop(taskID, contractID)) {
            SHUTDOWN()

            if (contractID->empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": How did an empty contract ID get in here?")
                    .Flush();

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(
                    ": Searching for unit definition contract for ")(contractID)
                    .Flush();
            }

            download_contract(taskID, *queue.op_, contractID);
        }

        // This is a list of nyms for which we do not have credentials..
        // We ask all known servers on which we are registered to try to find
        // their credentials.
        const auto nyms = missing_nyms_.Copy();

        for (const auto& [targetID, taskID] : nyms) {
            SHUTDOWN()

            if (targetID->empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": How did an empty nymID get in here?")
                    .Flush();

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(": Searching for nym ")(
                    targetID)
                    .Flush();
            }

            [[maybe_unused]] const auto& notUsed = taskID;
            find_nym(*queue.op_, targetID);
        }

        // This is a list of nyms which haven't been updated in a while and
        // are known or suspected to be available on this server
        auto& nymQueue = get_nym_fetch(serverID);

        while (nymQueue.Pop(taskID, targetNymID)) {
            SHUTDOWN()

            if (targetNymID->empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": How did an empty nymID get in here?")
                    .Flush();

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(": Refreshing nym ")(
                    targetNymID)
                    .Flush();
            }

            download_nym(taskID, *queue.op_, targetNymID);
        }

        // This is a list of nyms which a user of this class has requested we
        // download from this server.
        while (queue.check_nym_.Pop(taskID, targetNymID)) {
            SHUTDOWN()

            if (targetNymID->empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": How did an empty nymID get in here?")
                    .Flush();

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(": Searching for nym ")(
                    targetNymID)
                    .Flush();
            }

            download_nym(taskID, *queue.op_, targetNymID);
        }

        // This is a list of messages which need to be delivered to a nym
        // on this server
        while (queue.send_message_.Pop(taskID, message)) {
            SHUTDOWN()

            message_nym(taskID, *queue.op_, message);
        }

        while (queue.peer_reply_.Pop(taskID, peerReply)) {
            SHUTDOWN()

            initiate_peer_reply(taskID, *queue.op_, peerReply);
        }

        while (queue.peer_request_.Pop(taskID, peerRequest)) {
            SHUTDOWN()

            initiate_peer_request(taskID, *queue.op_, peerRequest);
        }

        SHUTDOWN();

        // Download the nymbox, if this operation has been scheduled
        if (queue.download_nymbox_.Pop(taskID, downloadNymbox)) {
            LogDetail(OT_METHOD)(__FUNCTION__)(": Downloading nymbox for ")(
                nymID)(" on ")(serverID)
                .Flush();
            if (false == download_nymbox(taskID, *queue.op_)) {
                queue.register_nym_.Push(++next_task_id_, resync);
            }
        }

        SHUTDOWN()

        // Get transaction numbers, if this operation has been scheduled
        if (queue.get_transaction_numbers_.Pop(taskID, getTransactionNumbers)) {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Getting transaction numbers for ")(nymID)(" on ")(serverID)
                .Flush();
            get_transaction_numbers(taskID, *queue.op_);
        }

        SHUTDOWN()

        // This is a list of cheques which need to be written and delivered to
        // a nym on this server
        while (queue.send_cheque_.Pop(taskID, sendCheque)) {
            SHUTDOWN()

            auto& [accountID, recipientID, amount, memo, start, end] =
                sendCheque;

            if (accountID->empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": How did an empty account ID get in here?")
                    .Flush();

                continue;
            }

            if (recipientID->empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": How did an empty recipient nymID get in here?")
                    .Flush();

                continue;
            }

            auto task_done = write_and_send_cheque(
                taskID,
                *queue.op_,
                accountID,
                recipientID,
                amount,
                memo,
                start,
                end);

            if (TaskDone::retry == task_done) {
                const auto numbersTaskID{++next_task_id_};
                start_task(
                    numbersTaskID,
                    queue.get_transaction_numbers_.Push(numbersTaskID, true));

                queue.send_cheque_.Push(taskID, sendCheque);
                break;
            }
        }

        SHUTDOWN()

        // This is a list of payments which need to be delivered to a nym
        // on this server
        while (queue.send_payment_.Pop(taskID, payment)) {
            SHUTDOWN()

            auto& [recipientID, pPayment] = payment;

            if (recipientID->empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": How did an empty recipient nymID get in here?")
                    .Flush();

                continue;
            }

            pay_nym(taskID, *queue.op_, recipientID, pPayment);
        }

        SHUTDOWN()

#if OT_CASH
        while (queue.download_mint_.Pop(taskID, unitID)) {
            SHUTDOWN()

            LogDetail(OT_METHOD)(__FUNCTION__)(": Downloading mint for ")(
                unitID)
                .Flush();

            download_mint(taskID, *queue.op_, unitID);
        }

        while (queue.withdraw_cash_.Pop(taskID, cash_withdrawal)) {
            SHUTDOWN()

            withdraw_cash(taskID, *queue.op_, cash_withdrawal);
        }

        // This is a list of cash payments which need to be delivered to a nym
        // on this server
        while (queue.send_cash_.Pop(taskID, cash_payment)) {
            SHUTDOWN()

            auto& [recipientID, workflowID] = cash_payment;

            if (recipientID->empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": How did an empty recipient nymID get in here?")
                    .Flush();

                continue;
            }

            pay_nym_cash(taskID, *queue.op_, recipientID, workflowID);
        }
#endif

        SHUTDOWN()

        // Register any accounts which have been scheduled for creation
        while (queue.register_account_.Pop(taskID, registerAccount)) {
            SHUTDOWN()

            const auto& [unitID, label] = registerAccount;

            if (unitID->empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": How did an empty unit ID get in here?")
                    .Flush();

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(": Creating account for ")(
                    unitID)(" on ")(serverID)
                    .Flush();
            }

            if (false == register_account(taskID, *queue.op_, unitID, label)) {
                queue.register_nym_.Push(++next_task_id_, resync);
            }
        }

        SHUTDOWN()

        // Issue unit definitions which have been scheduled
        while (queue.issue_unit_definition_.Pop(taskID, issueUnit)) {
            SHUTDOWN()

            const auto& [unitID, label] = issueUnit;

            if (unitID->empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": How did an empty unit ID get in here?")
                    .Flush();

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(
                    ": Issuing unit definition for ")(unitID)(" on ")(serverID)
                    .Flush();
            }

            if (false ==
                issue_unit_definition(taskID, *queue.op_, unitID, label)) {
                queue.register_nym_.Push(++next_task_id_, resync);
            }
        }

        SHUTDOWN()

        // Deposit any queued payments
        while (queue.deposit_payment_.Pop(taskID, deposit)) {
            auto& [accountIDHint, payment] = deposit;

            SHUTDOWN()
            OT_ASSERT(payment)

            const auto status =
                can_deposit(*payment, nymID, accountIDHint, nullID, accountID);

            switch (status) {
                case Depositability::READY: {
                    if (false == deposit_cheque(
                                     taskID,
                                     *queue.op_,
                                     accountID,
                                     payment,
                                     depositPaymentRetry)) {
                        queue.register_nym_.Push(++next_task_id_, resync);
                    }
                } break;
                case Depositability::NOT_REGISTERED:
                case Depositability::NO_ACCOUNT: {
                    LogDetail(OT_METHOD)(__FUNCTION__)(
                        ": Temporary failure trying to deposit payment")
                        .Flush();
                    depositPaymentRetry.Push(taskID, deposit);
                } break;
                default: {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Permanent failure trying to deposit payment.")
                        .Flush();
                }
            }
        }

        // Requeue all payments which will be retried
        while (depositPaymentRetry.Pop(taskID, deposit)) {
            SHUTDOWN()

            queue.deposit_payment_.Push(taskID, deposit);
        }

        SHUTDOWN()

        // This is a list of transfers which need to be delivered to a nym
        // on this server
        while (queue.send_transfer_.Pop(taskID, transfer)) {
            SHUTDOWN()

            const auto& [sourceAccountID, targetAccountID, value, memo] =
                transfer;
            send_transfer(
                taskID,
                *queue.op_,
                sourceAccountID,
                targetAccountID,
                value,
                memo);
        }

        while (queue.publish_server_contract_.Pop(taskID, targetServerID)) {
            SHUTDOWN()

            if (contractID->empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Empty targetServerID?")
                    .Flush();

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(
                    ": Uploading server contract ")(targetServerID)
                    .Flush();
            }

            publish_server_contract(taskID, *queue.op_, targetServerID);
        }

        while (queue.process_inbox_.Pop(taskID, accountID)) {
            SHUTDOWN()

            if (accountID->empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": How did an empty account ID get in here?")
                    .Flush();

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(": Processing inbox ")(
                    accountID)
                    .Flush();
            }

            process_inbox(taskID, *queue.op_, accountID);
        }

        const bool missing =
            !(missing_nyms_.empty() && missing_servers_.empty());
        queue.increment_counter(missing, lock, run);
    }
}

ThreadStatus OTX::status(const Lock& lock, const TaskID taskID) const
{
    OT_ASSERT(verify_lock(lock, task_status_lock_))

    if (!running_) { return ThreadStatus::SHUTDOWN; }

    auto it = task_status_.find(taskID);

    if (task_status_.end() == it) { return ThreadStatus::ERROR; }

    const auto output = it->second.first;
    const bool success = (ThreadStatus::FINISHED_SUCCESS == output);
    const bool failed = (ThreadStatus::FINISHED_FAILED == output);
    const bool finished = (success || failed);

    if (finished) { task_status_.erase(it); }

    return output;
}

ThreadStatus OTX::Status(const TaskID taskID) const
{
    Lock lock(task_status_lock_);

    return status(lock, taskID);
}

void OTX::update_task(
    const TaskID taskID,
    const ThreadStatus status,
    Result&& result) const
{
    if (0 == taskID) { return; }

    Lock lock(task_status_lock_);

    if (0 == task_status_.count(taskID)) { return; }

    auto& row = task_status_.at(taskID);
    auto& [state, promise] = row;
    state = status;
    bool value{false};
    bool publish{false};

    switch (status) {
        case ThreadStatus::FINISHED_SUCCESS: {
            value = true;
            publish = true;
            promise.set_value(std::move(result));
        } break;
        case ThreadStatus::FINISHED_FAILED: {
            value = false;
            publish = true;
            promise.set_value(std::move(result));
        } break;
        case ThreadStatus::SHUTDOWN: {
            Result cancel{proto::LASTREPLYSTATUS_UNKNOWN, nullptr};
            promise.set_value(std::move(cancel));
        }
        case ThreadStatus::ERROR:
        case ThreadStatus::RUNNING:
        default: {
        }
    }

    if (publish) {
        auto message = zmq::Message::Factory();
        message->AddFrame();
        message->AddFrame(std::to_string(taskID));
        message->AddFrame(Data::Factory(&value, sizeof(value)));
        task_finished_->Publish(message);
    }
}

Depositability OTX::valid_account(
    const OTPayment& payment,
    const Identifier& recipient,
    const Identifier& paymentServerID,
    const Identifier& paymentUnitID,
    const Identifier& accountIDHint,
    OTIdentifier& depositAccount) const
{
    std::set<OTIdentifier> matchingAccounts{};

    for (const auto& it : client_.Storage().AccountList()) {
        const auto accountID = Identifier::Factory(it.first);
        const auto nymID = client_.Storage().AccountOwner(accountID);
        const auto serverID = client_.Storage().AccountServer(accountID);
        const auto unitID = client_.Storage().AccountContract(accountID);

        if (nymID != recipient) { continue; }

        if (serverID != paymentServerID) { continue; }

        if (unitID != paymentUnitID) { continue; }

        matchingAccounts.emplace(accountID);
    }

    if (accountIDHint.empty()) {
        if (0 == matchingAccounts.size()) {

            return Depositability::NO_ACCOUNT;
        } else if (1 == matchingAccounts.size()) {
            depositAccount = *matchingAccounts.begin();

            return Depositability::READY;
        } else {

            return Depositability::ACCOUNT_NOT_SPECIFIED;
        }
    }

    if (0 == matchingAccounts.size()) {

        return Depositability::NO_ACCOUNT;
    } else if (1 == matchingAccounts.count(accountIDHint)) {
        depositAccount = accountIDHint;

        return Depositability::READY;
    } else {

        return Depositability::WRONG_ACCOUNT;
    }
}

bool OTX::valid_context(
    const identifier::Nym& nymID,
    const Identifier& serverID) const
{
    const auto nyms = client_.Wallet().LocalNyms();

    if (0 == nyms.count(nymID)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Nym ")(nymID)(
            " does not belong to this wallet.")
            .Flush();

        return false;
    }

    if (serverID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid server.").Flush();

        return false;
    }

    const auto context = client_.Wallet().ServerContext(nymID, serverID);

    if (false == bool(context)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Context does not exist.").Flush();

        return false;
    }

    if (0 == context->Request()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Nym is not registered at this server.")
            .Flush();

        return false;
    }

    return true;
}

Depositability OTX::valid_recipient(
    const OTPayment& payment,
    const identifier::Nym& specified,
    const Identifier& recipient) const
{
    if (specified.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Payment can be accepted by any nym.")
            .Flush();

        return Depositability::READY;
    }

    if (recipient == specified) { return Depositability::READY; }

    return Depositability::WRONG_RECIPIENT;
}

#if OT_CASH
bool OTX::withdraw_cash(
    const TaskID taskID,
    api::client::internal::Operation& op,
    const WithdrawCashTask& task) const
{
    const auto& [accountID, amount] = task;

    DO_OPERATION(WithdrawCash, accountID, amount);

    return finish_task(taskID, success, std::move(result));
}

OTX::BackgroundTask OTX::WithdrawCash(
    const identifier::Nym& nymID,
    const identifier::Server& serverID,
    const Identifier& account,
    const Amount amount) const
{
    CHECK_ARGS(nymID, serverID, account)

    start_introduction_server(nymID);
    auto& queue = get_operations({nymID, serverID});
    const auto taskID{++next_task_id_};

    return start_task(
        taskID, queue.withdraw_cash_.Push(taskID, {account, amount}));
}
#endif

OTX::TaskDone OTX::write_and_send_cheque(
    const TaskID taskID,
    api::client::internal::Operation& op,
    const Identifier& accountID,
    const identifier::Nym& recipient,
    const Amount value,
    const std::string& memo,
    const Time validFrom,
    const Time validTo) const
{
    OT_ASSERT(false == accountID.empty())
    OT_ASSERT(false == recipient.empty())

    if (0 >= value) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid amount.").Flush();

        return task_done(finish_task(taskID, false, error_result()));
    }

    auto context = client_.Wallet().ServerContext(op.NymID(), op.ServerID());

    OT_ASSERT(context);

    const auto available = context->AvailableNumbers();

    if (0 == available) { return TaskDone::retry; }

    std::unique_ptr<Cheque> cheque(client_.OTAPI().WriteCheque(
        op.ServerID(),
        value,
        Clock::to_time_t(validFrom),
        Clock::to_time_t(validTo),
        accountID,
        op.NymID(),
        String::Factory(memo.c_str()),
        recipient));

    if (false == bool(cheque)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to write cheque.").Flush();

        return task_done(finish_task(taskID, false, error_result()));
    }

    std::shared_ptr<OTPayment> payment{
        client_.Factory().Payment(String::Factory(*cheque))};

    if (false == bool(payment)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate payment.")
            .Flush();

        return task_done(finish_task(taskID, false, error_result()));
    }

    if (false == payment->SetTempValues()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid payment.").Flush();

        return task_done(finish_task(taskID, false, error_result()));
    }

    DO_OPERATION_TASK_DONE(ConveyPayment, recipient, payment);

    return task_done(finish_task(taskID, success, std::move(result)));
}

OTX::~OTX()
{
    for (auto& [id, thread] : state_machines_) {
        const auto& notUsed [[maybe_unused]] = id;

        OT_ASSERT(thread)

        if (thread->joinable()) { thread->join(); }
    }

    state_machines_.clear();

    for (auto& it : task_status_) {
        auto& promise = it.second.second;

        try {
            promise.set_value(error_result());
        } catch (...) {
        }
    }
}
}  // namespace opentxs::api::client::implementation

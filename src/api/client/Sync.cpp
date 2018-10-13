// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/Pair.hpp"
#include "opentxs/api/client/Sync.hpp"
#include "opentxs/api/client/ServerAction.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/Wallet.hpp"
#if OT_CASH
#include "opentxs/cash/Purse.hpp"
#endif  // OT_CASH
#include "opentxs/client/NymData.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/OTClient.hpp"
#include "opentxs/client/OTWallet.hpp"
#include "opentxs/client/ServerAction.hpp"
#include "opentxs/client/Utility.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
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

#include <atomic>
#include <chrono>
#include <memory>
#include <map>
#include <thread>
#include <tuple>

#include "Sync.hpp"

#define CONTACT_REFRESH_DAYS 1
#define CONTRACT_DOWNLOAD_MILLISECONDS 10000
#define MAIN_LOOP_MILLISECONDS 5000
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
            otErr << OT_METHOD << __FUNCTION__ << ": Invalid " << #a           \
                  << std::endl;                                                \
                                                                               \
            return Identifier::Factory();                                      \
        }                                                                      \
    }

#define CHECK_SERVER(a, b)                                                     \
    {                                                                          \
        CHECK_NYM(a)                                                           \
                                                                               \
        if (b.empty()) {                                                       \
            otErr << OT_METHOD << __FUNCTION__ << ": Invalid " << #b           \
                  << std::endl;                                                \
                                                                               \
            return Identifier::Factory();                                      \
        }                                                                      \
    }

#define CHECK_ARGS(a, b, c)                                                    \
    {                                                                          \
        CHECK_SERVER(a, b)                                                     \
                                                                               \
        if (c.empty()) {                                                       \
            otErr << OT_METHOD << __FUNCTION__ << ": Invalid " << #c           \
                  << std::endl;                                                \
                                                                               \
            return Identifier::Factory();                                      \
        }                                                                      \
    }

#define INTRODUCTION_SERVER_KEY "introduction_server_id"
#define MASTER_SECTION "Master"
#define PROCESS_INBOX_RETRIES 3

#define OT_METHOD "opentxs::api::client::implementation::Sync::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs
{
api::client::Sync* Factory::Sync(
    const Flag& running,
    const api::client::Manager& client,
    OTClient& otclient,
    const ContextLockCallback& lockCallback)
{
    return new api::client::implementation::Sync(
        running, client, otclient, lockCallback);
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
const std::string Sync::DEFAULT_INTRODUCTION_SERVER =
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

Sync::Sync(
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

std::pair<bool, std::size_t> Sync::accept_incoming(
    const rLock& lock[[maybe_unused]],
    const std::size_t max,
    const Identifier& accountID,
    ServerContext& context) const
{
    class Cleanup
    {
    public:
        void SetSuccess() { recover_ = false; }

        Cleanup(const TransactionNumber number, ServerContext& context)
            : context_(context)
            , number_(number)
        {
        }
        Cleanup() = delete;
        ~Cleanup()
        {
            if (recover_ && (0 != number_)) {
                otErr << OT_METHOD << "Cleanup::" << __FUNCTION__
                      << ": Recovering unused number " << number_ << std::endl;
                const bool recovered = context_.RecoverAvailableNumber(number_);

                if (false == recovered) { otErr << "Failed" << std::endl; }
            }
        }

    private:
        ServerContext& context_;
        const TransactionNumber number_{0};
        bool recover_{true};
    };

    std::pair<bool, std::size_t> output{false, 0};
    auto& [success, remaining] = output;
    const std::string account = accountID.str();
    auto processInbox = client_.OTAPI().CreateProcessInbox(accountID, context);
    auto& response = std::get<0>(processInbox);
    auto& inbox = std::get<1>(processInbox);
    auto& recoverNumber = std::get<2>(processInbox);

    if (false == bool(response)) {
        if (nullptr == inbox) {
            // This is a new account which has never instantiated an inbox.
            success = true;

            return output;
        }

        otErr << OT_METHOD << __FUNCTION__
              << ": Error instantiating processInbox for account: " << account
              << std::endl;

        return output;
    }

    Cleanup cleanup(recoverNumber, context);
    const std::size_t items =
        (inbox->GetTransactionCount() >= 0) ? inbox->GetTransactionCount() : 0;
    const std::size_t count = (items > max) ? max : items;
    remaining = items - count;

    if (0 == count) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": No items to accept in this account.")
            .Flush();
        success = true;

        return output;
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Items to accept in this account: ")(count)
            .Flush();
    }

    for (std::size_t i = 0; i < count; i++) {
        auto transaction = inbox->GetTransactionByIndex(i);

        OT_ASSERT(nullptr != transaction);

        const TransactionNumber number = transaction->GetTransactionNum();

        if (transaction->IsAbbreviated()) {
            inbox->LoadBoxReceipt(number);
            transaction = inbox->GetTransaction(number);

            if (nullptr == transaction) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Unable to load item: " << number << std::endl;

                continue;
            }
        }

        if (transactionType::chequeReceipt == transaction->GetType()) {
            const auto workflowUpdated = client_.Workflow().ClearCheque(
                context.Nym()->ID(), *transaction);

            if (workflowUpdated) {
                otErr << OT_METHOD << __FUNCTION__ << ": Updated workflow."
                      << std::endl;
            } else {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Failed to update workflow." << std::endl;
            }
        }

        const bool accepted = client_.OTAPI().IncludeResponse(
            accountID, true, context, *transaction, *response);

        if (!accepted) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed to accept item: " << number << std::endl;

            return output;
        }
    }

    const bool finalized = client_.OTAPI().FinalizeProcessInbox(
        accountID, context, *response, *inbox);

    if (false == finalized) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to finalize response."
              << std::endl;

        return output;
    }

    auto action = client_.ServerAction().ProcessInbox(
        context.Nym()->ID(), context.Server(), accountID, response);
    action->Run();
    success = (SendResult::VALID_REPLY == action->LastSendResult());

    if (success) { cleanup.SetSuccess(); }

    return output;
}

bool Sync::AcceptIncoming(
    const Identifier& nymID,
    const Identifier& accountID,
    const Identifier& serverID,
    const std::size_t max) const
{
    rLock apiLock(lock_callback_({nymID.str(), serverID.str()}));
    auto context = client_.Wallet().mutable_ServerContext(nymID, serverID);
    std::size_t remaining{1};
    std::size_t retries{PROCESS_INBOX_RETRIES};

    while (0 < remaining) {
        const auto attempt =
            accept_incoming(apiLock, max, accountID, context.It());
        const auto& [success, unprocessed] = attempt;
        remaining = unprocessed;

        if (false == success) {
            if (0 == retries) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Exceeded maximum retries." << std::endl;

                return false;
            }

            Utility utility(context.It(), client_);
            const auto download = utility.getIntermediaryFiles(
                context.It().Server().str(),
                context.It().Nym()->ID().str(),
                accountID.str(),
                true);

            if (false == download) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Failed to download account files." << std::endl;

                return false;
            } else {
                --retries;

                continue;
            }
        }

        if (0 != remaining) {
            otErr << OT_METHOD << __FUNCTION__ << ": Accepting " << remaining
                  << " more items." << std::endl;
        }
    }

    return true;
}

void Sync::add_task(const Identifier& taskID, const ThreadStatus status) const
{
    Lock lock(task_status_lock_);

    if (0 != task_status_.count(taskID)) { return; }

    task_status_[taskID] = status;
}

void Sync::associate_message_id(
    const Identifier& messageID,
    const Identifier& taskID) const
{
    Lock lock(task_status_lock_);
    task_message_id_.emplace(taskID, messageID);
}

Depositability Sync::can_deposit(
    const OTPayment& payment,
    const Identifier& recipient,
    const Identifier& accountIDHint,
    OTIdentifier& depositServer,
    OTIdentifier& depositAccount) const
{
    auto unitID = Identifier::Factory();
    auto nymID = Identifier::Factory();

    if (false == extract_payment_data(payment, nymID, depositServer, unitID)) {

        return Depositability::INVALID_INSTRUMENT;
    }

    auto output = valid_recipient(payment, nymID, recipient);

    if (Depositability::READY != output) { return output; }

    const bool registered = client_.Exec().IsNym_RegisteredAtServer(
        recipient.str(), depositServer->str());

    if (false == registered) {
        schedule_download_nymbox(recipient, depositServer);
        otErr << OT_METHOD << __FUNCTION__ << ": Recipient nym "
              << recipient.str() << " not registered on server "
              << depositServer->str() << std::endl;

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
            otErr << OT_METHOD << __FUNCTION__
                  << ": Multiple valid accounts exist. "
                  << "This payment can not be automatically deposited"
                  << std::endl;
        } break;
        case Depositability::WRONG_ACCOUNT: {
            otErr << OT_METHOD << __FUNCTION__
                  << ": The specified account is not valid for this payment."
                  << std::endl;
        } break;
        case Depositability::NO_ACCOUNT: {
            otErr << OT_METHOD << __FUNCTION__ << ": Recipient "
                  << recipient.str() << " needs an account for "
                  << unitID->str() << " on server " << depositServer->str()
                  << std::endl;
            schedule_register_account(recipient, depositServer, unitID);
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

Messagability Sync::can_message(
    const Identifier& senderNymID,
    const Identifier& recipientContactID,
    OTIdentifier& recipientNymID,
    OTIdentifier& serverID) const
{
    auto senderNym = client_.Wallet().Nym(senderNymID);

    if (false == bool(senderNym)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load sender nym "
              << senderNymID.str() << std::endl;

        return Messagability::MISSING_SENDER;
    }

    const bool canSign = senderNym->HasCapability(NymCapability::SIGN_MESSAGE);

    if (false == canSign) {
        otErr << OT_METHOD << __FUNCTION__ << ": Sender nym "
              << senderNymID.str() << " can not sign messages (no private key)."
              << std::endl;

        return Messagability::INVALID_SENDER;
    }

    const auto contact = client_.Contacts().Contact(recipientContactID);

    if (false == bool(contact)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Recipient contact "
              << recipientContactID.str() << " does not exist." << std::endl;

        return Messagability::MISSING_CONTACT;
    }

    const auto nyms = contact->Nyms();

    if (0 == nyms.size()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Recipient contact "
              << recipientContactID.str() << " does not have a nym."
              << std::endl;

        return Messagability::CONTACT_LACKS_NYM;
    }

    std::shared_ptr<const Nym> recipientNym{nullptr};

    for (const auto& it : nyms) {
        recipientNym = client_.Wallet().Nym(it);

        if (recipientNym) {
            recipientNymID = Identifier::Factory(it);
            break;
        }
    }

    if (false == bool(recipientNym)) {
        for (const auto& id : nyms) {
            missing_nyms_.Push(Identifier::Random(), id);
        }

        otErr << OT_METHOD << __FUNCTION__ << ": Recipient contact "
              << recipientContactID.str() << " credentials not available."
              << std::endl;

        return Messagability::MISSING_RECIPIENT;
    }

    OT_ASSERT(recipientNym)

    const auto& claims = recipientNym->Claims();
    serverID = Identifier::Factory(claims.PreferredOTServer());

    // TODO maybe some of the other nyms in this contact do specify a server
    if (serverID->empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Recipient contact "
              << recipientContactID.str() << ", nym " << recipientNymID->str()
              << ": credentials do not specify a server." << std::endl;
        missing_nyms_.Push(Identifier::Random(), recipientNymID);

        return Messagability::NO_SERVER_CLAIM;
    }

    const bool registered = client_.Exec().IsNym_RegisteredAtServer(
        senderNymID.str(), serverID->str());

    if (false == registered) {
        schedule_download_nymbox(senderNymID, serverID);
        otErr << OT_METHOD << __FUNCTION__ << ": Sender nym "
              << senderNymID.str() << " not registered on server "
              << serverID->str() << std::endl;

        return Messagability::UNREGISTERED;
    }

    return Messagability::READY;
}

Depositability Sync::CanDeposit(
    const Identifier& recipientNymID,
    const OTPayment& payment) const
{
    auto accountHint = Identifier::Factory();

    return CanDeposit(recipientNymID, accountHint, payment);
}

Depositability Sync::CanDeposit(
    const Identifier& recipientNymID,
    const Identifier& accountIDHint,
    const OTPayment& payment) const
{
    auto serverID = Identifier::Factory();
    auto accountID = Identifier::Factory();

    return can_deposit(
        payment, recipientNymID, accountIDHint, serverID, accountID);
}

Messagability Sync::CanMessage(
    const Identifier& senderNymID,
    const Identifier& recipientContactID) const
{
    if (senderNymID.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid sender" << std::endl;

        return Messagability::INVALID_SENDER;
    }

    if (recipientContactID.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid recipient"
              << std::endl;

        return Messagability::MISSING_CONTACT;
    }

    auto nymID = Identifier::Factory();
    auto serverID = Identifier::Factory();
    start_introduction_server(senderNymID);

    return can_message(senderNymID, recipientContactID, nymID, serverID);
}

void Sync::check_nym_revision(
    const ServerContext& context,
    OperationQueue& queue) const
{
    if (context.StaleNym()) {
        const auto& nymID = context.Nym()->ID();
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID.str()
              << " has is newer than version last registered version on server "
              << context.Server().str() << std::endl;
        queue.register_nym_.Push(Identifier::Random(), true);
    }
}

bool Sync::check_registration(
    const Identifier& nymID,
    const Identifier& serverID,
    std::shared_ptr<const ServerContext>& context) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())

    context = client_.Wallet().ServerContext(nymID, serverID);
    RequestNumber request{0};

    if (context) {
        request = context->Request();
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID.str()
              << " has never registered on " << serverID.str() << std::endl;
    }

    if (0 != request) {
        OT_ASSERT(context)

        return true;
    }

    const auto output = register_nym(Identifier::Random(), nymID, serverID);

    if (output) {
        context = client_.Wallet().ServerContext(nymID, serverID);

        OT_ASSERT(context)
    }

    return output;
}

bool Sync::check_server_contract(const Identifier& serverID) const
{
    OT_ASSERT(false == serverID.empty())

    const auto serverContract = client_.Wallet().Server(serverID);

    if (serverContract) { return true; }

    otErr << OT_METHOD << __FUNCTION__ << ": Server contract for "
          << serverID.str() << " is not in the wallet." << std::endl;
    missing_servers_.Push(Identifier::Random(), serverID);

    return false;
}

OTIdentifier Sync::check_server_name(const ServerContext& context) const
{
    const auto null = Identifier::Factory();
    const auto& nymID = context.Nym()->ID();
    const auto& serverID = context.Server();
    const auto server = client_.Wallet().Server(serverID);

    OT_ASSERT(server)

    const auto myName = server->Alias();
    const auto hisName = server->EffectiveName();

    if (myName == hisName) { return null; }

    auto action = client_.ServerAction().AddServerClaim(
        nymID,
        serverID,
        proto::CONTACTSECTION_SCOPE,
        proto::CITEMTYPE_SERVER,
        myName,
        true);
    action->Run();

    if (SendResult::VALID_REPLY == action->LastSendResult()) {
        OT_ASSERT(action->Reply());

        if (action->Reply()->m_bSuccess) {

            return Identifier::Factory(context.RemoteNym().ID());
        } else {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed to rename server nym" << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Communication error while renaming server nym on server "
              << serverID.str() << std::endl;
    }

    return null;
}

bool Sync::deposit_cheque(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const std::shared_ptr<const OTPayment>& payment,
    UniqueQueue<DepositPaymentTask>& retry) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == accountID.empty())
    OT_ASSERT(payment)

    if ((false == payment->IsCheque()) && (false == payment->IsVoucher())) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unhandled payment type."
              << std::endl;

        return finish_task(taskID, false);
    }

    auto cheque{client_.Factory().Cheque()};

    OT_ASSERT(false != bool(cheque));

    const auto loaded = cheque->LoadContractFromString(payment->Payment());

    if (false == loaded) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid cheque" << std::endl;

        return finish_task(taskID, false);
    }

    auto action = client_.ServerAction().DepositCheque(
        nymID, serverID, accountID, cheque);
    action->Run();

    if (SendResult::VALID_REPLY == action->LastSendResult()) {
        OT_ASSERT(action->Reply());

        if (action->Reply()->m_bSuccess) {

            return finish_task(taskID, true);
        } else {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed to deposit cheque:\n"
                  << String::Factory(*cheque) << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Communication error while depositing cheque "
              << " on server " << serverID.str() << std::endl;
    }

    retry.Push(taskID, {accountID, payment});

    return false;
}

std::size_t Sync::DepositCheques(const Identifier& nymID) const
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

std::size_t Sync::DepositCheques(
    const Identifier& nymID,
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

OTIdentifier Sync::DepositPayment(
    const Identifier& recipientNymID,
    const std::shared_ptr<const OTPayment>& payment) const
{
    auto notUsed = Identifier::Factory();

    return DepositPayment(recipientNymID, notUsed, payment);
}

OTIdentifier Sync::DepositPayment(
    const Identifier& recipientNymID,
    const Identifier& accountIDHint,
    const std::shared_ptr<const OTPayment>& payment) const
{
    OT_ASSERT(payment)

    if (recipientNymID.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid recipient"
              << std::endl;

        return Identifier::Factory();
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
            const auto taskID(Identifier::Random());

            return start_task(
                taskID,
                queue.deposit_payment_.Push(taskID, {accountIDHint, payment}));
        } break;
        default: {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to queue payment for download" << std::endl;
        }
    }

    return Identifier::Factory();
}

void Sync::DisableAutoaccept() const { auto_process_inbox_->Off(); }

bool Sync::download_account(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& accountID) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == accountID.empty())

    const auto success = client_.ServerAction().DownloadAccount(
        nymID, serverID, accountID, false);

    return finish_task(taskID, success);
}

bool Sync::download_contract(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& contractID) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == contractID.empty())

    auto action =
        client_.ServerAction().DownloadContract(nymID, serverID, contractID);
    action->Run();

    if (SendResult::VALID_REPLY == action->LastSendResult()) {
        OT_ASSERT(action->Reply());

        if (action->Reply()->m_bSuccess) {
            client_.Pair().Update();

            return finish_task(taskID, true);
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Server " << serverID.str()
                  << " does not have the contract " << contractID.str()
                  << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Communication error while downloading contract "
              << contractID.str() << " from server " << serverID.str()
              << std::endl;
    }

    return finish_task(taskID, false);
}

bool Sync::download_nym(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& targetNymID) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == targetNymID.empty())

    auto action =
        client_.ServerAction().DownloadNym(nymID, serverID, targetNymID);
    action->Run();

    if (SendResult::VALID_REPLY == action->LastSendResult()) {
        OT_ASSERT(action->Reply());

        if (action->Reply()->m_bSuccess) {
            client_.Pair().Update();

            return finish_task(taskID, true);
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Server " << serverID.str()
                  << " does not have nym " << targetNymID.str() << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Communication error while downloading nym "
              << targetNymID.str() << " from server " << serverID.str()
              << std::endl;
    }

    return finish_task(taskID, false);
}

bool Sync::download_nymbox(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())

    const auto success = client_.ServerAction().DownloadNymbox(nymID, serverID);

    return finish_task(taskID, success);
}

bool Sync::extract_payment_data(
    const OTPayment& payment,
    OTIdentifier& nymID,
    OTIdentifier& serverID,
    OTIdentifier& unitID) const
{
    if (false == payment.GetRecipientNymID(nymID)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to load recipient nym from instrument" << std::endl;

        return false;
    }

    if (false == payment.GetNotaryID(serverID)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to load recipient nym from instrument" << std::endl;

        return false;
    }

    OT_ASSERT(false == serverID->empty())

    if (false == payment.GetInstrumentDefinitionID(unitID)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to load recipient nym from instrument" << std::endl;

        return false;
    }

    OT_ASSERT(false == unitID->empty())

    return true;
}

bool Sync::find_nym(
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& targetID) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == targetID.empty())

    const auto nym = client_.Wallet().Nym(targetID);

    if (nym) {
        missing_nyms_.CancelByValue(targetID);

        return true;
    }

    if (download_nym(Identifier::Factory(), nymID, serverID, targetID)) {
        missing_nyms_.CancelByValue(targetID);

        return true;
    }

    return false;
}

bool Sync::find_server(
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& targetID) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == targetID.empty())

    const auto serverContract = client_.Wallet().Server(targetID);

    if (serverContract) {
        missing_servers_.CancelByValue(targetID);

        return true;
    }

    if (download_contract(Identifier::Factory(), nymID, serverID, targetID)) {
        missing_servers_.CancelByValue(targetID);

        return true;
    }

    return false;
}

OTIdentifier Sync::FindNym(const Identifier& nymID) const
{
    CHECK_NYM(nymID)

    const auto taskID(Identifier::Random());

    return start_task(taskID, missing_nyms_.Push(taskID, nymID));
}

OTIdentifier Sync::FindNym(
    const Identifier& nymID,
    const Identifier& serverIDHint) const
{
    CHECK_NYM(nymID)

    auto& serverQueue = get_nym_fetch(serverIDHint);
    const auto taskID(Identifier::Random());

    return start_task(taskID, serverQueue.Push(taskID, nymID));
}

OTIdentifier Sync::FindServer(const Identifier& serverID) const
{
    CHECK_NYM(serverID)

    const auto taskID(Identifier::Random());

    return start_task(taskID, missing_servers_.Push(taskID, serverID));
}

bool Sync::finish_task(const Identifier& taskID, const bool success) const
{
    if (success) {
        update_task(taskID, ThreadStatus::FINISHED_SUCCESS);
    } else {
        update_task(taskID, ThreadStatus::FINISHED_FAILED);
    }

    return success;
}

bool Sync::get_admin(
    const Identifier& nymID,
    const Identifier& serverID,
    const OTPassword& password) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())

    bool success{false};

    {
        const std::string serverPassword(password.getPassword());
        auto action = client_.ServerAction().RequestAdmin(
            nymID, serverID, serverPassword);
        action->Run();

        if (SendResult::VALID_REPLY == action->LastSendResult()) {
            auto reply = action->Reply();

            OT_ASSERT(reply)

            success = reply->m_bSuccess;
        }
    }

    auto mContext = client_.Wallet().mutable_ServerContext(nymID, serverID);
    auto& context = mContext.It();
    context.SetAdminAttempted();

    if (success) {
        otErr << OT_METHOD << __FUNCTION__ << ": Got admin on server "
              << serverID.str() << std::endl;
        context.SetAdminSuccess();
    }

    return success;
}

OTIdentifier Sync::get_introduction_server(const Lock& lock) const
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

UniqueQueue<OTIdentifier>& Sync::get_nym_fetch(const Identifier& serverID) const
{
    Lock lock(nym_fetch_lock_);

    return server_nym_fetch_[serverID];
}

Sync::OperationQueue& Sync::get_operations(const ContextID& id) const
{
    Lock lock(lock_);
    auto& queue = operations_[id];
    auto& thread = state_machines_[id];

    if (false == bool(thread)) {
        thread.reset(new std::thread(
            [id, &queue, this]() { state_machine(id, queue); }));
    }

    return queue;
}

OTIdentifier Sync::import_default_introduction_server(const Lock& lock) const
{
    OT_ASSERT(verify_lock(lock, introduction_server_lock_))

    const auto serialized = proto::StringToProto<proto::ServerContract>(
        String::Factory(DEFAULT_INTRODUCTION_SERVER.c_str()));
    const auto instantiated = client_.Wallet().Server(serialized);

    OT_ASSERT(instantiated)

    return set_introduction_server(lock, *instantiated);
}

const Identifier& Sync::IntroductionServer() const
{
    Lock lock(introduction_server_lock_);

    if (false == bool(introduction_server_id_)) {
        load_introduction_server(lock);
    }

    OT_ASSERT(introduction_server_id_)

    return *introduction_server_id_;
}

bool Sync::issue_unit_definition(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& unitID) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == unitID.empty())

    auto unitdefinition = client_.Wallet().UnitDefinition(unitID);
    if (false == bool(unitdefinition)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unit definition not found"
              << std::endl;

        return false;
    }

    auto action = client_.ServerAction().IssueUnitDefinition(
        nymID, serverID, unitdefinition->PublicContract());
    action->Run();

    if (SendResult::VALID_REPLY == action->LastSendResult()) {
        OT_ASSERT(action->Reply());

        if (action->Reply()->m_bSuccess) {
            client_.Pair().Update();

            return finish_task(taskID, true);
        } else {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed to issue unit definition for " << unitID.str()
                  << " on server " << serverID.str() << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Communication error while issuing unit definition"
              << " on server " << serverID.str() << std::endl;
    }

    return finish_task(taskID, false);
}

void Sync::load_introduction_server(const Lock& lock) const
{
    OT_ASSERT(verify_lock(lock, introduction_server_lock_))

    introduction_server_id_.reset(
        new OTIdentifier(get_introduction_server(lock)));
}

bool Sync::message_nym(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& targetNymID,
    const std::string& text) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == targetNymID.empty())

    auto action =
        client_.ServerAction().SendMessage(nymID, serverID, targetNymID, text);
    action->Run();

    if (SendResult::VALID_REPLY == action->LastSendResult()) {
        OT_ASSERT(action->Reply());

        if (action->Reply()->m_bSuccess) {
            const auto messageID = action->MessageID();

            if (false == messageID->empty()) {
                LogVerbose(OT_METHOD)(__FUNCTION__)(": Sent message  ")(
                    messageID)
                    .Flush();
                associate_message_id(messageID, taskID);
            }

            return finish_task(taskID, true);
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Server  " << serverID.str()
                  << " does not accept message for " << targetNymID.str()
                  << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Communication error while messaging nym "
              << targetNymID.str() << " on server " << serverID.str()
              << std::endl;
    }

    return finish_task(taskID, false);
}

bool Sync::pay_nym(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& targetNymID,
    std::shared_ptr<const OTPayment>& payment) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == targetNymID.empty())

    auto action = client_.ServerAction().SendPayment(
        nymID, serverID, targetNymID, payment);
    action->Run();

    if (SendResult::VALID_REPLY == action->LastSendResult()) {
        OT_ASSERT(action->Reply());

        if (action->Reply()->m_bSuccess) {
            const auto messageID = action->MessageID();

            if (false == messageID->empty()) {
                LogVerbose(OT_METHOD)(__FUNCTION__)(": Sent (payment) "
                                                    "message ")(messageID)
                    .Flush();
            }

            return finish_task(taskID, true);
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Server  " << serverID.str()
                  << " does not accept (payment) message "
                     "for "
                  << targetNymID.str() << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Communication error while messaging (a payment) to nym "
              << targetNymID.str() << " on server " << serverID.str()
              << std::endl;
    }

    return finish_task(taskID, false);
}

#if OT_CASH
bool Sync::pay_nym_cash(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& targetNymID,
    std::shared_ptr<const Purse>& recipientCopy,
    std::shared_ptr<const Purse>& senderCopy) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == targetNymID.empty())

    auto action = client_.ServerAction().SendCash(
        nymID, serverID, targetNymID, recipientCopy, senderCopy);
    action->Run();

    if (SendResult::VALID_REPLY == action->LastSendResult()) {
        OT_ASSERT(action->Reply());

        if (action->Reply()->m_bSuccess) {
            const auto messageID = action->MessageID();

            if (false == messageID->empty()) {
                LogVerbose(OT_METHOD)(__FUNCTION__)(": Sent (cash) message  ")(
                    messageID)
                    .Flush();
            }

            return finish_task(taskID, true);
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Server  " << serverID.str()
                  << " does not accept (cash) message for " << targetNymID.str()
                  << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Communication error while messaging (cash) to nym "
              << targetNymID.str() << " on server " << serverID.str()
              << std::endl;
    }

    return finish_task(taskID, false);
}
#endif  // OT_CASH

OTIdentifier Sync::MessageContact(
    const Identifier& senderNymID,
    const Identifier& contactID,
    const std::string& message) const
{
    CHECK_SERVER(senderNymID, contactID)

    start_introduction_server(senderNymID);
    auto serverID = Identifier::Factory();
    auto recipientNymID = Identifier::Factory();
    const auto canMessage =
        can_message(senderNymID, contactID, recipientNymID, serverID);

    if (Messagability::READY != canMessage) { return Identifier::Factory(); }

    OT_ASSERT(false == serverID->empty())
    OT_ASSERT(false == recipientNymID->empty())

    auto& queue = get_operations({senderNymID, serverID});
    const auto taskID(Identifier::Random());

    return start_task(
        taskID, queue.send_message_.Push(taskID, {recipientNymID, message}));
}

std::pair<ThreadStatus, OTIdentifier> Sync::MessageStatus(
    const Identifier& taskID) const
{
    std::pair<ThreadStatus, OTIdentifier> output{{},
                                                 Identifier::Factory(taskID)};
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

OTIdentifier Sync::PayContact(
    const Identifier& senderNymID,
    const Identifier& contactID,
    std::shared_ptr<const OTPayment> payment) const
{
    CHECK_SERVER(senderNymID, contactID)

    start_introduction_server(senderNymID);
    auto serverID = Identifier::Factory();
    auto recipientNymID = Identifier::Factory();
    const auto canMessage =
        can_message(senderNymID, contactID, recipientNymID, serverID);

    if (Messagability::READY != canMessage) { return Identifier::Factory(); }

    OT_ASSERT(false == serverID->empty())
    OT_ASSERT(false == recipientNymID->empty())

    auto& queue = get_operations({senderNymID, serverID});
    const auto taskID(Identifier::Random());

    return start_task(
        taskID,
        queue.send_payment_.Push(
            taskID,
            {recipientNymID, std::shared_ptr<const OTPayment>(payment)}));
}

#if OT_CASH
OTIdentifier Sync::PayContactCash(
    const Identifier& senderNymID,
    const Identifier& contactID,
    std::shared_ptr<const Purse>& recipientCopy,
    std::shared_ptr<const Purse>& senderCopy) const
{
    CHECK_SERVER(senderNymID, contactID)

    start_introduction_server(senderNymID);
    auto serverID = Identifier::Factory();
    auto recipientNymID = Identifier::Factory();
    const auto canMessage =
        can_message(senderNymID, contactID, recipientNymID, serverID);

    if (Messagability::READY != canMessage) { return Identifier::Factory(); }

    OT_ASSERT(false == serverID->empty())
    OT_ASSERT(false == recipientNymID->empty())

    auto& queue = get_operations({senderNymID, serverID});
    const auto taskID(Identifier::Random());

    return start_task(
        taskID,
        queue.send_cash_.Push(
            taskID,
            {recipientNymID,
             std::shared_ptr<const Purse>(recipientCopy),
             std::shared_ptr<const Purse>(senderCopy)}));
}
#endif  // OT_CASH

void Sync::process_account(const zmq::Message& message) const
{
    OT_ASSERT(2 == message.Body().size())

    const std::string id(*message.Body().begin());
    const auto& balance = message.Body().at(1);

    OT_ASSERT(balance.size() == sizeof(Amount))

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Account ")(id)(" balance: ")(
        *static_cast<const Amount*>(balance.data()))
        .Flush();
}

bool Sync::process_inbox(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& accountID) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == accountID.empty())

    const auto processed = AcceptIncoming(nymID, accountID, serverID);

    if (processed) {
        return finish_task(taskID, true);
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to process inbox ")(
            accountID)
            .Flush();
    }

    return finish_task(taskID, false);
}

void Sync::process_notification(const zmq::Message& message) const
{
    OT_ASSERT(0 < message.Body().size())

    const auto& frame = message.Body().at(0);
    const auto notification = otx::Reply::Factory(
        client_,
        proto::RawToProto<proto::ServerReply>(frame.data(), frame.size()));
    const auto& nymID = notification->Recipient();
    const auto& serverID = notification->Server();

    if (false == valid_context(nymID, serverID)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": No context available to handle notification" << std::endl;

        return;
    }

    auto context = client_.Wallet().mutable_ServerContext(nymID, serverID);

    switch (notification->Type()) {
        case proto::SERVERREPLY_PUSH: {
            ot_client_.ProcessNotification(notification, context.It());
        } break;
        default: {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unsupported server reply type: "
                  << std::to_string(notification->Type()) << std::endl;
        }
    }
}

bool Sync::publish_server_contract(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& contractID) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == contractID.empty())

    auto action = client_.ServerAction().PublishServerContract(
        nymID, serverID, contractID);
    action->Run();

    if (SendResult::VALID_REPLY == action->LastSendResult()) {
        OT_ASSERT(action->Reply());

        if (action->Reply()->m_bSuccess) {

            return finish_task(taskID, true);
        } else {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed to publish server contract " << contractID.str()
                  << " on server " << serverID.str() << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Communication error while uploading contract "
              << contractID.str() << " to server " << serverID.str()
              << std::endl;
    }

    return finish_task(taskID, false);
}

bool Sync::publish_server_registration(
    const Identifier& nymID,
    const Identifier& serverID,
    const bool forcePrimary) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())

    auto nym = client_.Wallet().mutable_Nym(nymID);

    return nym.AddPreferredOTServer(serverID.str(), forcePrimary);
}

bool Sync::queue_cheque_deposit(const Identifier& nymID, const Cheque& cheque)
    const
{
    auto payment{client_.Factory().Payment(String::Factory(cheque))};

    OT_ASSERT(false != bool(payment));

    payment->SetTempValuesFromCheque(cheque);

    if (cheque.GetRecipientNymID().empty()) {
        payment->SetTempRecipientNymID(nymID);
    }

    std::shared_ptr<OTPayment> ppayment{payment.release()};
    const auto taskID = DepositPayment(nymID, ppayment);

    return (false == taskID->empty());
}

void Sync::Refresh() const
{
    client_.Pair().Update();
    refresh_accounts();

    SHUTDOWN()

    refresh_contacts();
    ++refresh_counter_;
}

std::uint64_t Sync::RefreshCount() const { return refresh_counter_.load(); }

void Sync::refresh_accounts() const
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
            LogDetail(OT_METHOD)(__FUNCTION__)(": Nym ")(nymID)(" ").Flush();
            const bool registered =
                client_.OTAPI().IsNym_RegisteredAtServer(nymID, serverID);

            if (registered) {
                LogDetail(OT_METHOD)(__FUNCTION__)(": is ").Flush();
                auto& queue = get_operations({nymID, serverID});
                const auto taskID(Identifier::Random());
                queue.download_nymbox_.Push(taskID, true);
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(": is not ").Flush();
            }

            LogDetail(OT_METHOD)(__FUNCTION__)(": registered here.").Flush();
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
        auto& queue = get_operations({nymID, serverID});
        const auto taskID(Identifier::Random());
        queue.download_account_.Push(taskID, accountID);
    }

    LogVerbose(OT_METHOD)(__FUNCTION__)(": End").Flush();
}

void Sync::refresh_contacts() const
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

        for (const auto& nymID : nymList) {
            SHUTDOWN()

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
                const auto taskID(Identifier::Random());
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

                    const auto taskID(Identifier::Random());
                    missing_nyms_.Push(taskID, nymID);
                    continue;
                }

                for (const auto& [claimID, item] : *serverGroup) {
                    SHUTDOWN()
                    OT_ASSERT(item)

                    const auto& notUsed[[maybe_unused]] = claimID;
                    const OTIdentifier serverID =
                        Identifier::Factory(item->Value());

                    if (serverID->empty()) { continue; }

                    LogVerbose(OT_METHOD)(__FUNCTION__)(": Will download nym ")(
                        nymID)(" from server ")(serverID)
                        .Flush();
                    auto& serverQueue = get_nym_fetch(serverID);
                    const auto taskID(Identifier::Random());
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

bool Sync::register_account(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& unitID) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == unitID.empty())

    auto contract = client_.Wallet().UnitDefinition(unitID);

    if (false == bool(contract)) {
        auto action =
            client_.ServerAction().DownloadContract(nymID, serverID, unitID);
        action->Run();

        if (SendResult::VALID_REPLY == action->LastSendResult()) {
            contract = client_.Wallet().UnitDefinition(unitID);

            if (false == bool(contract)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Notary ")(serverID)(
                    " does not have unit definition ")(unitID)
                    .Flush();

                return finish_task(taskID, false);
            }
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Communication error while downloading unit definition ")(
                unitID)(" on server ")(serverID)
                .Flush();

            return finish_task(taskID, false);
        }
    }

    auto action =
        client_.ServerAction().RegisterAccount(nymID, serverID, unitID);
    action->Run();

    if (SendResult::VALID_REPLY == action->LastSendResult()) {
        OT_ASSERT(action->Reply());

        if (action->Reply()->m_bSuccess) {
            client_.Pair().Update();

            return finish_task(taskID, true);
        } else {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed to register account for " << unitID.str()
                  << " on server " << serverID.str() << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Communication error while registering account "
              << " on server " << serverID.str() << std::endl;
    }

    return finish_task(taskID, false);
}

bool Sync::register_nym(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())

    set_contact(nymID, serverID);
    auto action = client_.ServerAction().RegisterNym(nymID, serverID);
    action->Run();

    if (SendResult::VALID_REPLY == action->LastSendResult()) {
        OT_ASSERT(action->Reply());

        if (action->Reply()->m_bSuccess) {
            client_.Pair().Update();

            return finish_task(taskID, true);
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Server " << serverID.str()
                  << " did not accept registration for nym " << nymID.str()
                  << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Communication error while registering nym " << nymID.str()
              << " on server " << serverID.str() << std::endl;
    }

    return finish_task(taskID, false);
}

OTIdentifier Sync::RegisterNym(
    const Identifier& nymID,
    const Identifier& serverID,
    const bool setContactData,
    const bool forcePrimary) const
{
    CHECK_SERVER(nymID, serverID)

    start_introduction_server(nymID);

    if (setContactData) {
        publish_server_registration(nymID, serverID, forcePrimary);
    }

    return ScheduleRegisterNym(nymID, serverID);
}

OTIdentifier Sync::SetIntroductionServer(const ServerContract& contract) const
{
    Lock lock(introduction_server_lock_);

    return set_introduction_server(lock, contract);
}

OTIdentifier Sync::schedule_download_nymbox(
    const Identifier& localNymID,
    const Identifier& serverID) const
{
    CHECK_SERVER(localNymID, serverID)

    start_introduction_server(localNymID);
    auto& queue = get_operations({localNymID, serverID});
    const auto taskID(Identifier::Random());

    return start_task(taskID, queue.download_nymbox_.Push(taskID, true));
}

OTIdentifier Sync::schedule_register_account(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& unitID) const
{
    CHECK_ARGS(localNymID, serverID, unitID)

    start_introduction_server(localNymID);
    auto& queue = get_operations({localNymID, serverID});
    const auto taskID(Identifier::Random());

    return start_task(taskID, queue.register_account_.Push(taskID, unitID));
}

OTIdentifier Sync::ScheduleDownloadAccount(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID) const
{
    CHECK_ARGS(localNymID, serverID, accountID)

    start_introduction_server(localNymID);
    auto& queue = get_operations({localNymID, serverID});
    const auto taskID(Identifier::Random());

    return start_task(taskID, queue.download_account_.Push(taskID, accountID));
}

OTIdentifier Sync::ScheduleDownloadContract(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& contractID) const
{
    CHECK_ARGS(localNymID, serverID, contractID)

    start_introduction_server(localNymID);
    auto& queue = get_operations({localNymID, serverID});
    const auto taskID(Identifier::Random());

    return start_task(
        taskID, queue.download_contract_.Push(taskID, contractID));
}

OTIdentifier Sync::ScheduleDownloadNym(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetNymID) const
{
    CHECK_ARGS(localNymID, serverID, targetNymID)

    start_introduction_server(localNymID);
    auto& queue = get_operations({localNymID, serverID});
    const auto taskID(Identifier::Random());

    return start_task(taskID, queue.check_nym_.Push(taskID, targetNymID));
}

OTIdentifier Sync::ScheduleDownloadNymbox(
    const Identifier& localNymID,
    const Identifier& serverID) const
{
    return schedule_download_nymbox(localNymID, serverID);
}

OTIdentifier Sync::ScheduleIssueUnitDefinition(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& unitID) const
{
    CHECK_ARGS(localNymID, serverID, unitID)

    start_introduction_server(localNymID);
    auto& queue = get_operations({localNymID, serverID});
    const auto taskID(Identifier::Random());

    return start_task(
        taskID, queue.issue_unit_definition_.Push(taskID, unitID));
}

OTIdentifier Sync::ScheduleProcessInbox(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID) const
{
    CHECK_ARGS(localNymID, serverID, accountID)

    auto& queue = get_operations({localNymID, serverID});
    const auto taskID(Identifier::Random());

    return start_task(taskID, queue.process_inbox_.Push(taskID, accountID));
}

OTIdentifier Sync::SchedulePublishServerContract(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& contractID) const
{
    CHECK_ARGS(localNymID, serverID, contractID)

    start_introduction_server(localNymID);
    auto& queue = get_operations({localNymID, serverID});
    const auto taskID(Identifier::Random());

    return start_task(
        taskID, queue.publish_server_contract_.Push(taskID, contractID));
}

OTIdentifier Sync::ScheduleRegisterAccount(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& unitID) const
{
    return schedule_register_account(localNymID, serverID, unitID);
}

OTIdentifier Sync::ScheduleRegisterNym(
    const Identifier& localNymID,
    const Identifier& serverID) const
{
    CHECK_SERVER(localNymID, serverID)

    start_introduction_server(localNymID);
    auto& queue = get_operations({localNymID, serverID});
    const auto taskID(Identifier::Random());

    return start_task(taskID, queue.register_nym_.Push(taskID, true));
}

OTIdentifier Sync::ScheduleSendCheque(
    const Identifier& senderNymID,
    const Identifier& sourceAccountID,
    const Identifier& contactID,
    const Amount value,
    const std::string& memo,
    const Time validFrom,
    const Time validTo) const
{
    CHECK_SERVER(senderNymID, contactID)

    start_introduction_server(senderNymID);
    auto serverID = Identifier::Factory();
    auto recipientNymID = Identifier::Factory();
    const auto canMessage =
        can_message(senderNymID, contactID, recipientNymID, serverID);

    if (Messagability::READY != canMessage) { return Identifier::Factory(); }

    OT_ASSERT(false == serverID->empty())
    OT_ASSERT(false == recipientNymID->empty())

    auto& queue = get_operations({senderNymID, serverID});
    const auto taskID(Identifier::Random());
    SendChequeTask task{
        sourceAccountID, recipientNymID, value, memo, validFrom, validTo};

    return start_task(taskID, queue.send_cheque_.Push(taskID, task));
}

bool Sync::send_transfer(
    const Identifier& taskID,
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& sourceAccountID,
    const Identifier& targetAccountID,
    const std::int64_t value,
    const std::string& memo) const
{
    auto action = client_.ServerAction().SendTransfer(
        localNymID, serverID, sourceAccountID, targetAccountID, value, memo);
    action->Run();

    if (SendResult::VALID_REPLY == action->LastSendResult()) {
        OT_ASSERT(action->Reply());

        if (action->Reply()->m_bSuccess) {
            return finish_task(taskID, true);
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Failed to send transfer "
                  << "to " << serverID.str() << " for account "
                  << targetAccountID.str() << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Communication error while sending transfer to account "
              << targetAccountID.str() << " on server " << serverID.str()
              << std::endl;
    }

    return finish_task(taskID, false);
}

OTIdentifier Sync::SendCheque(
    const Identifier& localNymID,
    const Identifier& sourceAccountID,
    const Identifier& recipientContactID,
    const Amount value,
    const std::string& memo,
    const Time validFrom,
    const Time validTo) const
{
    CHECK_ARGS(localNymID, sourceAccountID, recipientContactID)

    if (0 >= value) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid amount" << std::endl;

        return Identifier::Factory();
    }

    const auto contact = client_.Contacts().Contact(recipientContactID);

    if (false == bool(contact)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid contact" << std::endl;

        return Identifier::Factory();
    }

    const auto nyms = contact->Nyms(false);

    if (0 == nyms.size()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Contact can not receive cheques" << std::endl;

        return Identifier::Factory();
    }

    // The first nym in the vector should be the primary, if a primary is set
    const auto& recipientNymID = nyms[0];
    auto account = client_.Wallet().Account(sourceAccountID);

    if (false == bool(account)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid account" << std::endl;

        return Identifier::Factory();
    }

    const auto& notaryID = account.get().GetRealNotaryID();
    account.Release();
    std::unique_ptr<Cheque> cheque(client_.OTAPI().WriteCheque(
        notaryID,
        value,
        Clock::to_time_t(validFrom),
        Clock::to_time_t(validTo),
        sourceAccountID,
        localNymID,
        String::Factory(memo.c_str()),
        recipientNymID));

    if (false == bool(cheque)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to write cheque"
              << std::endl;

        return Identifier::Factory();
    }

    auto payment{client_.Factory().Payment(String::Factory(*cheque))};

    OT_ASSERT(false != bool(payment));

    if (false == bool(cheque)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to instantiate payment"
              << std::endl;

        return Identifier::Factory();
    }

    if (false == payment->SetTempValues()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid payment" << std::endl;

        return Identifier::Factory();
    }

    std::shared_ptr<OTPayment> ppayment{payment.release()};

    return PayContact(localNymID, recipientContactID, ppayment);
}

OTIdentifier Sync::SendExternalTransfer(
    const Identifier& localNymID,
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
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid source account"
              << std::endl;

        return Identifier::Factory();
    }

    if (sourceAccount.get().GetNymID() != localNymID) {
        otErr << OT_METHOD << __FUNCTION__ << ": Wrong owner on source account"
              << std::endl;

        return Identifier::Factory();
    }

    if (sourceAccount.get().GetRealNotaryID() != serverID) {
        otErr << OT_METHOD << __FUNCTION__ << ": Wrong notary on source account"
              << std::endl;

        return Identifier::Factory();
    }

    auto& queue = get_operations({localNymID, serverID});
    const auto taskID(Identifier::Random());

    return start_task(
        taskID,
        queue.send_transfer_.Push(
            taskID, {sourceAccountID, targetAccountID, value, memo}));
}

OTIdentifier Sync::SendTransfer(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& sourceAccountID,
    const Identifier& targetAccountID,
    const std::int64_t value,
    const std::string& memo) const
{
    CHECK_ARGS(localNymID, serverID, targetAccountID)
    CHECK_NYM(sourceAccountID)

    auto targetAccount = client_.Wallet().Account(targetAccountID);

    if (false == bool(targetAccount)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid target account"
              << std::endl;

        return Identifier::Factory();
    }

    if (targetAccount.get().GetNymID() != localNymID) {
        otErr << OT_METHOD << __FUNCTION__ << ": Wrong owner on target account"
              << std::endl;

        return Identifier::Factory();
    }

    if (targetAccount.get().GetRealNotaryID() != serverID) {
        otErr << OT_METHOD << __FUNCTION__ << ": Wrong notary on target account"
              << std::endl;

        return Identifier::Factory();
    }

    auto sourceAccount = client_.Wallet().Account(sourceAccountID);

    if (false == bool(sourceAccount)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid source account"
              << std::endl;

        return Identifier::Factory();
    }

    if (sourceAccount.get().GetNymID() != localNymID) {
        otErr << OT_METHOD << __FUNCTION__ << ": Wrong owner on source account"
              << std::endl;

        return Identifier::Factory();
    }

    if (sourceAccount.get().GetRealNotaryID() != serverID) {
        otErr << OT_METHOD << __FUNCTION__ << ": Wrong notary on source account"
              << std::endl;

        return Identifier::Factory();
    }

    if (sourceAccount.get().GetInstrumentDefinitionID() !=
        targetAccount.get().GetInstrumentDefinitionID()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Source and target account"
              << " instrument definition ids don't match" << std::endl;

        return Identifier::Factory();
    }

    auto& queue = get_operations({localNymID, serverID});
    const auto taskID(Identifier::Random());

    return start_task(
        taskID,
        queue.send_transfer_.Push(
            taskID, {sourceAccountID, targetAccountID, value, memo}));
}

void Sync::set_contact(const Identifier& nymID, const Identifier& serverID)
    const
{
    auto nym = client_.Wallet().mutable_Nym(nymID);
    const auto server = nym.PreferredOTServer();

    if (server.empty()) { nym.AddPreferredOTServer(serverID.str(), true); }
}

OTIdentifier Sync::set_introduction_server(
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

void Sync::start_introduction_server(const Identifier& nymID) const
{
    auto& serverID = IntroductionServer();

    if (serverID.empty()) { return; }

    auto& queue = get_operations({nymID, serverID});
    const auto taskID(Identifier::Random());
    start_task(taskID, queue.download_nymbox_.Push(taskID, true));
}

OTIdentifier Sync::start_task(const Identifier& taskID, bool success) const
{
    if (taskID.empty()) { return Identifier::Factory(); }

    if (false == success) { return Identifier::Factory(); }

    add_task(taskID, ThreadStatus::RUNNING);

    return taskID;
}

void Sync::StartIntroductionServer(const Identifier& localNymID) const
{
    start_introduction_server(localNymID);
}

void Sync::state_machine(const ContextID id, OperationQueue& queue) const
{
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
        if (check_registration(nymID, serverID, context)) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Nym ")(nymID)(
                " has registered on server ")(serverID)(" at least once.")
                .Flush();

            break;
        }

        YIELD(NYM_REGISTRATION_MILLISECONDS);
    }

    SHUTDOWN()
    OT_ASSERT(context)

    bool queueValue{false};
    bool needAdmin{false};
    bool registerNym{false};
    bool registerNymQueued{false};
    bool downloadNymbox{false};
    auto taskID = Identifier::Factory();
    auto accountID = Identifier::Factory();
    auto unitID = Identifier::Factory();
    auto contractID = Identifier::Factory();
    auto targetNymID = Identifier::Factory();
    auto nullID = Identifier::Factory();
    OTPassword serverPassword;
    MessageTask message{Identifier::Factory(), ""};
    PaymentTask payment{Identifier::Factory(), {}};
    SendChequeTask sendCheque{Identifier::Factory(),
                              Identifier::Factory(),
                              0,
                              "",
                              Clock::now(),
                              Clock::now()};
#if OT_CASH
    PayCashTask cash_payment{Identifier::Factory(), {}, {}};
#endif  // OT_CASH
    DepositPaymentTask deposit{Identifier::Factory(), {}};
    UniqueQueue<DepositPaymentTask> depositPaymentRetry;
    SendTransferTask transfer{
        Identifier::Factory(), Identifier::Factory(), {}, {}};

    // Primary loop
    while (running_) {
        SHUTDOWN()

        // If the local nym has updated since the last registernym operation,
        // schedule a registernym
        check_nym_revision(*context, queue);

        SHUTDOWN()

        // Register the nym, if scheduled. Keep trying until success
        registerNymQueued = queue.register_nym_.Pop(taskID, queueValue);
        registerNym |= queueValue;

        if (registerNymQueued || registerNym) {
            if (register_nym(taskID, nymID, serverID)) {
                registerNym = false;
                queueValue = false;
            } else {
                registerNym = true;
            }
        }

        SHUTDOWN()

        // If this server was added by a pairing operation that included
        // a server password then request admin permissions on the server
        const auto haveAdmin = context->isAdmin();
        needAdmin = context->HaveAdminPassword() && (false == haveAdmin);

        if (needAdmin) {
            serverPassword.setPassword(context->AdminPassword());
            get_admin(nymID, serverID, serverPassword);
        }

        SHUTDOWN()

        if (haveAdmin) { check_server_name(*context); }

        SHUTDOWN()

        if (0 == queue.counter_ % 100) {
            // download server nym in case it has been renamed
            queue.check_nym_.Push(
                Identifier::Random(), context->RemoteNym().ID());
        }

        SHUTDOWN()

        // This is a list of servers for which we do not have a contract.
        // We ask all known servers on which we are registered to try to find
        // the contracts.
        const auto servers = missing_servers_.Copy();

        for (const auto& [targetID, taskID] : servers) {
            SHUTDOWN()

            if (targetID->empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty serverID get in here?"
                      << std::endl;

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(
                    ": Searching for server contract for ")(targetID)
                    .Flush();
            }

            const auto& notUsed[[maybe_unused]] = taskID;
            find_server(nymID, serverID, targetID);
        }

        // This is a list of contracts (server and unit definition) which a
        // user of this class has requested we download from this server.
        while (queue.download_contract_.Pop(taskID, contractID)) {
            SHUTDOWN()

            if (contractID->empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty contract ID get in here?"
                      << std::endl;

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(
                    ": Searching for unit definition contract for ")(contractID)
                    .Flush();
            }

            download_contract(taskID, nymID, serverID, contractID);
        }

        // This is a list of nyms for which we do not have credentials..
        // We ask all known servers on which we are registered to try to find
        // their credentials.
        const auto nyms = missing_nyms_.Copy();

        for (const auto& [targetID, taskID] : nyms) {
            SHUTDOWN()

            if (targetID->empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty nymID get in here?" << std::endl;

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(": Searching for nym ")(
                    targetID)
                    .Flush();
            }

            const auto& notUsed[[maybe_unused]] = taskID;
            find_nym(nymID, serverID, targetID);
        }

        // This is a list of nyms which haven't been updated in a while and
        // are known or suspected to be available on this server
        auto& nymQueue = get_nym_fetch(serverID);

        while (nymQueue.Pop(taskID, targetNymID)) {
            SHUTDOWN()

            if (targetNymID->empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty nymID get in here?" << std::endl;

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(": Refreshing nym ")(
                    targetNymID)
                    .Flush();
            }

            download_nym(taskID, nymID, serverID, targetNymID);
        }

        // This is a list of nyms which a user of this class has requested we
        // download from this server.
        while (queue.check_nym_.Pop(taskID, targetNymID)) {
            SHUTDOWN()

            if (targetNymID->empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty nymID get in here?" << std::endl;

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(": Searching for nym ")(
                    targetNymID)
                    .Flush();
            }

            download_nym(taskID, nymID, serverID, targetNymID);
        }

        // This is a list of messages which need to be delivered to a nym
        // on this server
        while (queue.send_message_.Pop(taskID, message)) {
            SHUTDOWN()

            const auto& [recipientID, text] = message;

            if (recipientID->empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty recipient nymID get in here?"
                      << std::endl;

                continue;
            }

            message_nym(taskID, nymID, serverID, recipientID, text);
        }

        // Download the nymbox, if this operation has been scheduled
        if (queue.download_nymbox_.Pop(taskID, downloadNymbox)) {
            LogDetail(OT_METHOD)(__FUNCTION__)(": Downloading nymbox for ")(
                nymID)(" on ")(serverID)
                .Flush();
            registerNym |= !download_nymbox(taskID, nymID, serverID);
        }

        SHUTDOWN()

        // This is a list of cheques which need to be written and delivered to
        // a nym on this server
        while (queue.send_cheque_.Pop(taskID, sendCheque)) {
            SHUTDOWN()

            auto& [accountID, recipientID, amount, memo, start, end] =
                sendCheque;

            if (accountID->empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty account ID get in here?"
                      << std::endl;

                continue;
            }

            if (recipientID->empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty recipient nymID get in here?"
                      << std::endl;

                continue;
            }

            write_and_send_cheque(
                taskID,
                nymID,
                serverID,
                accountID,
                recipientID,
                amount,
                memo,
                start,
                end);
        }

        SHUTDOWN()

        // This is a list of payments which need to be delivered to a nym
        // on this server
        while (queue.send_payment_.Pop(taskID, payment)) {
            SHUTDOWN()

            auto& [recipientID, pPayment] = payment;

            if (recipientID->empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty recipient nymID get in here?"
                      << std::endl;

                continue;
            }

            pay_nym(taskID, nymID, serverID, recipientID, pPayment);
        }

        SHUTDOWN()

#if OT_CASH
        // This is a list of cash payments which need to be delivered to a nym
        // on this server
        while (queue.send_cash_.Pop(taskID, cash_payment)) {
            SHUTDOWN()

            auto& [recipientID, pRecipientPurse, pSenderPurse] = cash_payment;

            if (recipientID->empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty recipient nymID get in here?"
                      << std::endl;

                continue;
            }

            pay_nym_cash(
                taskID,
                nymID,
                serverID,
                recipientID,
                pRecipientPurse,
                pSenderPurse);
        }
#endif

        SHUTDOWN()

        // Download any accounts which have been scheduled for download
        while (queue.download_account_.Pop(taskID, accountID)) {
            SHUTDOWN()

            if (accountID->empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty account ID get in here?"
                      << std::endl;

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(": Downloading account ")(
                    accountID)(" for ")(nymID)(" on ")(serverID)
                    .Flush();
            }

            registerNym |=
                !download_account(taskID, nymID, serverID, accountID);
        }

        SHUTDOWN()

        // Register any accounts which have been scheduled for creation
        while (queue.register_account_.Pop(taskID, unitID)) {
            SHUTDOWN()

            if (unitID->empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty unit ID get in here?" << std::endl;

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(": Creating account for ")(
                    unitID)(" on ")(serverID)
                    .Flush();
            }

            registerNym |= !register_account(taskID, nymID, serverID, unitID);
        }

        SHUTDOWN()

        // Issue unit definitions which have been scheduled
        while (queue.issue_unit_definition_.Pop(taskID, unitID)) {
            SHUTDOWN()

            if (unitID->empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty unit ID get in here?" << std::endl;

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(
                    ": Issuing unit definition for ")(unitID)(" on ")(serverID)
                    .Flush();
            }

            registerNym |=
                !issue_unit_definition(taskID, nymID, serverID, unitID);
        }

        SHUTDOWN()

        // Issue unit definitions which have been scheduled
        while (queue.issue_unit_definition_.Pop(taskID, unitID)) {
            SHUTDOWN()

            if (unitID->empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty unit ID get in here?" << std::endl;

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(
                    ": Issuing unit definition for ")(unitID)(" on ")(serverID)
                    .Flush();
            }

            registerNym |=
                !issue_unit_definition(taskID, nymID, serverID, unitID);
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
                    registerNym |= !deposit_cheque(
                        taskID,
                        nymID,
                        serverID,
                        accountID,
                        payment,
                        depositPaymentRetry);
                } break;
                case Depositability::NOT_REGISTERED:
                case Depositability::NO_ACCOUNT: {
                    LogDetail(OT_METHOD)(__FUNCTION__)(
                        ": Temporary failure trying to deposit payment")
                        .Flush();
                    depositPaymentRetry.Push(taskID, deposit);
                } break;
                default: {
                    otErr << OT_METHOD << __FUNCTION__
                          << ": Permanent failure trying to deposit payment"
                          << std::endl;
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
                nymID,
                serverID,
                sourceAccountID,
                targetAccountID,
                value,
                memo);
        }

        while (queue.publish_server_contract_.Pop(taskID, contractID)) {
            SHUTDOWN()

            if (contractID->empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": How did an empty contract ID get in here?"
                      << std::endl;

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(
                    ": Uploading server contract ")(contractID)
                    .Flush();
            }

            publish_server_contract(taskID, nymID, serverID, contractID);
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

            process_inbox(taskID, nymID, serverID, accountID);
        }

        ++queue.counter_;

        YIELD(MAIN_LOOP_MILLISECONDS);
    }
}

ThreadStatus Sync::status(const Lock& lock, const Identifier& taskID) const
{
    OT_ASSERT(verify_lock(lock, task_status_lock_))

    if (!running_) { return ThreadStatus::SHUTDOWN; }

    auto it = task_status_.find(taskID);

    if (task_status_.end() == it) { return ThreadStatus::ERROR; }

    const auto output = it->second;
    const bool success = (ThreadStatus::FINISHED_SUCCESS == output);
    const bool failed = (ThreadStatus::FINISHED_FAILED == output);
    const bool finished = (success || failed);

    if (finished) { task_status_.erase(it); }

    return output;
}

ThreadStatus Sync::Status(const Identifier& taskID) const
{
    Lock lock(task_status_lock_);

    return status(lock, taskID);
}

void Sync::update_task(const Identifier& taskID, const ThreadStatus status)
    const
{
    if (taskID.empty()) { return; }

    Lock lock(task_status_lock_);

    if (0 == task_status_.count(taskID)) { return; }

    task_status_[taskID] = status;
    bool value{false};
    bool publish{false};

    switch (status) {
        case ThreadStatus::FINISHED_SUCCESS: {
            value = true;
            publish = true;
        } break;
        case ThreadStatus::FINISHED_FAILED: {
            value = false;
            publish = true;
        } break;
        case ThreadStatus::ERROR:
        case ThreadStatus::RUNNING:
        case ThreadStatus::SHUTDOWN:
        default: {
        }
    }

    if (publish) {
        auto message = zmq::Message::Factory();
        message->AddFrame();
        message->AddFrame(taskID.str());
        message->AddFrame(Data::Factory(&value, sizeof(value)));
        task_finished_->Publish(message);
    }
}

Depositability Sync::valid_account(
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

bool Sync::valid_context(const Identifier& nymID, const Identifier& serverID)
    const
{
    const auto nyms = client_.Wallet().LocalNyms();

    if (0 == nyms.count(nymID)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID.str()
              << " does not belong to this wallet." << std::endl;

        return false;
    }

    if (serverID.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid server" << std::endl;

        return false;
    }

    const auto context = client_.Wallet().ServerContext(nymID, serverID);

    if (false == bool(context)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Context does not exist"
              << std::endl;

        return false;
    }

    if (0 == context->Request()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Nym is not registered at this server" << std::endl;

        return false;
    }

    return true;
}

Depositability Sync::valid_recipient(
    const OTPayment& payment,
    const Identifier& specified,
    const Identifier& recipient) const
{
    if (specified.empty()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Payment can be accepted by any nym" << std::endl;

        return Depositability::READY;
    }

    if (recipient == specified) { return Depositability::READY; }

    return Depositability::WRONG_RECIPIENT;
}

bool Sync::write_and_send_cheque(
    const Identifier& taskID,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const Identifier& targetNymID,
    const Amount value,
    const std::string& memo,
    const Time validFrom,
    const Time validTo) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())
    OT_ASSERT(false == accountID.empty())
    OT_ASSERT(false == targetNymID.empty())

    if (0 >= value) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid amount").Flush();

        return finish_task(taskID, false);
    }

    auto downloaded = client_.ServerAction().DownloadNymbox(nymID, serverID);

    if (false == downloaded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to get nymbox (before transaction numbers)")
            .Flush();

        return finish_task(taskID, false);
    }

    auto gotnumbers =
        client_.ServerAction().GetTransactionNumbers(nymID, serverID, 1);

    if (false == gotnumbers) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to get transaction numbers")
            .Flush();

        return finish_task(taskID, false);
    }

    downloaded = client_.ServerAction().DownloadNymbox(nymID, serverID);

    if (false == downloaded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to get nymbox (after transaction numbers)")
            .Flush();

        return finish_task(taskID, false);
    }

    std::unique_ptr<Cheque> cheque(client_.OTAPI().WriteCheque(
        serverID,
        value,
        Clock::to_time_t(validFrom),
        Clock::to_time_t(validTo),
        accountID,
        nymID,
        String::Factory(memo.c_str()),
        targetNymID));

    if (false == bool(cheque)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to write cheque").Flush();

        return finish_task(taskID, false);
    }

    auto payment{client_.Factory().Payment(String::Factory(*cheque))};

    if (false == bool(payment)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate payment")
            .Flush();

        return finish_task(taskID, false);
    }

    if (false == payment->SetTempValues()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid payment").Flush();

        return finish_task(taskID, false);
    }

    std::shared_ptr<const OTPayment> ppayment{payment.release()};
    auto action = client_.ServerAction().SendPayment(
        nymID, serverID, targetNymID, ppayment);
    action->Run();

    if (SendResult::VALID_REPLY == action->LastSendResult()) {
        OT_ASSERT(action->Reply());

        if (action->Reply()->m_bSuccess) {
            const auto messageID = action->MessageID();

            if (false == messageID->empty()) {
                LogVerbose(OT_METHOD)(__FUNCTION__)(": Sent (payment) ")(
                    "message ")(messageID)
                    .Flush();
            }

            return finish_task(taskID, true);
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Server  " << serverID.str()
                  << " does not accept (payment) message "
                     "for "
                  << targetNymID.str() << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Communication error while messaging (a payment) to nym "
              << targetNymID.str() << " on server " << serverID.str()
              << std::endl;
    }

    return finish_task(taskID, false);
}

Sync::~Sync()
{
    for (auto& [id, thread] : state_machines_) {
        const auto& notUsed[[maybe_unused]] = id;

        OT_ASSERT(thread)

        if (thread->joinable()) { thread->join(); }
    }
}
}  // namespace opentxs::api::client::implementation

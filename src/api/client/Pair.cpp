// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/Pair.hpp"
#include "opentxs/api/client/Sync.hpp"
#include "opentxs/api/client/Issuer.hpp"
#include "opentxs/api/client/ServerAction.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/ServerAction.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/contact/ContactSection.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/UniqueQueue.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"

#include <atomic>
#include <memory>
#include <map>
#include <set>
#include <thread>
#include <tuple>

#include "Pair.hpp"

#define MINIMUM_UNUSED_BAILMENTS 3

#define SHUTDOWN()                                                             \
    {                                                                          \
        if (!running_) { return; }                                             \
                                                                               \
        Log::Sleep(std::chrono::milliseconds(50));                             \
    }

#define OT_METHOD "opentxs::api::client::implementation::Pair::"

template class opentxs::Pimpl<opentxs::network::zeromq::PublishSocket>;

namespace opentxs
{
api::client::Pair* Factory::Pair(
    const Flag& running,
    const api::client::Manager& client)
{
    return new api::client::implementation::Pair(running, client);
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
Pair::Cleanup::Cleanup(Flag& run)
    : run_(run)
{
    run_.On();
}

Pair::Cleanup::~Cleanup() { run_.Off(); }

Pair::Pair(const Flag& running, const api::client::Manager& client)
    : running_(running)
    , client_(client)
    , status_lock_()
    , pairing_(Flag::Factory(false))
    , last_refresh_(0)
    , pairing_thread_(nullptr)
    , refresh_thread_(nullptr)
    , pair_status_()
    , update_()
    , pair_event_(client.ZeroMQ().PublishSocket())
    , pending_bailment_(client.ZeroMQ().PublishSocket())
{
    // WARNING: do not access client_.Wallet() during construction
    refresh_thread_.reset(new std::thread(&Pair::check_refresh, this));
    pair_event_->Start(client_.Endpoints().PairEvent());
    pending_bailment_->Start(client_.Endpoints().PendingBailment());
}

bool Pair::AddIssuer(
    const Identifier& localNymID,
    const Identifier& issuerNymID,
    const std::string& pairingCode) const
{
    if (localNymID.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid local nym id."
              << std::endl;

        return false;
    }

    if (!client_.Wallet().IsLocalNym(localNymID.str())) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid local nym"
              << std::endl;

        return false;
    }

    if (issuerNymID.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid issuer nym id."
              << std::endl;

        return false;
    }

    auto editor = client_.Wallet().mutable_Issuer(localNymID, issuerNymID);
    auto& issuer = editor.It();
    const bool needPairingCode = issuer.PairingCode().empty();
    const bool havePairingCode = (false == pairingCode.empty());

    if (havePairingCode && needPairingCode) {
        issuer.SetPairingCode(pairingCode);
    }

    update_pairing();

    return true;
}

bool Pair::CheckIssuer(
    const Identifier& localNymID,
    const Identifier& unitDefinitionID) const
{
    const auto contract = client_.Wallet().UnitDefinition(unitDefinitionID);

    if (false == bool(contract)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unit definition contract does not exist." << std::endl;

        return false;
    }

    return AddIssuer(localNymID, contract->Nym()->ID(), "");
}

void Pair::check_pairing() const
{
    Cleanup cleanup(pairing_);

    for (const auto& [nymID, issuerSet] : create_issuer_map()) {
        SHUTDOWN()

        for (const auto& issuerID : issuerSet) {
            SHUTDOWN()

            state_machine(nymID, issuerID);
        }
    }
}

void Pair::check_refresh() const
{
    auto taskID = Identifier::Factory();
    bool update{false};

    while (running_) {
        const auto current = client_.Sync().RefreshCount();
        const auto previous = last_refresh_.exchange(current);

        if (previous != current) { refresh(); }

        if (update_.Pop(taskID, update)) { refresh(); }

        Log::Sleep(std::chrono::milliseconds(100));
    }
}

std::map<OTIdentifier, std::set<OTIdentifier>> Pair::create_issuer_map() const
{
    std::map<OTIdentifier, std::set<OTIdentifier>> output;
    const auto nymList = client_.OTAPI().LocalNymList();

    for (const auto& nymID : nymList) {
        output[nymID] = client_.Wallet().IssuerList(nymID);
    }

    return output;
}

std::pair<bool, OTIdentifier> Pair::get_connection(
    const Identifier& localNymID,
    const Identifier& issuerNymID,
    const Identifier& serverID,
    const proto::ConnectionInfoType type) const
{
    std::pair<bool, OTIdentifier> output{false, Identifier::Factory()};
    auto& [success, requestID] = output;
    auto action = client_.ServerAction().InitiateRequestConnection(
        localNymID, serverID, issuerNymID, type);
    action->Run();

    if (SendResult::VALID_REPLY != action->LastSendResult()) { return output; }

    OT_ASSERT(action->Reply());

    success = action->Reply()->m_bSuccess;

    OT_ASSERT(action->SentPeerRequest());

    requestID = action->SentPeerRequest()->ID();

    return output;
}

std::pair<bool, OTIdentifier> Pair::initiate_bailment(
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& issuerID,
    const Identifier& unitID) const
{
    std::pair<bool, OTIdentifier> output(false, Identifier::Factory());
    auto& [success, requestID] = output;
    const auto contract = client_.Wallet().UnitDefinition(unitID);

    if (false == bool(contract)) {
        queue_unit_definition(nymID, serverID, unitID);

        return output;
    }

    auto action = client_.ServerAction().InitiateBailment(
        nymID, serverID, issuerID, unitID);
    action->Run();

    if (SendResult::VALID_REPLY != action->LastSendResult()) { return output; }

    OT_ASSERT(action->Reply());

    success = action->Reply()->m_bSuccess;

    OT_ASSERT(action->SentPeerRequest());

    requestID = action->SentPeerRequest()->ID();

    return output;
}

std::string Pair::IssuerDetails(
    const Identifier& localNymID,
    const Identifier& issuerNymID) const
{
    auto issuer = client_.Wallet().Issuer(localNymID, issuerNymID);

    if (false == bool(issuer)) { return {}; }

    return *issuer;
}

std::set<OTIdentifier> Pair::IssuerList(
    const Identifier& localNymID,
    const bool onlyTrusted) const
{
    Lock lock(status_lock_);

    if (0 == pair_status_.size()) {
        update_pairing();

        return {};
    }

    std::set<OTIdentifier> output{};

    for (const auto& [key, value] : pair_status_) {
        const auto& issuerID = std::get<1>(key);
        const auto& trusted = std::get<1>(value);

        if (trusted || (false == onlyTrusted)) { output.emplace(issuerID); }
    }

    return output;
}

bool Pair::need_registration(
    const Identifier& localNymID,
    const Identifier& serverID) const
{
    auto context = client_.Wallet().ServerContext(localNymID, serverID);

    if (context) { return (0 == context->Request()); }

    return true;
}

void Pair::process_connection_info(
    const Lock& lock,
    const Identifier& nymID,
    const proto::PeerReply& reply) const
{
    OT_ASSERT(verify_lock(lock, peer_lock_))
    OT_ASSERT(nymID == Identifier::Factory(reply.initiator()))
    OT_ASSERT(proto::PEERREQUEST_CONNECTIONINFO == reply.type())

    const auto requestID = Identifier::Factory(reply.cookie());
    const auto replyID = Identifier::Factory(reply.id());
    const auto issuerNymID = Identifier::Factory(reply.recipient());
    auto editor = client_.Wallet().mutable_Issuer(nymID, issuerNymID);
    auto& issuer = editor.It();
    const auto added =
        issuer.AddReply(proto::PEERREQUEST_CONNECTIONINFO, requestID, replyID);

    if (added) {
        client_.Wallet().PeerRequestComplete(nymID, replyID);
        update_.Push(Identifier::Random(), true);
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to add reply."
              << std::endl;
    }
}

void Pair::process_peer_replies(const Lock& lock, const Identifier& nymID) const
{
    OT_ASSERT(verify_lock(lock, peer_lock_));

    auto replies = client_.Wallet().PeerReplyIncoming(nymID);

    for (const auto& it : replies) {
        const auto replyID = Identifier::Factory(it.first);
        const auto reply = client_.Wallet().PeerReply(
            nymID, replyID, StorageBox::INCOMINGPEERREPLY);

        if (false == bool(reply)) {
            otErr << OT_METHOD << __FUNCTION__ << ": Failed to load peer reply "
                  << it.first << std::endl;

            continue;
        }

        const auto& type = reply->type();

        switch (type) {
            case proto::PEERREQUEST_BAILMENT: {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Received bailment reply." << std::endl;
                process_request_bailment(lock, nymID, *reply);
            } break;
            case proto::PEERREQUEST_OUTBAILMENT: {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Received outbailment reply." << std::endl;
                process_request_outbailment(lock, nymID, *reply);
            } break;
            case proto::PEERREQUEST_CONNECTIONINFO: {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Received connection info reply." << std::endl;
                process_connection_info(lock, nymID, *reply);
            } break;
            case proto::PEERREQUEST_STORESECRET: {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Received store secret reply." << std::endl;
                process_store_secret(lock, nymID, *reply);
            } break;
            case proto::PEERREQUEST_ERROR:
            case proto::PEERREQUEST_PENDINGBAILMENT:
            case proto::PEERREQUEST_VERIFICATIONOFFER:
            case proto::PEERREQUEST_FAUCET:
            default: {
                continue;
            }
        }
    }
}

void Pair::process_peer_requests(const Lock& lock, const Identifier& nymID)
    const
{
    OT_ASSERT(verify_lock(lock, peer_lock_));

    const auto requests = client_.Wallet().PeerRequestIncoming(nymID);

    for (const auto& it : requests) {
        const auto requestID = Identifier::Factory(it.first);
        std::time_t time{};
        const auto request = client_.Wallet().PeerRequest(
            nymID, requestID, StorageBox::INCOMINGPEERREQUEST, time);

        if (false == bool(request)) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed to load peer request " << it.first << std::endl;

            continue;
        }

        const auto& type = request->type();

        switch (type) {
            case proto::PEERREQUEST_PENDINGBAILMENT: {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Received pending bailment notification."
                      << std::endl;
                process_pending_bailment(lock, nymID, *request);
            } break;
            case proto::PEERREQUEST_ERROR:
            case proto::PEERREQUEST_BAILMENT:
            case proto::PEERREQUEST_OUTBAILMENT:
            case proto::PEERREQUEST_CONNECTIONINFO:
            case proto::PEERREQUEST_STORESECRET:
            case proto::PEERREQUEST_VERIFICATIONOFFER:
            case proto::PEERREQUEST_FAUCET:
            default: {

                continue;
            }
        }
    }
}

void Pair::process_pending_bailment(
    const Lock& lock,
    const Identifier& nymID,
    const proto::PeerRequest& request) const
{
    OT_ASSERT(verify_lock(lock, peer_lock_))
    OT_ASSERT(nymID == Identifier::Factory(request.recipient()))
    OT_ASSERT(proto::PEERREQUEST_PENDINGBAILMENT == request.type())

    const auto requestID = Identifier::Factory(request.id());
    const auto issuerNymID = Identifier::Factory(request.initiator());
    const auto serverID = Identifier::Factory(request.server());
    auto editor = client_.Wallet().mutable_Issuer(nymID, issuerNymID);
    auto& issuer = editor.It();
    const auto added =
        issuer.AddRequest(proto::PEERREQUEST_PENDINGBAILMENT, requestID);

    if (added) {
        pending_bailment_->Publish(proto::ProtoAsString(request));
        const OTIdentifier originalRequest =
            Identifier::Factory(request.pendingbailment().requestid());
        if (!originalRequest->empty()) {
            issuer.SetUsed(proto::PEERREQUEST_BAILMENT, originalRequest, true);
        } else {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed to set request as used on issuer." << std::endl;
        }

        auto action = client_.ServerAction().AcknowledgeNotice(
            nymID, serverID, issuerNymID, requestID, true);
        action->Run();

        if (SendResult::VALID_REPLY == action->LastSendResult()) {
            OT_ASSERT(action->SentPeerReply())

            const auto replyID(action->SentPeerReply()->ID());
            issuer.AddReply(
                proto::PEERREQUEST_PENDINGBAILMENT, requestID, replyID);
            update_.Push(Identifier::Random(), true);
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to add request."
              << std::endl;
    }
}

void Pair::process_request_bailment(
    const Lock& lock,
    const Identifier& nymID,
    const proto::PeerReply& reply) const
{
    OT_ASSERT(verify_lock(lock, peer_lock_))
    OT_ASSERT(nymID == Identifier::Factory(reply.initiator()))
    OT_ASSERT(proto::PEERREQUEST_BAILMENT == reply.type())

    const auto requestID = Identifier::Factory(reply.cookie());
    const auto replyID = Identifier::Factory(reply.id());
    const auto issuerNymID = Identifier::Factory(reply.recipient());
    auto editor = client_.Wallet().mutable_Issuer(nymID, issuerNymID);
    auto& issuer = editor.It();
    const auto added =
        issuer.AddReply(proto::PEERREQUEST_BAILMENT, requestID, replyID);

    if (added) {
        client_.Wallet().PeerRequestComplete(nymID, replyID);
        update_.Push(Identifier::Random(), true);
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to add reply."
              << std::endl;
    }
}

void Pair::process_request_outbailment(
    const Lock& lock,
    const Identifier& nymID,
    const proto::PeerReply& reply) const
{
    OT_ASSERT(verify_lock(lock, peer_lock_))
    OT_ASSERT(nymID == Identifier::Factory(reply.initiator()))
    OT_ASSERT(proto::PEERREQUEST_OUTBAILMENT == reply.type())

    const auto requestID = Identifier::Factory(reply.cookie());
    const auto replyID = Identifier::Factory(reply.id());
    const auto issuerNymID = Identifier::Factory(reply.recipient());
    auto editor = client_.Wallet().mutable_Issuer(nymID, issuerNymID);
    auto& issuer = editor.It();
    const auto added =
        issuer.AddReply(proto::PEERREQUEST_OUTBAILMENT, requestID, replyID);

    if (added) {
        client_.Wallet().PeerRequestComplete(nymID, replyID);
        update_.Push(Identifier::Random(), true);
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to add reply."
              << std::endl;
    }
}

void Pair::process_store_secret(
    const Lock& lock,
    const Identifier& nymID,
    const proto::PeerReply& reply) const
{
    OT_ASSERT(verify_lock(lock, peer_lock_))
    OT_ASSERT(nymID == Identifier::Factory(reply.initiator()))
    OT_ASSERT(proto::PEERREQUEST_STORESECRET == reply.type())

    const auto requestID = Identifier::Factory(reply.cookie());
    const auto replyID = Identifier::Factory(reply.id());
    const auto issuerNymID = Identifier::Factory(reply.recipient());
    auto editor = client_.Wallet().mutable_Issuer(nymID, issuerNymID);
    auto& issuer = editor.It();
    const auto added =
        issuer.AddReply(proto::PEERREQUEST_STORESECRET, requestID, replyID);

    if (added) {
        client_.Wallet().PeerRequestComplete(nymID, replyID);
        update_.Push(Identifier::Random(), true);
        proto::PairEvent event;
        event.set_version(1);
        event.set_type(proto::PAIREVENT_STORESECRET);
        event.set_issuer(issuerNymID->str());
        const auto published =
            pair_event_->Publish(proto::ProtoAsString(event));

        if (published) {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Published store secret notification.")
                .Flush();
        } else {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Error Publishing store secret notification."
                  << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to add reply."
              << std::endl;
    }
}

void Pair::queue_nym_download(
    const Identifier& localNymID,
    const Identifier& targetNymID) const
{
    client_.Sync().StartIntroductionServer(localNymID);
    client_.Sync().FindNym(targetNymID);
}

void Pair::queue_nym_registration(
    const Identifier& nymID,
    const Identifier& serverID,
    const bool setData) const
{
    client_.Sync().RegisterNym(nymID, serverID, setData);
}

void Pair::queue_server_contract(
    const Identifier& nymID,
    const Identifier& serverID) const
{
    client_.Sync().StartIntroductionServer(nymID);
    client_.Sync().FindServer(serverID);
}

void Pair::queue_unit_definition(
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& unitID) const
{
    client_.Sync().ScheduleDownloadContract(nymID, serverID, unitID);
}

void Pair::refresh() const
{
    update_pairing();
    update_peer();
}

std::pair<bool, OTIdentifier> Pair::register_account(
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& unitID) const
{
    std::pair<bool, OTIdentifier> output{false, Identifier::Factory()};
    auto& [success, accountID] = output;
    const auto contract = client_.Wallet().UnitDefinition(unitID);

    if (false == bool(contract)) {
        queue_unit_definition(nymID, serverID, unitID);

        return output;
    }

    auto action =
        client_.ServerAction().RegisterAccount(nymID, serverID, unitID);
    action->Run();

    if (SendResult::VALID_REPLY != action->LastSendResult()) { return output; }

    OT_ASSERT(action->Reply());

    const auto& reply = *action->Reply();

    success = reply.m_bSuccess;
    accountID->SetString(reply.m_strAcctID);

    return output;
}

void Pair::state_machine(
    const Identifier& localNymID,
    const Identifier& issuerNymID) const
{
    LogDetail(OT_METHOD)(__FUNCTION__)(": Local nym: ")(localNymID)(
        "Issuer Nym: ")(issuerNymID)
        .Flush();
    Lock lock(status_lock_);
    auto& [status, trusted] = pair_status_[{localNymID, issuerNymID}];
    lock.unlock();
    const auto issuerNym = client_.Wallet().Nym(issuerNymID);

    if (false == bool(issuerNym)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Issuer nym not yet downloaded."
              << std::endl;
        queue_nym_download(localNymID, issuerNymID);
        status = Status::Error;

        return;
    }

    SHUTDOWN()

    const auto& issuerClaims = issuerNym->Claims();
    const auto serverID = issuerClaims.PreferredOTServer();
    const auto contractSection =
        issuerClaims.Section(proto::CONTACTSECTION_CONTRACT);
    const auto haveAccounts = bool(contractSection);

    if (serverID->empty()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Issuer nym does not advertise a server." << std::endl;
        // Maybe there's a new version
        queue_nym_download(localNymID, issuerNymID);
        status = Status::Error;

        return;
    }

    SHUTDOWN()

    if (false == haveAccounts) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Issuer does not advertise any contracts." << std::endl;
    } else {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Issuer advertise ")(
            contractSection->Size())(" contracts.")
            .Flush();
    }

    auto editor = client_.Wallet().mutable_Issuer(localNymID, issuerNymID);
    auto& issuer = editor.It();
    trusted = issuer.Paired();
    bool needStoreSecret{false};

    SHUTDOWN()

    switch (status) {
        case Status::Error: {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": First pass through state machine.")
                .Flush();
            status = Status::Started;
            [[fallthrough]];
        }
        case Status::Started: {
            if (need_registration(localNymID, serverID)) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Local nym not registered on issuer's notary."
                      << std::endl;
                auto contract = client_.Wallet().Server(serverID);

                SHUTDOWN()

                if (false == bool(contract)) {
                    otErr << OT_METHOD << __FUNCTION__
                          << ": Waiting on server contract." << std::endl;
                    queue_server_contract(localNymID, serverID);

                    return;
                }

                SHUTDOWN()

                queue_nym_registration(localNymID, serverID, trusted);

                return;
            } else {
                status = Status::Registered;
            }

            [[fallthrough]];
        }
        case Status::Registered: {
            SHUTDOWN()

            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Local nym is registered on issuer's notary.")
                .Flush();

            if (trusted) {
                needStoreSecret = (false == issuer.StoreSecretComplete()) &&
                                  (false == issuer.StoreSecretInitiated());
                auto editor = client_.Wallet().mutable_ServerContext(
                    localNymID, serverID);
                auto& context = editor.It();

                if (context.AdminPassword() != issuer.PairingCode()) {
                    context.SetAdminPassword(issuer.PairingCode());
                }

                if (context.ShouldRename()) {
                    proto::PairEvent event;
                    event.set_version(1);
                    event.set_type(proto::PAIREVENT_RENAME);
                    event.set_issuer(issuerNymID.str());
                    const auto published =
                        pair_event_->Publish(proto::ProtoAsString(event));

                    if (published) {
                        LogDetail(OT_METHOD)(__FUNCTION__)(
                            ": Published should rename notification.")
                            .Flush();
                    } else {
                        otErr
                            << OT_METHOD << __FUNCTION__
                            << ": Error publishing should rename notification."
                            << std::endl;
                    }
                }
            }

            SHUTDOWN()

            if (needStoreSecret) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Sending store secret peer request" << std::endl;
                const auto [sent, requestID] =
                    store_secret(localNymID, issuerNymID, serverID);

                if (sent) {
                    issuer.AddRequest(
                        proto::PEERREQUEST_STORESECRET, requestID);
                }
            }

            SHUTDOWN()

            if (trusted) {
                const auto btcrpc =
                    issuer.ConnectionInfo(proto::CONNECTIONINFO_BTCRPC);
                const bool needInfo =
                    (btcrpc.empty() &&
                     (false == issuer.ConnectionInfoInitiated(
                                   proto::CONNECTIONINFO_BTCRPC)));

                if (needInfo) {
                    otErr << OT_METHOD << __FUNCTION__
                          << ": Sending connection info peer request"
                          << std::endl;
                    const auto [sent, requestID] = get_connection(
                        localNymID,
                        issuerNymID,
                        serverID,
                        proto::CONNECTIONINFO_BTCRPC);

                    if (sent) {
                        issuer.AddRequest(
                            proto::PEERREQUEST_CONNECTIONINFO, requestID);
                    }
                }
            }

            if (haveAccounts) {
                for (const auto& [type, pGroup] : *contractSection) {
                    SHUTDOWN()
                    OT_ASSERT(pGroup);

                    const auto& group = *pGroup;

                    for (const auto& [id, pClaim] : group) {
                        SHUTDOWN()
                        OT_ASSERT(pClaim);

                        const auto& notUsed[[maybe_unused]] = id;
                        const auto& claim = *pClaim;
                        const OTIdentifier unitID =
                            Identifier::Factory(claim.Value());
                        const auto accountList =
                            issuer.AccountList(type, unitID);

                        if (0 == accountList.size()) {
                            otErr << OT_METHOD << __FUNCTION__
                                  << ": Registering " << unitID->str()
                                  << " account for " << localNymID.str()
                                  << " on " << serverID->str() << std::endl;
                            const auto& [registered, accountID] =
                                register_account(localNymID, serverID, unitID);

                            if (registered) {
                                issuer.AddAccount(type, unitID, accountID);
                            } else {
                                continue;
                            }
                        } else {
                            LogDetail(OT_METHOD)(__FUNCTION__)(": ")(unitID)(
                                " account for ")(localNymID)(" on ")(serverID)(
                                " already exists.")
                                .Flush();
                        }

                        const auto instructions =
                            issuer.BailmentInstructions(unitID);
                        const bool needBailment =
                            (MINIMUM_UNUSED_BAILMENTS > instructions.size());
                        const bool nonePending =
                            (false == issuer.BailmentInitiated(unitID));

                        if (needBailment && nonePending) {
                            otErr << OT_METHOD << __FUNCTION__
                                  << ": Requesting bailment info for "
                                  << String::Factory(unitID) << std::endl;
                            const auto& [sent, requestID] = initiate_bailment(
                                localNymID, serverID, issuerNymID, unitID);

                            if (sent) {
                                issuer.AddRequest(
                                    proto::PEERREQUEST_BAILMENT, requestID);
                            }
                        }
                    }
                }
            }
        }
        default: {
        }
    }
}

std::pair<bool, OTIdentifier> Pair::store_secret(
    const Identifier& localNymID,
    const Identifier& issuerNymID,
    const Identifier& serverID) const
{
    std::pair<bool, OTIdentifier> output{false, Identifier::Factory()};
    auto& [success, requestID] = output;
    auto action = client_.ServerAction().InitiateStoreSecret(
        localNymID,
        serverID,
        issuerNymID,
        proto::SECRETTYPE_BIP39,
        client_.Exec().Wallet_GetWords(),
        client_.Exec().Wallet_GetPassphrase());
    action->Run();

    if (SendResult::VALID_REPLY != action->LastSendResult()) { return output; }

    OT_ASSERT(action->Reply());

    success = action->Reply()->m_bSuccess;

    OT_ASSERT(action->SentPeerRequest());

    requestID = action->SentPeerRequest()->ID();

    return output;
}

void Pair::Update() const { update_.Push(Identifier::Random(), true); }

void Pair::update_pairing() const
{
    const auto pairing = pairing_->Set(true);

    if (false == pairing) {
        if (pairing_thread_) {
            pairing_thread_->join();
            pairing_thread_.reset();
        }

        pairing_thread_.reset(new std::thread(&Pair::check_pairing, this));
    }
}

void Pair::update_peer() const
{
    Lock lock(peer_lock_);

    for (const auto& [nymID, issuerSet] : create_issuer_map()) {
        const auto& notUsed[[maybe_unused]] = issuerSet;
        process_peer_replies(lock, nymID);
        process_peer_requests(lock, nymID);
    }
}

Pair::~Pair()
{
    if (pairing_.get()) { Log::Sleep(std::chrono::milliseconds(250)); }

    if (refresh_thread_) {
        refresh_thread_->join();
        refresh_thread_.reset();
    }

    if (pairing_thread_) {
        pairing_thread_->join();
        pairing_thread_.reset();
    }
}
}  // namespace opentxs::api::client::implementation

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::contract::peer::ConnectionInfoType
// IWYU pragma: no_forward_declare opentxs::identity::wot::claim::SectionType
// IWYU pragma: no_forward_declare opentxs::otx::LastReplyStatus

#include "otx/client/Pair.hpp"  // IWYU pragma: associated

#include <PairEvent.pb.h>
#include <PeerReply.pb.h>
#include <PeerRequest.pb.h>
#include <PendingBailment.pb.h>
#include <ZMQEnums.pb.h>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iterator>
#include <memory>
#include <ratio>
#include <string_view>

#include "core/StateMachine.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/Core.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/core/contract/peer/Peer.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "internal/otx/client/Factory.hpp"
#include "internal/otx/client/Issuer.hpp"
#include "internal/otx/client/Pair.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/consensus/Server.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/PeerReply.hpp"
#include "internal/serialization/protobuf/verify/PeerRequest.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/Lockable.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/contract/peer/PeerRequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/SecretType.hpp"       // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/wot/claim/Data.hpp"
#include "opentxs/identity/wot/claim/Group.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/identity/wot/claim/Section.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"  // IWYU pragma: keep
#include "opentxs/otx/Types.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

#define MINIMUM_UNUSED_BAILMENTS 3

#define PAIR_SHUTDOWN()                                                        \
    if (!running_) { return; }                                                 \
                                                                               \
    Sleep(50ms)

namespace opentxs::factory
{
auto PairAPI(const Flag& running, const api::session::Client& client)
    -> otx::client::Pair*
{
    using ReturnType = otx::client::implementation::Pair;

    return new ReturnType(running, client);
}
}  // namespace opentxs::factory

namespace opentxs::otx::client::implementation
{
Pair::Pair(const Flag& running, const api::session::Client& client)
    : otx::client::Pair()
    , Lockable()
    , StateMachine([this]() -> bool {
        return state_.run(
            [this](const auto& id) -> void { state_machine(id); });
    })
    , running_(running)
    , client_(client)
    , state_(decision_lock_)
    , startup_promise_()
    , startup_(startup_promise_.get_future())
    , nym_callback_(zmq::ListenCallback::Factory(
          [this](const auto& in) -> void { callback_nym(in); }))
    , peer_reply_callback_(zmq::ListenCallback::Factory(
          [this](const auto& in) -> void { callback_peer_reply(in); }))
    , peer_request_callback_(zmq::ListenCallback::Factory(
          [this](const auto& in) -> void { callback_peer_request(in); }))
    , pair_event_(client_.Network().ZeroMQ().Internal().PublishSocket())
    , pending_bailment_(client_.Network().ZeroMQ().Internal().PublishSocket())
    , nym_subscriber_(client_.Network().ZeroMQ().Internal().SubscribeSocket(
          nym_callback_,
          "Pair nym"))
    , peer_reply_subscriber_(
          client_.Network().ZeroMQ().Internal().SubscribeSocket(
              peer_reply_callback_,
              "Pair reply"))
    , peer_request_subscriber_(
          client_.Network().ZeroMQ().Internal().SubscribeSocket(
              peer_request_callback_,
              "Pair request"))
{
    // WARNING: do not access client_.Wallet() during construction
    pair_event_->Start(client_.Endpoints().PairEvent().data());
    pending_bailment_->Start(client_.Endpoints().PendingBailment().data());
    nym_subscriber_->Start(client_.Endpoints().NymDownload().data());
    peer_reply_subscriber_->Start(client_.Endpoints().PeerReplyUpdate().data());
    peer_request_subscriber_->Start(
        client_.Endpoints().PeerRequestUpdate().data());
}

Pair::State::State(std::mutex& lock) noexcept
    : lock_(lock)
    , state_()
    , issuers_()
{
}

void Pair::State::Add(
    const Lock& lock,
    const identifier::Nym& localNymID,
    const identifier::Nym& issuerNymID,
    const bool trusted) noexcept
{
    OT_ASSERT(CheckLock(lock, lock_));

    issuers_.emplace(issuerNymID);  // copy, then move
    state_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(
            identifier::Nym{localNymID}, identifier::Nym{issuerNymID}),
        std::forward_as_tuple(
            std::make_unique<std::mutex>(),
            identifier::Notary{},
            identifier::Nym{},
            Status::Error,
            trusted,
            0,
            0,
            UnallocatedVector<AccountDetails>{},
            UnallocatedVector<api::session::OTX::BackgroundTask>{},
            false));
}

void Pair::State::Add(
    const identifier::Nym& localNymID,
    const identifier::Nym& issuerNymID,
    const bool trusted) noexcept
{
    Lock lock(lock_);
    Add(lock,
        identifier::Nym{localNymID},
        identifier::Nym{issuerNymID},
        trusted);
}

auto Pair::State::CheckIssuer(const identifier::Nym& id) const noexcept -> bool
{
    Lock lock(lock_);

    return 0 < issuers_.count(id);
}

auto Pair::State::check_state() const noexcept -> bool
{
    Lock lock(lock_);

    for (auto& [id, details] : state_) {
        auto& [mutex, serverID, serverNymID, status, trusted, offered, registered, accountDetails, pending, needRename] =
            details;

        OT_ASSERT(mutex);

        Lock rowLock(*mutex);

        if (Status::Registered != status) {
            LogTrace()(OT_PRETTY_CLASS())("Not registered").Flush();

            goto repeat;
        }

        if (needRename) {
            LogTrace()(OT_PRETTY_CLASS())("Notary name not set").Flush();

            goto repeat;
        }

        const auto accountCount = count_currencies(accountDetails);

        if (accountCount != offered) {
            LogTrace()(OT_PRETTY_CLASS())(
                ": Waiting for account registration, "
                "expected: ")(offered)(", have ")(accountCount)
                .Flush();

            goto repeat;
        }

        for (const auto& [unit, account, bailments] : accountDetails) {
            if (bailments < MINIMUM_UNUSED_BAILMENTS) {
                LogTrace()(OT_PRETTY_CLASS())(
                    ": Waiting for bailment instructions for "
                    "account ")(account)(", "
                                         "expected:"
                                         " ")(MINIMUM_UNUSED_BAILMENTS)(", "
                                                                        "have"
                                                                        " ")(
                    bailments)
                    .Flush();

                goto repeat;
            }
        }
    }

    // No reason to continue executing state machine
    LogTrace()(OT_PRETTY_CLASS())("Done").Flush();

    return false;

repeat:
    lock.unlock();
    LogTrace()(OT_PRETTY_CLASS())("Repeating").Flush();
    // Rate limit state machine to reduce unproductive execution while waiting
    // on network activity
    Sleep(50ms);

    return true;
}

auto Pair::State::count_currencies(
    const UnallocatedVector<AccountDetails>& in) noexcept -> std::size_t
{
    auto unique = UnallocatedSet<identifier::UnitDefinition>{};
    std::transform(
        std::begin(in),
        std::end(in),
        std::inserter(unique, unique.end()),
        [](const auto& item) -> identifier::UnitDefinition {
            return std::get<0>(item);
        });

    return unique.size();
}

auto Pair::State::count_currencies(
    const api::Session& api,
    const identity::wot::claim::Section& in) noexcept -> std::size_t
{
    auto unique = UnallocatedSet<identifier::UnitDefinition>{};

    for (const auto& [type, pGroup] : in) {
        OT_ASSERT(pGroup);

        const auto& group = *pGroup;

        for (const auto& [id, pClaim] : group) {
            OT_ASSERT(pClaim);

            const auto& claim = *pClaim;
            unique.emplace(api.Factory().UnitIDFromBase58(claim.Value()));
        }
    }

    return unique.size();
}

auto Pair::State::get_account(
    const api::session::Client& client,
    const identifier::UnitDefinition& unit,
    const identifier::Generic& account,
    UnallocatedVector<AccountDetails>& details) noexcept -> AccountDetails&
{
    OT_ASSERT(false == unit.empty());
    OT_ASSERT(false == account.empty());

    for (auto& row : details) {
        const auto& [unitID, accountID, bailment] = row;
        const auto match = (unit.asBase58(client.Crypto()) ==
                            unitID.asBase58(client.Crypto())) &&
                           (account.asBase58(client.Crypto()) ==
                            accountID.asBase58(client.Crypto()));

        if (match) { return row; }
    }

    return details.emplace_back(AccountDetails{unit, account, 0});
}

auto Pair::State::GetDetails(
    const identifier::Nym& localNymID,
    const identifier::Nym& issuerNymID) noexcept -> StateMap::iterator
{
    Lock lock(lock_);

    return state_.find({localNymID, issuerNymID});
}

auto Pair::State::IssuerList(
    const identifier::Nym& localNymID,
    const bool onlyTrusted) const noexcept -> UnallocatedSet<identifier::Nym>
{
    Lock lock(lock_);
    UnallocatedSet<identifier::Nym> output{};

    for (auto& [key, value] : state_) {
        auto& pMutex = std::get<0>(value);

        OT_ASSERT(pMutex);

        Lock rowLock(*pMutex);
        const auto& issuerID = std::get<1>(key);
        const auto& trusted = std::get<4>(value);

        if (trusted || (false == onlyTrusted)) { output.emplace(issuerID); }
    }

    return output;
}

auto Pair::State::run(const std::function<void(const IssuerID&)> fn) noexcept
    -> bool
{
    auto list = UnallocatedSet<IssuerID>{};

    {
        Lock lock(lock_);
        std::transform(
            std::begin(state_),
            std::end(state_),
            std::inserter(list, list.end()),
            [](const auto& in) -> IssuerID { return in.first; });
    }

    std::for_each(std::begin(list), std::end(list), fn);

    return check_state();
}

auto Pair::AddIssuer(
    const identifier::Nym& localNymID,
    const identifier::Nym& issuerNymID,
    const UnallocatedCString& pairingCode) const noexcept -> bool
{
    if (localNymID.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid local nym id.").Flush();

        return false;
    }

    if (!client_.Wallet().IsLocalNym(localNymID.asBase58(client_.Crypto()))) {
        LogError()(OT_PRETTY_CLASS())("Invalid local nym.").Flush();

        return false;
    }

    if (issuerNymID.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid issuer nym id.").Flush();

        return false;
    }

    if (blockchain::Type::Unknown != blockchain::Chain(client_, issuerNymID)) {
        LogError()(OT_PRETTY_CLASS())(
            ": blockchains can not be used as otx issuers.")
            .Flush();

        return false;
    }

    bool trusted{false};

    {
        auto editor =
            client_.Wallet().Internal().mutable_Issuer(localNymID, issuerNymID);
        auto& issuer = editor.get();
        const bool needPairingCode = issuer.PairingCode().empty();
        const bool havePairingCode = (false == pairingCode.empty());

        if (havePairingCode && needPairingCode) {
            issuer.SetPairingCode(pairingCode);
        }

        trusted = issuer.Paired();
    }

    state_.Add(localNymID, issuerNymID, trusted);
    Trigger();

    return true;
}

void Pair::callback_nym(const zmq::Message& in) noexcept
{
    startup_.get();
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    const auto nymID = client_.Factory().NymIDFromHash(body.at(1).Bytes());
    auto trigger{state_.CheckIssuer(nymID)};

    {
        Lock lock(decision_lock_);

        for (auto& [id, details] : state_) {
            auto& [mutex, serverID, serverNymID, status, trusted, offered, registered, accountDetails, pending, needRename] =
                details;

            OT_ASSERT(mutex);

            Lock rowLock(*mutex);

            if (serverNymID == nymID) { trigger = true; }
        }
    }

    if (trigger) { Trigger(); }
}

void Pair::callback_peer_reply(const zmq::Message& in) noexcept
{
    startup_.get();
    const auto body = in.Body();

    OT_ASSERT(2 <= body.size());

    const auto nymID = client_.Factory().NymIDFromBase58(body.at(0).Bytes());
    const auto reply = proto::Factory<proto::PeerReply>(body.at(1));
    auto trigger{false};

    if (false == proto::Validate(reply, VERBOSE)) { return; }

    switch (translate(reply.type())) {
        case contract::peer::PeerRequestType::Bailment: {
            LogDetail()(OT_PRETTY_CLASS())("Received bailment reply.").Flush();
            Lock lock(decision_lock_);
            trigger = process_request_bailment(lock, nymID, reply);
        } break;
        case contract::peer::PeerRequestType::OutBailment: {
            LogDetail()(OT_PRETTY_CLASS())("Received outbailment reply.")
                .Flush();
            Lock lock(decision_lock_);
            trigger = process_request_outbailment(lock, nymID, reply);
        } break;
        case contract::peer::PeerRequestType::ConnectionInfo: {
            LogDetail()(OT_PRETTY_CLASS())(": Received connection info reply.")
                .Flush();
            Lock lock(decision_lock_);
            trigger = process_connection_info(lock, nymID, reply);
        } break;
        case contract::peer::PeerRequestType::StoreSecret: {
            LogDetail()(OT_PRETTY_CLASS())("Received store secret reply.")
                .Flush();
            Lock lock(decision_lock_);
            trigger = process_store_secret(lock, nymID, reply);
        } break;
        case contract::peer::PeerRequestType::Error:
        case contract::peer::PeerRequestType::PendingBailment:
        case contract::peer::PeerRequestType::VerificationOffer:
        case contract::peer::PeerRequestType::Faucet:
        default: {
        }
    }

    if (trigger) { Trigger(); }
}

void Pair::callback_peer_request(const zmq::Message& in) noexcept
{
    startup_.get();
    const auto body = in.Body();

    OT_ASSERT(2 <= body.size());

    const auto nymID = client_.Factory().NymIDFromBase58(body.at(0).Bytes());
    const auto request = proto::Factory<proto::PeerRequest>(body.at(1));
    auto trigger{false};

    if (false == proto::Validate(request, VERBOSE)) { return; }

    switch (translate(request.type())) {
        case contract::peer::PeerRequestType::PendingBailment: {
            Lock lock(decision_lock_);
            trigger = process_pending_bailment(lock, nymID, request);
        } break;
        default: {
        }
    }

    if (trigger) { Trigger(); }
}

void Pair::check_accounts(
    const identity::wot::claim::Data& issuerClaims,
    otx::client::Issuer& issuer,
    const identifier::Notary& serverID,
    std::size_t& offered,
    std::size_t& registeredAccounts,
    UnallocatedVector<Pair::State::AccountDetails>& accountDetails)
    const noexcept
{
    const auto& localNymID = issuer.LocalNymID();
    const auto& issuerNymID = issuer.IssuerID();
    const auto contractSection =
        issuerClaims.Section(identity::wot::claim::SectionType::Contract);
    const auto haveAccounts = bool(contractSection);

    if (false == haveAccounts) {
        LogError()(OT_PRETTY_CLASS())(
            ": Issuer does not advertise any contracts.")
            .Flush();
    } else {
        offered = State::count_currencies(client_, *contractSection);
        LogDetail()(OT_PRETTY_CLASS())("Issuer advertises ")(
            offered)(" contract")((1 == offered) ? "." : "s.")
            .Flush();
    }

    auto uniqueRegistered = UnallocatedSet<identifier::UnitDefinition>{};

    if (false == haveAccounts) { return; }

    for (const auto& [type, pGroup] : *contractSection) {
        PAIR_SHUTDOWN();
        OT_ASSERT(pGroup);

        const auto& group = *pGroup;

        for (const auto& [id, pClaim] : group) {
            PAIR_SHUTDOWN();
            OT_ASSERT(pClaim);

            const auto& notUsed [[maybe_unused]] = id;
            const auto& claim = *pClaim;
            const auto unitID =
                client_.Factory().UnitIDFromBase58(claim.Value());

            if (unitID.empty()) {
                LogDetail()(OT_PRETTY_CLASS())("Invalid unit definition")
                    .Flush();

                continue;
            }

            const auto accountList =
                issuer.AccountList(ClaimToUnit(type), unitID);

            if (0 == accountList.size()) {
                LogDetail()(OT_PRETTY_CLASS())("Registering ")(
                    unitID)(" account for ")(localNymID)(" on"
                                                         " ")(serverID)(".")
                    .Flush();
                const auto& [registered, accountid] =
                    register_account(localNymID, serverID, unitID);

                if (registered) {
                    LogDetail()(OT_PRETTY_CLASS())(
                        ": Success registering account")
                        .Flush();
                    issuer.AddAccount(ClaimToUnit(type), unitID, accountid);
                } else {
                    LogError()(OT_PRETTY_CLASS())(
                        ": Failed to register account")
                        .Flush();
                }

                continue;
            } else {
                LogDetail()(OT_PRETTY_CLASS())(unitID)(" account for ")(
                    localNymID)(" on ")(serverID)(" already exists.")
                    .Flush();
            }

            for (const auto& accountID : accountList) {
                auto& details = State::get_account(
                    client_, unitID, accountID, accountDetails);
                uniqueRegistered.emplace(unitID);
                auto& bailmentCount = std::get<2>(details);
                const auto instructions =
                    issuer.BailmentInstructions(client_, unitID);
                bailmentCount = instructions.size();
                const bool needBailment =
                    (MINIMUM_UNUSED_BAILMENTS > instructions.size());
                const bool nonePending =
                    (false == issuer.BailmentInitiated(unitID));

                if (needBailment && nonePending) {
                    LogDetail()(OT_PRETTY_CLASS())(
                        ": Requesting bailment info for ")(unitID)(".")
                        .Flush();
                    const auto& [sent, requestID] = initiate_bailment(
                        localNymID, serverID, issuerNymID, unitID);

                    if (sent) {
                        issuer.AddRequest(
                            contract::peer::PeerRequestType::Bailment,
                            requestID);
                    }
                }
            }
        }
    }

    registeredAccounts = uniqueRegistered.size();
}

void Pair::check_connection_info(
    otx::client::Issuer& issuer,
    const identifier::Notary& serverID) const noexcept
{
    const auto trusted = issuer.Paired();

    if (false == trusted) { return; }

    const auto btcrpc = issuer.ConnectionInfo(
        client_, contract::peer::ConnectionInfoType::BtcRpc);
    const bool needInfo =
        (btcrpc.empty() &&
         (false == issuer.ConnectionInfoInitiated(
                       contract::peer::ConnectionInfoType::BtcRpc)));

    if (needInfo) {
        LogDetail()(OT_PRETTY_CLASS())(
            ": Sending connection info peer request.")
            .Flush();
        const auto [sent, requestID] = get_connection(
            issuer.LocalNymID(),
            issuer.IssuerID(),
            serverID,
            contract::peer::ConnectionInfoType::BtcRpc);

        if (sent) {
            issuer.AddRequest(
                contract::peer::PeerRequestType::ConnectionInfo, requestID);
        }
    }
}

void Pair::check_rename(
    const otx::client::Issuer& issuer,
    const identifier::Notary& serverID,
    const PasswordPrompt& reason,
    bool& needRename) const noexcept
{
    if (false == issuer.Paired()) {
        LogTrace()(OT_PRETTY_CLASS())("Not trusted").Flush();

        return;
    }

    auto editor = client_.Wallet().Internal().mutable_ServerContext(
        issuer.LocalNymID(), serverID, reason);
    auto& context = editor.get();

    if (context.AdminPassword() != issuer.PairingCode()) {
        context.SetAdminPassword(issuer.PairingCode());
    }

    needRename = context.ShouldRename();

    if (needRename) {
        const auto published = pair_event_->Send([&] {
            auto out = opentxs::network::zeromq::Message{};
            out.Internal().AddFrame([&] {
                auto proto = proto::PairEvent{};
                proto.set_version(1);
                proto.set_type(proto::PAIREVENT_RENAME);
                proto.set_issuer(issuer.IssuerID().asBase58(client_.Crypto()));

                return proto;
            }());

            return out;
        }());

        if (published) {
            LogDetail()(OT_PRETTY_CLASS())(
                ": Published should rename notification.")
                .Flush();
        } else {
            LogError()(OT_PRETTY_CLASS())(
                ": Error publishing should rename notification.")
                .Flush();
        }
    } else {
        LogTrace()(OT_PRETTY_CLASS())("No reason to rename").Flush();
    }
}

void Pair::check_store_secret(
    otx::client::Issuer& issuer,
    const identifier::Notary& serverID) const noexcept
{
    if (false == issuer.Paired()) { return; }

    const auto needStoreSecret = (false == issuer.StoreSecretComplete()) &&
                                 (false == issuer.StoreSecretInitiated()) &&
                                 api::crypto::HaveHDKeys();

    if (needStoreSecret) {
        LogDetail()(OT_PRETTY_CLASS())("Sending store secret peer request.")
            .Flush();
        const auto [sent, requestID] =
            store_secret(issuer.LocalNymID(), issuer.IssuerID(), serverID);

        if (sent) {
            issuer.AddRequest(
                contract::peer::PeerRequestType::StoreSecret, requestID);
        }
    }
}

auto Pair::CheckIssuer(
    const identifier::Nym& localNymID,
    const identifier::UnitDefinition& unitDefinitionID) const noexcept -> bool
{
    try {
        const auto contract =
            client_.Wallet().Internal().UnitDefinition(unitDefinitionID);

        return AddIssuer(localNymID, contract->Nym()->ID(), "");
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())(
            ": Unit definition contract does not exist.")
            .Flush();

        return false;
    }
}

auto Pair::cleanup() const noexcept -> std::shared_future<void>
{
    peer_request_subscriber_->Close();
    peer_reply_subscriber_->Close();
    nym_subscriber_->Close();

    return StateMachine::Stop();
}

auto Pair::get_connection(
    const identifier::Nym& localNymID,
    const identifier::Nym& issuerNymID,
    const identifier::Notary& serverID,
    const contract::peer::ConnectionInfoType type) const
    -> std::pair<bool, identifier::Generic>
{
    auto output = std::pair<bool, identifier::Generic>{};
    auto& [success, requestID] = output;

    auto setID = [&](const identifier::Generic& in) -> void {
        output.second = in;
    };
    auto [taskID, future] = client_.OTX().InitiateRequestConnection(
        localNymID, serverID, issuerNymID, type, setID);

    if (0 == taskID) { return output; }

    const auto result = std::get<0>(future.get());
    success = (otx::LastReplyStatus::MessageSuccess == result);

    return output;
}

void Pair::init() noexcept
{
    Lock lock(decision_lock_);

    for (const auto& nymID : client_.Wallet().LocalNyms()) {
        for (const auto& issuerID : client_.Wallet().IssuerList(nymID)) {
            const auto pIssuer =
                client_.Wallet().Internal().Issuer(nymID, issuerID);

            OT_ASSERT(pIssuer);

            const auto& issuer = *pIssuer;
            state_.Add(lock, nymID, issuerID, issuer.Paired());
        }

        process_peer_replies(lock, nymID);
        process_peer_requests(lock, nymID);
    }

    lock.unlock();
    startup_promise_.set_value();
    Trigger();
}

auto Pair::initiate_bailment(
    const identifier::Nym& nymID,
    const identifier::Notary& serverID,
    const identifier::Nym& issuerID,
    const identifier::UnitDefinition& unitID) const
    -> std::pair<bool, identifier::Generic>
{
    auto output = std::pair<bool, identifier::Generic>{};
    auto& success = std::get<0>(output);

    try {
        client_.Wallet().Internal().UnitDefinition(unitID);
    } catch (...) {
        queue_unit_definition(nymID, serverID, unitID);

        return output;
    }

    auto setID = [&](const identifier::Generic& in) -> void {
        output.second = in;
    };
    auto [taskID, future] = client_.OTX().InitiateBailment(
        nymID, serverID, issuerID, unitID, setID);

    if (0 == taskID) { return output; }

    const auto result = std::get<0>(future.get());
    success = (otx::LastReplyStatus::MessageSuccess == result);

    return output;
}

auto Pair::IssuerDetails(
    const identifier::Nym& localNymID,
    const identifier::Nym& issuerNymID) const noexcept -> UnallocatedCString
{
    auto issuer = client_.Wallet().Internal().Issuer(localNymID, issuerNymID);

    if (false == bool(issuer)) { return {}; }

    return issuer->toString();
}

auto Pair::need_registration(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID) const -> bool
{
    auto context =
        client_.Wallet().Internal().ServerContext(localNymID, serverID);

    if (context) { return (0 == context->Request()); }

    return true;
}

auto Pair::process_connection_info(
    const Lock& lock,
    const identifier::Nym& nymID,
    const proto::PeerReply& reply) const -> bool
{
    OT_ASSERT(CheckLock(lock, decision_lock_));
    OT_ASSERT(
        nymID == client_.Factory().IdentifierFromBase58(reply.initiator()));
    OT_ASSERT(
        contract::peer::PeerRequestType::ConnectionInfo ==
        translate(reply.type()));

    const auto requestID =
        client_.Factory().IdentifierFromBase58(reply.cookie());
    const auto replyID = client_.Factory().IdentifierFromBase58(reply.id());
    const auto issuerNymID =
        client_.Factory().NymIDFromBase58(reply.recipient());
    auto editor =
        client_.Wallet().Internal().mutable_Issuer(nymID, issuerNymID);
    auto& issuer = editor.get();
    const auto added = issuer.AddReply(
        contract::peer::PeerRequestType::ConnectionInfo, requestID, replyID);

    if (added) {
        client_.Wallet().PeerRequestComplete(nymID, replyID);

        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to add reply.").Flush();

        return false;
    }
}

void Pair::process_peer_replies(const Lock& lock, const identifier::Nym& nymID)
    const
{
    OT_ASSERT(CheckLock(lock, decision_lock_));

    auto replies = client_.Wallet().PeerReplyIncoming(nymID);

    for (const auto& it : replies) {
        const auto replyID = client_.Factory().IdentifierFromBase58(it.first);
        auto reply = proto::PeerReply{};
        if (false == client_.Wallet().Internal().PeerReply(
                         nymID,
                         replyID,
                         otx::client::StorageBox::INCOMINGPEERREPLY,
                         reply)) {

            LogError()(OT_PRETTY_CLASS())("Failed to load peer reply ")(
                it.first)(".")
                .Flush();

            continue;
        }

        const auto& type = reply.type();

        switch (translate(type)) {
            case contract::peer::PeerRequestType::Bailment: {
                LogDetail()(OT_PRETTY_CLASS())("Received bailment reply.")
                    .Flush();
                process_request_bailment(lock, nymID, reply);
            } break;
            case contract::peer::PeerRequestType::OutBailment: {
                LogDetail()(OT_PRETTY_CLASS())(": Received outbailment reply.")
                    .Flush();
                process_request_outbailment(lock, nymID, reply);
            } break;
            case contract::peer::PeerRequestType::ConnectionInfo: {
                LogDetail()(OT_PRETTY_CLASS())(
                    ": Received connection info reply.")
                    .Flush();
                process_connection_info(lock, nymID, reply);
            } break;
            case contract::peer::PeerRequestType::StoreSecret: {
                LogDetail()(OT_PRETTY_CLASS())(": Received store secret reply.")
                    .Flush();
                process_store_secret(lock, nymID, reply);
            } break;
            case contract::peer::PeerRequestType::Error:
            case contract::peer::PeerRequestType::PendingBailment:
            case contract::peer::PeerRequestType::VerificationOffer:
            case contract::peer::PeerRequestType::Faucet:
            default: {
                continue;
            }
        }
    }
}

void Pair::process_peer_requests(const Lock& lock, const identifier::Nym& nymID)
    const
{
    OT_ASSERT(CheckLock(lock, decision_lock_));

    const auto requests = client_.Wallet().PeerRequestIncoming(nymID);

    for (const auto& it : requests) {
        const auto requestID = client_.Factory().IdentifierFromBase58(it.first);
        std::time_t time{};
        auto request = proto::PeerRequest{};
        if (false == client_.Wallet().Internal().PeerRequest(
                         nymID,
                         requestID,
                         otx::client::StorageBox::INCOMINGPEERREQUEST,
                         time,
                         request)) {

            LogError()(OT_PRETTY_CLASS())("Failed to load peer request ")(
                it.first)(".")
                .Flush();

            continue;
        }

        const auto& type = request.type();

        switch (translate(type)) {
            case contract::peer::PeerRequestType::PendingBailment: {
                LogError()(OT_PRETTY_CLASS())(
                    ": Received pending bailment notification.")
                    .Flush();
                process_pending_bailment(lock, nymID, request);
            } break;
            case contract::peer::PeerRequestType::Error:
            case contract::peer::PeerRequestType::Bailment:
            case contract::peer::PeerRequestType::OutBailment:
            case contract::peer::PeerRequestType::ConnectionInfo:
            case contract::peer::PeerRequestType::StoreSecret:
            case contract::peer::PeerRequestType::VerificationOffer:
            case contract::peer::PeerRequestType::Faucet:
            default: {

                continue;
            }
        }
    }
}

auto Pair::process_pending_bailment(
    const Lock& lock,
    const identifier::Nym& nymID,
    const proto::PeerRequest& request) const -> bool
{
    OT_ASSERT(CheckLock(lock, decision_lock_));
    OT_ASSERT(
        nymID == client_.Factory().IdentifierFromBase58(request.recipient()));
    OT_ASSERT(
        contract::peer::PeerRequestType::PendingBailment ==
        translate(request.type()));

    const auto requestID = client_.Factory().IdentifierFromBase58(request.id());
    const auto issuerNymID =
        client_.Factory().NymIDFromBase58(request.initiator());
    const auto serverID =
        client_.Factory().NotaryIDFromBase58(request.server());
    auto editor =
        client_.Wallet().Internal().mutable_Issuer(nymID, issuerNymID);
    auto& issuer = editor.get();
    const auto added = issuer.AddRequest(
        contract::peer::PeerRequestType::PendingBailment, requestID);

    if (added) {
        pending_bailment_->Send([&] {
            auto out = opentxs::network::zeromq::Message{};
            out.Internal().AddFrame(request);

            return out;
        }());
        const identifier::Generic originalRequest =
            client_.Factory().IdentifierFromBase58(
                request.pendingbailment().requestid());
        if (!originalRequest.empty()) {
            issuer.SetUsed(
                contract::peer::PeerRequestType::Bailment,
                originalRequest,
                true);
        } else {
            LogError()(OT_PRETTY_CLASS())(
                ": Failed to set request as used on issuer.")
                .Flush();
        }

        auto [taskID, future] = client_.OTX().AcknowledgeNotice(
            nymID, serverID, issuerNymID, requestID, true);

        if (0 == taskID) {
            LogDetail()(OT_PRETTY_CLASS())(
                ": Acknowledgement request already queued.")
                .Flush();

            return false;
        }

        const auto result = future.get();
        const auto status = std::get<0>(result);

        if (otx::LastReplyStatus::MessageSuccess == status) {
            const auto message = std::get<1>(result);
            auto replyID = identifier::Generic{};
            message->GetIdentifier(replyID);
            issuer.AddReply(
                contract::peer::PeerRequestType::PendingBailment,
                requestID,
                replyID);

            return true;
        }
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to add request.").Flush();
    }

    return false;
}

auto Pair::process_request_bailment(
    const Lock& lock,
    const identifier::Nym& nymID,
    const proto::PeerReply& reply) const -> bool
{
    OT_ASSERT(CheckLock(lock, decision_lock_));
    OT_ASSERT(
        nymID == client_.Factory().IdentifierFromBase58(reply.initiator()));
    OT_ASSERT(
        contract::peer::PeerRequestType::Bailment == translate(reply.type()));

    const auto requestID =
        client_.Factory().IdentifierFromBase58(reply.cookie());
    const auto replyID = client_.Factory().IdentifierFromBase58(reply.id());
    const auto issuerNymID =
        client_.Factory().NymIDFromBase58(reply.recipient());
    auto editor =
        client_.Wallet().Internal().mutable_Issuer(nymID, issuerNymID);
    auto& issuer = editor.get();
    const auto added = issuer.AddReply(
        contract::peer::PeerRequestType::Bailment, requestID, replyID);

    if (added) {
        client_.Wallet().PeerRequestComplete(nymID, replyID);

        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to add reply.").Flush();

        return false;
    }
}

auto Pair::process_request_outbailment(
    const Lock& lock,
    const identifier::Nym& nymID,
    const proto::PeerReply& reply) const -> bool
{
    OT_ASSERT(CheckLock(lock, decision_lock_));
    OT_ASSERT(
        nymID == client_.Factory().IdentifierFromBase58(reply.initiator()));
    OT_ASSERT(
        contract::peer::PeerRequestType::OutBailment ==
        translate(reply.type()));

    const auto requestID =
        client_.Factory().IdentifierFromBase58(reply.cookie());
    const auto replyID = client_.Factory().IdentifierFromBase58(reply.id());
    const auto issuerNymID =
        client_.Factory().NymIDFromBase58(reply.recipient());
    auto editor =
        client_.Wallet().Internal().mutable_Issuer(nymID, issuerNymID);
    auto& issuer = editor.get();
    const auto added = issuer.AddReply(
        contract::peer::PeerRequestType::OutBailment, requestID, replyID);

    if (added) {
        client_.Wallet().PeerRequestComplete(nymID, replyID);

        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to add reply.").Flush();

        return false;
    }
}

auto Pair::process_store_secret(
    const Lock& lock,
    const identifier::Nym& nymID,
    const proto::PeerReply& reply) const -> bool
{
    OT_ASSERT(CheckLock(lock, decision_lock_));
    OT_ASSERT(
        nymID == client_.Factory().IdentifierFromBase58(reply.initiator()));
    OT_ASSERT(
        contract::peer::PeerRequestType::StoreSecret ==
        translate(reply.type()));

    const auto requestID =
        client_.Factory().IdentifierFromBase58(reply.cookie());
    const auto replyID = client_.Factory().IdentifierFromBase58(reply.id());
    const auto issuerNymID =
        client_.Factory().NymIDFromBase58(reply.recipient());
    auto editor =
        client_.Wallet().Internal().mutable_Issuer(nymID, issuerNymID);
    auto& issuer = editor.get();
    const auto added = issuer.AddReply(
        contract::peer::PeerRequestType::StoreSecret, requestID, replyID);

    if (added) {
        client_.Wallet().PeerRequestComplete(nymID, replyID);
        const auto published = pair_event_->Send([&] {
            auto out = opentxs::network::zeromq::Message{};
            out.Internal().AddFrame([&] {
                auto event = proto::PairEvent{};
                event.set_version(1);
                event.set_type(proto::PAIREVENT_STORESECRET);
                event.set_issuer(issuerNymID.asBase58(client_.Crypto()));

                return event;
            }());

            return out;
        }());

        if (published) {
            LogDetail()(OT_PRETTY_CLASS())(
                ": Published store secret notification.")
                .Flush();
        } else {
            LogError()(OT_PRETTY_CLASS())(
                ": Error Publishing store secret notification.")
                .Flush();
        }

        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to add reply.").Flush();

        return false;
    }
}

auto Pair::queue_nym_download(
    const identifier::Nym& localNymID,
    const identifier::Nym& targetNymID) const
    -> api::session::OTX::BackgroundTask
{
    client_.OTX().StartIntroductionServer(localNymID);

    return client_.OTX().FindNym(targetNymID);
}

auto Pair::queue_nym_registration(
    const identifier::Nym& nymID,
    const identifier::Notary& serverID,
    const bool setData) const -> api::session::OTX::BackgroundTask
{
    return client_.OTX().RegisterNym(nymID, serverID, setData);
}

auto Pair::queue_server_contract(
    const identifier::Nym& nymID,
    const identifier::Notary& serverID) const
    -> api::session::OTX::BackgroundTask
{
    client_.OTX().StartIntroductionServer(nymID);

    return client_.OTX().FindServer(serverID);
}

void Pair::queue_unit_definition(
    const identifier::Nym& nymID,
    const identifier::Notary& serverID,
    const identifier::UnitDefinition& unitID) const
{
    const auto [taskID, future] =
        client_.OTX().DownloadUnitDefinition(nymID, serverID, unitID);

    if (0 == taskID) {
        LogError()(OT_PRETTY_CLASS())(
            ": Failed to queue unit definition download")
            .Flush();

        return;
    }

    const auto [result, pReply] = future.get();
    const auto success = (otx::LastReplyStatus::MessageSuccess == result);

    if (success) {
        LogDetail()(OT_PRETTY_CLASS())("Obtained unit definition ")(unitID)
            .Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())(": Failed to download unit definition ")(
            unitID)
            .Flush();
    }
}

auto Pair::register_account(
    const identifier::Nym& nymID,
    const identifier::Notary& serverID,
    const identifier::UnitDefinition& unitID) const
    -> std::pair<bool, identifier::Generic>
{
    auto output = std::pair<bool, identifier::Generic>{};
    auto& [success, accountID] = output;

    try {
        client_.Wallet().Internal().UnitDefinition(unitID);
    } catch (...) {
        LogTrace()(OT_PRETTY_CLASS())("Waiting for unit definition ")(unitID)
            .Flush();
        queue_unit_definition(nymID, serverID, unitID);

        return output;
    }

    auto [taskID, future] =
        client_.OTX().RegisterAccount(nymID, serverID, unitID);

    if (0 == taskID) { return output; }

    const auto [result, pReply] = future.get();
    success = (otx::LastReplyStatus::MessageSuccess == result);

    if (success) {
        OT_ASSERT(pReply);

        const auto& reply = *pReply;
        accountID =
            client_.Factory().IdentifierFromBase58(reply.acct_id_->Bytes());
    }

    return output;
}

void Pair::state_machine(const IssuerID& id) const
{
    const auto& [localNymID, issuerNymID] = id;
    LogDetail()(OT_PRETTY_CLASS())("Local nym: ")(localNymID)(" Issuer Nym: ")(
        issuerNymID)
        .Flush();
    auto reason = client_.Factory().PasswordPrompt("Pairing state machine");
    auto it = state_.GetDetails(localNymID, issuerNymID);

    OT_ASSERT(state_.end() != it);

    auto& [mutex, serverID, serverNymID, status, trusted, offered, registeredAccounts, accountDetails, pending, needRename] =
        it->second;

    OT_ASSERT(mutex);

    for (auto i = pending.begin(); i != pending.end();) {
        const auto& [task, future] = *i;
        const auto state = future.wait_for(10ms);

        if (std::future_status::ready == state) {
            const auto result = future.get();

            if (otx::LastReplyStatus::MessageSuccess == result.first) {
                LogTrace()(OT_PRETTY_CLASS())("Task ")(
                    task)(" completed successfully.")
                    .Flush();
            } else {
                LogError()(OT_PRETTY_CLASS())("Task ")(task)(" failed.")
                    .Flush();
            }

            i = pending.erase(i);
        } else {
            ++i;
        }
    }

    if (0 < pending.size()) { return; }

    Lock lock(*mutex);
    const auto issuerNym = client_.Wallet().Nym(issuerNymID);

    if (false == bool(issuerNym)) {
        LogVerbose()(OT_PRETTY_CLASS())("Issuer nym not yet downloaded.")
            .Flush();
        pending.emplace_back(queue_nym_download(localNymID, issuerNymID));
        status = Status::Error;

        return;
    }

    PAIR_SHUTDOWN();

    const auto& issuerClaims = issuerNym->Claims();
    serverID = issuerClaims.PreferredOTServer();

    if (serverID.empty()) {
        LogError()(OT_PRETTY_CLASS())(
            ": Issuer nym does not advertise a server.")
            .Flush();
        // Maybe there's a new version
        pending.emplace_back(queue_nym_download(localNymID, issuerNymID));
        status = Status::Error;

        return;
    }

    PAIR_SHUTDOWN();

    auto editor =
        client_.Wallet().Internal().mutable_Issuer(localNymID, issuerNymID);
    auto& issuer = editor.get();
    trusted = issuer.Paired();

    PAIR_SHUTDOWN();

    switch (status) {
        case Status::Error: {
            LogDetail()(OT_PRETTY_CLASS())(
                ": First pass through state machine.")
                .Flush();
            status = Status::Started;

            [[fallthrough]];
        }
        case Status::Started: {
            if (need_registration(localNymID, serverID)) {
                LogError()(OT_PRETTY_CLASS())(
                    ": Local nym not registered on issuer's notary.")
                    .Flush();

                try {
                    const auto contract =
                        client_.Wallet().Internal().Server(serverID);

                    PAIR_SHUTDOWN();

                    pending.emplace_back(
                        queue_nym_registration(localNymID, serverID, trusted));
                } catch (...) {
                    LogError()(OT_PRETTY_CLASS())(
                        ": Waiting on server contract.")
                        .Flush();
                    pending.emplace_back(
                        queue_server_contract(localNymID, serverID));

                    return;
                }

                return;
            } else {
                status = Status::Registered;
            }

            [[fallthrough]];
        }
        case Status::Registered: {
            PAIR_SHUTDOWN();

            LogDetail()(OT_PRETTY_CLASS())(
                ": Local nym is registered on issuer's notary.")
                .Flush();

            if (serverNymID.empty()) {
                try {
                    serverNymID = client_.Wallet()
                                      .Internal()
                                      .Server(serverID)
                                      ->Nym()
                                      ->ID();
                } catch (...) {

                    return;
                }
            }

            PAIR_SHUTDOWN();

            check_rename(issuer, serverID, reason, needRename);

            PAIR_SHUTDOWN();

            check_store_secret(issuer, serverID);

            PAIR_SHUTDOWN();

            check_connection_info(issuer, serverID);

            PAIR_SHUTDOWN();

            check_accounts(
                issuerClaims,
                issuer,
                serverID,
                offered,
                registeredAccounts,
                accountDetails);
            [[fallthrough]];
        }
        default: {
        }
    }
}

auto Pair::store_secret(
    const identifier::Nym& localNymID,
    const identifier::Nym& issuerNymID,
    const identifier::Notary& serverID) const
    -> std::pair<bool, identifier::Generic>
{
    auto output = std::pair<bool, identifier::Generic>{};

    if (false == api::crypto::HaveHDKeys()) { return output; }

    auto reason = client_.Factory().PasswordPrompt(
        "Backing up BIP-39 data to paired node");
    auto& [success, requestID] = output;

    auto setID = [&](const identifier::Generic& in) -> void {
        output.second = in;
    };
    const auto seedID = client_.Crypto().Seed().DefaultSeed().first;
    auto [taskID, future] = client_.OTX().InitiateStoreSecret(
        localNymID,
        serverID,
        issuerNymID,
        contract::peer::SecretType::Bip39,
        client_.Crypto().Seed().Words(seedID, reason),
        client_.Crypto().Seed().Passphrase(seedID, reason),
        setID);

    if (0 == taskID) { return output; }

    const auto result = std::get<0>(future.get());
    success = (otx::LastReplyStatus::MessageSuccess == result);

    return output;
}
}  // namespace opentxs::otx::client::implementation

#undef PAIR_SHUTDOWN
#undef MINIMUM_UNUSED_BAILMENTS

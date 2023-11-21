// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/client/Pair.hpp"  // IWYU pragma: associated

#include <PairEvent.pb.h>
#include <ZMQEnums.pb.h>
#include <algorithm>
#include <chrono>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <span>
#include <string_view>

#include "core/StateMachine.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/core/Core.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
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
#include "internal/util/Editor.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/Lockable.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/contract/peer/ConnectionInfoType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Reply.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/SecretType.hpp"   // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/contract/peer/reply/Bailment.hpp"
#include "opentxs/core/contract/peer/reply/Connection.hpp"
#include "opentxs/core/contract/peer/reply/Outbailment.hpp"
#include "opentxs/core/contract/peer/reply/StoreSecret.hpp"
#include "opentxs/core/contract/peer/request/BailmentNotice.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/HDSeed.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/claim/Data.hpp"
#include "opentxs/identity/wot/claim/Group.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/identity/wot/claim/Section.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"  // IWYU pragma: keep
#include "opentxs/otx/Types.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Writer.hpp"

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
    , api_(client)
    , state_(client.Crypto(), decision_lock_)
    , startup_promise_()
    , startup_(startup_promise_.get_future())
    , nym_callback_(zmq::ListenCallback::Factory(
          [this](const auto& in) -> void { callback_nym(in); }))
    , peer_reply_callback_(zmq::ListenCallback::Factory(
          [this](const auto& in) -> void { callback_peer_reply(in); }))
    , peer_request_callback_(zmq::ListenCallback::Factory(
          [this](const auto& in) -> void { callback_peer_request(in); }))
    , pair_event_(api_.Network().ZeroMQ().Internal().PublishSocket())
    , pending_bailment_(api_.Network().ZeroMQ().Internal().PublishSocket())
    , nym_subscriber_(api_.Network().ZeroMQ().Internal().SubscribeSocket(
          nym_callback_,
          "Pair nym"))
    , peer_reply_subscriber_(api_.Network().ZeroMQ().Internal().SubscribeSocket(
          peer_reply_callback_,
          "Pair reply"))
    , peer_request_subscriber_(
          api_.Network().ZeroMQ().Internal().SubscribeSocket(
              peer_request_callback_,
              "Pair request"))
{
    // WARNING: do not access api_.Wallet() during construction
    pair_event_->Start(api_.Endpoints().Internal().PairEvent().data());
    pending_bailment_->Start(
        api_.Endpoints().Internal().PendingBailment().data());
    nym_subscriber_->Start(api_.Endpoints().NymDownload().data());
    peer_reply_subscriber_->Start(
        api_.Endpoints().Internal().PeerReplyUpdate().data());
    peer_request_subscriber_->Start(
        api_.Endpoints().Internal().PeerRequestUpdate().data());
}

Pair::State::State(const api::Crypto& crypto, std::mutex& lock) noexcept
    : crypto_(crypto)
    , lock_(lock)
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
    assert_true(CheckLock(lock, lock_));

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

auto Pair::State::Add(
    const identifier::Nym& localNymID,
    const identifier::Nym& issuerNymID,
    const bool trusted) noexcept -> void
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

        assert_false(nullptr == mutex);

        Lock rowLock(*mutex);

        if (Status::Registered != status) {
            LogTrace()()("Not registered").Flush();

            goto repeat;
        }

        if (needRename) {
            LogTrace()()("Notary name not set").Flush();

            goto repeat;
        }

        const auto accountCount = count_currencies(accountDetails);

        if (accountCount != offered) {
            LogTrace()()(": Waiting for account registration, expected: ")(
                offered)(", have ")(accountCount)
                .Flush();

            goto repeat;
        }

        for (const auto& [unit, account, bailments] : accountDetails) {
            if (bailments < MINIMUM_UNUSED_BAILMENTS) {
                LogTrace()()(": Waiting for bailment instructions for "
                             "account ")(account, crypto_)(", expected: ")(
                    MINIMUM_UNUSED_BAILMENTS)(", have ")(bailments)
                    .Flush();

                goto repeat;
            }
        }
    }

    // No reason to continue executing state machine
    LogTrace()()("Done").Flush();

    return false;

repeat:
    lock.unlock();
    LogTrace()()("Repeating").Flush();
    // Rate limit state machine to reduce unproductive execution while waiting
    // on network activity
    Sleep(50ms);

    return true;
}

auto Pair::State::count_currencies(
    const UnallocatedVector<AccountDetails>& in) noexcept -> std::size_t
{
    auto unique = UnallocatedSet<identifier::UnitDefinition>{};
    std::ranges::transform(
        in,
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
        assert_false(nullptr == pGroup);

        const auto& group = *pGroup;

        for (const auto& [id, pClaim] : group) {
            assert_false(nullptr == pClaim);

            const auto& claim = *pClaim;
            unique.emplace(api.Factory().UnitIDFromBase58(claim.Value()));
        }
    }

    return unique.size();
}

auto Pair::State::get_account(
    const api::session::Client& client,
    const identifier::UnitDefinition& unit,
    const identifier::Account& account,
    UnallocatedVector<AccountDetails>& details) noexcept -> AccountDetails&
{
    assert_false(unit.empty());
    assert_false(account.empty());

    for (auto& row : details) {
        const auto& [unitID, accountID, bailment] = row;
        const auto match = (unit.asBase58(client.Crypto()) ==
                            unitID.asBase58(client.Crypto())) &&
                           (account.asBase58(client.Crypto()) ==
                            accountID.asBase58(client.Crypto()));

        if (match) { return row; }
    }

    return details.emplace_back(unit, account, 0);
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

        assert_false(nullptr == pMutex);

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
        std::ranges::transform(
            state_,
            std::inserter(list, list.end()),
            [](const auto& in) -> IssuerID { return in.first; });
    }

    std::ranges::for_each(list, fn);

    return check_state();
}

auto Pair::AddIssuer(
    const identifier::Nym& localNymID,
    const identifier::Nym& issuerNymID,
    const UnallocatedCString& pairingCode) const noexcept -> bool
{
    if (localNymID.empty()) {
        LogError()()("Invalid local nym id.").Flush();

        return false;
    }

    if (!api_.Wallet().IsLocalNym(localNymID.asBase58(api_.Crypto()))) {
        LogError()()("Invalid local nym.").Flush();

        return false;
    }

    if (issuerNymID.empty()) {
        LogError()()("Invalid issuer nym id.").Flush();

        return false;
    }

    if (blockchain::Type::UnknownBlockchain !=
        blockchain::Chain(api_, issuerNymID)) {
        LogError()()(": blockchains can not be used as otx issuers.").Flush();

        return false;
    }

    bool trusted{false};

    {
        auto editor =
            api_.Wallet().Internal().mutable_Issuer(localNymID, issuerNymID);
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

auto Pair::callback_nym(const zmq::Message& in) noexcept -> void
{
    startup_.get();
    const auto body = in.Payload();

    assert_true(1 < body.size());

    const auto nymID = api_.Factory().NymIDFromHash(body[1].Bytes());
    auto trigger{state_.CheckIssuer(nymID)};

    {
        Lock lock(decision_lock_);

        for (auto& [id, details] : state_) {
            auto& [mutex, serverID, serverNymID, status, trusted, offered, registered, accountDetails, pending, needRename] =
                details;

            assert_false(nullptr == mutex);

            Lock rowLock(*mutex);

            if (serverNymID == nymID) { trigger = true; }
        }
    }

    if (trigger) { Trigger(); }
}

auto Pair::callback_peer_reply(const zmq::Message& in) noexcept -> void
{
    startup_.get();
    const auto body = in.Payload();

    assert_true(2 <= body.size());

    auto trigger{false};
    const auto nymID = api_.Factory().NymIDFromBase58(body[0].Bytes());
    const auto reply = api_.Factory().PeerReply(body[1]);

    if (false == reply.IsValid()) { return; }

    switch (reply.Type()) {
        case contract::peer::RequestType::Bailment: {
            LogDetail()()("Received bailment reply.").Flush();
            Lock lock(decision_lock_);
            trigger = process_request_bailment(lock, nymID, reply.asBailment());
        } break;
        case contract::peer::RequestType::OutBailment: {
            LogDetail()()("Received outbailment reply.").Flush();
            Lock lock(decision_lock_);
            trigger =
                process_request_outbailment(lock, nymID, reply.asOutbailment());
        } break;
        case contract::peer::RequestType::ConnectionInfo: {
            LogDetail()()(": Received connection info reply.").Flush();
            Lock lock(decision_lock_);
            trigger =
                process_connection_info(lock, nymID, reply.asConnection());
        } break;
        case contract::peer::RequestType::StoreSecret: {
            LogDetail()()("Received store secret reply.").Flush();
            Lock lock(decision_lock_);
            trigger = process_store_secret(lock, nymID, reply.asStoreSecret());
        } break;
        case contract::peer::RequestType::Error:
        case contract::peer::RequestType::PendingBailment:
        case contract::peer::RequestType::VerifiedClaim:
        case contract::peer::RequestType::Faucet:
        default: {
        }
    }

    if (trigger) { Trigger(); }
}

auto Pair::callback_peer_request(const zmq::Message& in) noexcept -> void
{
    startup_.get();
    const auto body = in.Payload();

    assert_true(2 <= body.size());

    auto trigger{false};
    const auto nymID = api_.Factory().NymIDFromBase58(body[0].Bytes());
    const auto request = api_.Factory().PeerRequest(body[1]);

    if (false == request.IsValid()) { return; }

    switch (request.Type()) {
        case contract::peer::RequestType::PendingBailment: {
            Lock lock(decision_lock_);
            trigger = process_pending_bailment(
                lock, nymID, request.asBailmentNotice());
        } break;
        default: {
        }
    }

    if (trigger) { Trigger(); }
}

auto Pair::check_accounts(
    const identity::wot::claim::Data& issuerClaims,
    otx::client::Issuer& issuer,
    const identifier::Notary& serverID,
    std::size_t& offered,
    std::size_t& registeredAccounts,
    UnallocatedVector<Pair::State::AccountDetails>& accountDetails)
    const noexcept -> void
{
    const auto& localNymID = issuer.LocalNymID();
    const auto& issuerNymID = issuer.IssuerID();
    const auto contractSection =
        issuerClaims.Section(identity::wot::claim::SectionType::Contract);
    const auto haveAccounts = bool(contractSection);

    if (false == haveAccounts) {
        LogError()()(": Issuer does not advertise any contracts.").Flush();
    } else {
        offered = State::count_currencies(api_, *contractSection);
        LogDetail()()("Issuer advertises ")(offered)(" contract")(
            (1 == offered) ? "." : "s.")
            .Flush();
    }

    auto uniqueRegistered = UnallocatedSet<identifier::UnitDefinition>{};

    if (false == haveAccounts) { return; }

    for (const auto& [type, pGroup] : *contractSection) {
        PAIR_SHUTDOWN();
        assert_false(nullptr == pGroup);

        const auto& group = *pGroup;

        for (const auto& [id, pClaim] : group) {
            PAIR_SHUTDOWN();
            assert_false(nullptr == pClaim);

            const auto& notUsed [[maybe_unused]] = id;
            const auto& claim = *pClaim;
            const auto unitID = api_.Factory().UnitIDFromBase58(claim.Value());

            if (unitID.empty()) {
                LogDetail()()("Invalid unit definition").Flush();

                continue;
            }

            const auto accountList =
                issuer.AccountList(ClaimToUnit(type), unitID);

            if (0 == accountList.size()) {
                LogDetail()()("Registering ")(unitID, api_.Crypto())(
                    " account for ")(localNymID, api_.Crypto())(" on ")(
                    serverID, api_.Crypto())(".")
                    .Flush();
                const auto& [registered, accountid] =
                    register_account(localNymID, serverID, unitID);

                if (registered) {
                    LogDetail()()(": Success registering account").Flush();
                    issuer.AddAccount(ClaimToUnit(type), unitID, accountid);
                } else {
                    LogError()()(": Failed to register account").Flush();
                }

                continue;
            } else {
                LogDetail()()(unitID, api_.Crypto())(" account for ")(
                    localNymID, api_.Crypto())(" on ")(serverID, api_.Crypto())(
                    " already exists.")
                    .Flush();
            }

            for (const auto& accountID : accountList) {
                auto& details =
                    State::get_account(api_, unitID, accountID, accountDetails);
                uniqueRegistered.emplace(unitID);
                auto& bailmentCount = std::get<2>(details);
                const auto instructions =
                    issuer.BailmentInstructions(api_, unitID);
                bailmentCount = instructions.size();
                const bool needBailment =
                    (MINIMUM_UNUSED_BAILMENTS > instructions.size());
                const bool nonePending =
                    (false == issuer.BailmentInitiated(unitID));

                if (needBailment && nonePending) {
                    LogDetail()()(": Requesting bailment info for ")(
                        unitID, api_.Crypto())(".")
                        .Flush();
                    const auto& [sent, requestID] = initiate_bailment(
                        localNymID, serverID, issuerNymID, unitID);

                    if (sent) {
                        issuer.AddRequest(
                            contract::peer::RequestType::Bailment, requestID);
                    }
                }
            }
        }
    }

    registeredAccounts = uniqueRegistered.size();
}

auto Pair::check_connection_info(otx::client::Issuer& issuer) const noexcept
    -> void
{
    const auto trusted = issuer.Paired();

    if (false == trusted) { return; }

    const auto btcrpc =
        issuer.ConnectionInfo(api_, contract::peer::ConnectionInfoType::BtcRpc);
    const bool needInfo =
        (btcrpc.empty() &&
         (false == issuer.ConnectionInfoInitiated(
                       contract::peer::ConnectionInfoType::BtcRpc)));

    if (needInfo) {
        LogDetail()()(": Sending connection info peer request.").Flush();
        const auto [sent, requestID] = get_connection(
            issuer.LocalNymID(),
            issuer.IssuerID(),
            contract::peer::ConnectionInfoType::BtcRpc);

        if (sent) {
            issuer.AddRequest(
                contract::peer::RequestType::ConnectionInfo, requestID);
        }
    }
}

auto Pair::check_rename(
    const otx::client::Issuer& issuer,
    const identifier::Notary& serverID,
    const PasswordPrompt& reason,
    bool& needRename) const noexcept -> void
{
    if (false == issuer.Paired()) {
        LogTrace()()("Not trusted").Flush();

        return;
    }

    auto editor = api_.Wallet().Internal().mutable_ServerContext(
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
                proto.set_issuer(issuer.IssuerID().asBase58(api_.Crypto()));

                return proto;
            }());

            return out;
        }());

        if (published) {
            LogDetail()()(": Published should rename notification.").Flush();
        } else {
            LogError()()(": Error publishing should rename notification.")
                .Flush();
        }
    } else {
        LogTrace()()("No reason to rename").Flush();
    }
}

auto Pair::check_store_secret(otx::client::Issuer& issuer) const noexcept
    -> void
{
    if (false == issuer.Paired()) { return; }

    const auto needStoreSecret = (false == issuer.StoreSecretComplete()) &&
                                 (false == issuer.StoreSecretInitiated()) &&
                                 api::crypto::HaveHDKeys();

    if (needStoreSecret) {
        LogDetail()()("Sending store secret peer request.").Flush();
        const auto [sent, requestID] =
            store_secret(issuer.LocalNymID(), issuer.IssuerID());

        if (sent) {
            issuer.AddRequest(
                contract::peer::RequestType::StoreSecret, requestID);
        }
    }
}

auto Pair::CheckIssuer(
    const identifier::Nym& localNymID,
    const identifier::UnitDefinition& unitDefinitionID) const noexcept -> bool
{
    try {
        const auto contract =
            api_.Wallet().Internal().UnitDefinition(unitDefinitionID);

        return AddIssuer(localNymID, contract->Signer()->ID(), "");
    } catch (...) {
        LogError()()(": Unit definition contract does not exist.").Flush();

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
    const contract::peer::ConnectionInfoType type) const
    -> std::pair<bool, identifier::Generic>
{
    auto output = std::pair<bool, identifier::Generic>{};
    auto& [success, requestID] = output;

    auto setID = [&](const identifier::Generic& in) -> void {
        output.second = in;
    };
    auto [taskID, future] = api_.OTX().InitiateRequestConnection(
        localNymID, issuerNymID, type, setID);

    if (0 == taskID) { return output; }

    const auto result = std::get<0>(future.get());
    success = (otx::LastReplyStatus::MessageSuccess == result);

    return output;
}

auto Pair::init() noexcept -> void
{
    Lock lock(decision_lock_);

    for (const auto& nymID : api_.Wallet().LocalNyms()) {
        for (const auto& issuerID : api_.Wallet().IssuerList(nymID)) {
            const auto pIssuer =
                api_.Wallet().Internal().Issuer(nymID, issuerID);

            assert_false(nullptr == pIssuer);

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
        api_.Wallet().Internal().UnitDefinition(unitID);
    } catch (...) {
        queue_unit_definition(nymID, serverID, unitID);

        return output;
    }

    auto setID = [&](const identifier::Generic& in) -> void {
        output.second = in;
    };
    auto [taskID, future] =
        api_.OTX().InitiateBailment(nymID, serverID, issuerID, unitID, setID);

    if (0 == taskID) { return output; }

    const auto result = std::get<0>(future.get());
    success = (otx::LastReplyStatus::MessageSuccess == result);

    return output;
}

auto Pair::IssuerDetails(
    const identifier::Nym& localNymID,
    const identifier::Nym& issuerNymID) const noexcept -> UnallocatedCString
{
    auto issuer = api_.Wallet().Internal().Issuer(localNymID, issuerNymID);

    if (false == bool(issuer)) { return {}; }

    return issuer->toString();
}

auto Pair::need_registration(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID) const -> bool
{
    auto context = api_.Wallet().Internal().ServerContext(localNymID, serverID);

    if (context) { return (0 == context->Request()); }

    return true;
}

auto Pair::process_connection_info(
    const Lock& lock,
    const identifier::Nym& nymID,
    const contract::peer::reply::Connection& reply) const -> bool
{
    assert_true(CheckLock(lock, decision_lock_));

    const auto& requestID = reply.InReferenceToRequest();
    const auto& replyID = reply.ID();
    const auto& issuerNymID = reply.Responder();
    auto editor = api_.Wallet().Internal().mutable_Issuer(nymID, issuerNymID);
    auto& issuer = editor.get();
    const auto added = issuer.AddReply(
        contract::peer::RequestType::ConnectionInfo, requestID, replyID);

    if (added) {
        api_.Wallet().PeerRequestComplete(nymID, replyID);

        return true;
    } else {
        LogError()()("Failed to add reply.").Flush();

        return false;
    }
}

auto Pair::process_peer_replies(const Lock& lock, const identifier::Nym& nymID)
    const -> void
{
    assert_true(CheckLock(lock, decision_lock_));

    auto replies = api_.Wallet().PeerReplyIncoming(nymID);

    for (const auto& it : replies) {
        const auto replyID = api_.Factory().IdentifierFromBase58(it.first);
        auto reply = api_.Wallet().PeerReply(
            nymID, replyID, otx::client::StorageBox::INCOMINGPEERREPLY);

        if (false == reply.IsValid()) {
            LogError()()("Failed to load peer reply ")(it.first)(".").Flush();

            continue;
        }

        switch (reply.Type()) {
            case contract::peer::RequestType::Bailment: {
                LogDetail()()("Received bailment reply.").Flush();
                process_request_bailment(lock, nymID, reply.asBailment());
            } break;
            case contract::peer::RequestType::OutBailment: {
                LogDetail()()(": Received outbailment reply.").Flush();
                process_request_outbailment(lock, nymID, reply.asOutbailment());
            } break;
            case contract::peer::RequestType::ConnectionInfo: {
                LogDetail()()(": Received connection info reply.").Flush();
                process_connection_info(lock, nymID, reply.asConnection());
            } break;
            case contract::peer::RequestType::StoreSecret: {
                LogDetail()()(": Received store secret reply.").Flush();
                process_store_secret(lock, nymID, reply.asStoreSecret());
            } break;
            case contract::peer::RequestType::Error:
            case contract::peer::RequestType::PendingBailment:
            case contract::peer::RequestType::VerifiedClaim:
            case contract::peer::RequestType::Faucet:
            default: {
                continue;
            }
        }
    }
}

auto Pair::process_peer_requests(const Lock& lock, const identifier::Nym& nymID)
    const -> void
{
    assert_true(CheckLock(lock, decision_lock_));

    const auto requests = api_.Wallet().PeerRequestIncoming(nymID);

    for (const auto& it : requests) {
        const auto requestID = api_.Factory().IdentifierFromBase58(it.first);
        auto request = api_.Wallet().PeerRequest(
            nymID, requestID, otx::client::StorageBox::INCOMINGPEERREQUEST);

        if (false == request.IsValid()) {
            LogError()()("Failed to load peer request ")(it.first)(".").Flush();

            continue;
        }

        switch (request.Type()) {
            case contract::peer::RequestType::PendingBailment: {
                LogError()()(": Received pending bailment notification.")
                    .Flush();
                process_pending_bailment(
                    lock, nymID, request.asBailmentNotice());
            } break;
            case contract::peer::RequestType::Error:
            case contract::peer::RequestType::Bailment:
            case contract::peer::RequestType::OutBailment:
            case contract::peer::RequestType::ConnectionInfo:
            case contract::peer::RequestType::StoreSecret:
            case contract::peer::RequestType::VerifiedClaim:
            case contract::peer::RequestType::Faucet:
            default: {

                continue;
            }
        }
    }
}

auto Pair::process_pending_bailment(
    const Lock& lock,
    const identifier::Nym& nymID,
    const contract::peer::request::BailmentNotice& request) const -> bool
{
    assert_true(CheckLock(lock, decision_lock_));

    const auto requestID = request.ID();
    const auto issuerNymID = request.Initiator();
    const auto serverID = request.Notary();
    auto editor = api_.Wallet().Internal().mutable_Issuer(nymID, issuerNymID);
    auto& issuer = editor.get();
    const auto added = issuer.AddRequest(
        contract::peer::RequestType::PendingBailment, requestID);

    if (added) {
        pending_bailment_->Send([&] {
            auto out = opentxs::network::zeromq::Message{};
            [[maybe_unused]] auto _ = request.Serialize(out.AppendBytes());

            return out;
        }());
        const auto& originalRequest = request.InReferenceToRequest();

        if (false == originalRequest.empty()) {
            issuer.SetUsed(
                contract::peer::RequestType::Bailment, originalRequest, true);
        } else {
            LogError()()("Failed to set request as used on issuer.").Flush();
        }

        auto [taskID, future] = api_.OTX().AcknowledgeStoreSecret(
            nymID, issuerNymID, requestID, true);

        if (0 == taskID) {
            LogDetail()()("StoreSecret request already queued.").Flush();

            return false;
        }

        const auto result = future.get();
        const auto status = std::get<0>(result);

        if (otx::LastReplyStatus::MessageSuccess == status) {
            const auto message = std::get<1>(result);
            auto replyID = identifier::Generic{};
            message->GetIdentifier(replyID);
            issuer.AddReply(
                contract::peer::RequestType::PendingBailment,
                requestID,
                replyID);

            return true;
        }
    } else {
        LogError()()("Failed to add request.").Flush();
    }

    return false;
}

auto Pair::process_request_bailment(
    const Lock& lock,
    const identifier::Nym& nymID,
    const contract::peer::reply::Bailment& reply) const -> bool
{
    assert_true(CheckLock(lock, decision_lock_));

    const auto requestID = reply.InReferenceToRequest();
    const auto replyID = reply.ID();
    const auto issuerNymID = reply.Responder();
    auto editor = api_.Wallet().Internal().mutable_Issuer(nymID, issuerNymID);
    auto& issuer = editor.get();
    const auto added = issuer.AddReply(
        contract::peer::RequestType::Bailment, requestID, replyID);

    if (added) {
        api_.Wallet().PeerRequestComplete(nymID, replyID);

        return true;
    } else {
        LogError()()("Failed to add reply.").Flush();

        return false;
    }
}

auto Pair::process_request_outbailment(
    const Lock& lock,
    const identifier::Nym& nymID,
    const contract::peer::reply::Outbailment& reply) const -> bool
{
    assert_true(CheckLock(lock, decision_lock_));

    const auto requestID = reply.InReferenceToRequest();
    const auto replyID = reply.ID();
    const auto issuerNymID = reply.Responder();
    auto editor = api_.Wallet().Internal().mutable_Issuer(nymID, issuerNymID);
    auto& issuer = editor.get();
    const auto added = issuer.AddReply(
        contract::peer::RequestType::OutBailment, requestID, replyID);

    if (added) {
        api_.Wallet().PeerRequestComplete(nymID, replyID);

        return true;
    } else {
        LogError()()("Failed to add reply.").Flush();

        return false;
    }
}

auto Pair::process_store_secret(
    const Lock& lock,
    const identifier::Nym& nymID,
    const contract::peer::reply::StoreSecret& reply) const -> bool
{
    assert_true(CheckLock(lock, decision_lock_));

    const auto requestID = reply.InReferenceToRequest();
    const auto replyID = reply.ID();
    const auto issuerNymID = reply.Responder();
    auto editor = api_.Wallet().Internal().mutable_Issuer(nymID, issuerNymID);
    auto& issuer = editor.get();
    const auto added = issuer.AddReply(
        contract::peer::RequestType::StoreSecret, requestID, replyID);

    if (added) {
        api_.Wallet().PeerRequestComplete(nymID, replyID);
        const auto published = pair_event_->Send([&] {
            auto out = opentxs::network::zeromq::Message{};
            out.Internal().AddFrame([&] {
                auto event = proto::PairEvent{};
                event.set_version(1);
                event.set_type(proto::PAIREVENT_STORESECRET);
                event.set_issuer(issuerNymID.asBase58(api_.Crypto()));

                return event;
            }());

            return out;
        }());

        if (published) {
            LogDetail()()(": Published store secret notification.").Flush();
        } else {
            LogError()()(": Error Publishing store secret notification.")
                .Flush();
        }

        return true;
    } else {
        LogError()()("Failed to add reply.").Flush();

        return false;
    }
}

auto Pair::queue_nym_download(
    const identifier::Nym& localNymID,
    const identifier::Nym& targetNymID) const
    -> api::session::OTX::BackgroundTask
{
    api_.OTX().StartIntroductionServer(localNymID);

    return api_.OTX().FindNym(targetNymID);
}

auto Pair::queue_nym_registration(
    const identifier::Nym& nymID,
    const identifier::Notary& serverID,
    const bool setData) const -> api::session::OTX::BackgroundTask
{
    return api_.OTX().RegisterNym(nymID, serverID, setData);
}

auto Pair::queue_server_contract(
    const identifier::Nym& nymID,
    const identifier::Notary& serverID) const
    -> api::session::OTX::BackgroundTask
{
    api_.OTX().StartIntroductionServer(nymID);

    return api_.OTX().FindServer(serverID);
}

auto Pair::queue_unit_definition(
    const identifier::Nym& nymID,
    const identifier::Notary& serverID,
    const identifier::UnitDefinition& unitID) const -> void
{
    const auto [taskID, future] =
        api_.OTX().DownloadUnitDefinition(nymID, serverID, unitID);

    if (0 == taskID) {
        LogError()()(": Failed to queue unit definition download").Flush();

        return;
    }

    const auto [result, pReply] = future.get();
    const auto success = (otx::LastReplyStatus::MessageSuccess == result);

    if (success) {
        LogDetail()()("Obtained unit definition ")(unitID, api_.Crypto())
            .Flush();
    } else {
        LogError()()(": Failed to download unit definition ")(
            unitID, api_.Crypto())
            .Flush();
    }
}

auto Pair::register_account(
    const identifier::Nym& nymID,
    const identifier::Notary& serverID,
    const identifier::UnitDefinition& unitID) const
    -> std::pair<bool, identifier::Account>
{
    auto output = std::pair<bool, identifier::Account>{};
    auto& [success, accountID] = output;

    try {
        api_.Wallet().Internal().UnitDefinition(unitID);
    } catch (...) {
        LogTrace()()("Waiting for unit definition ")(unitID, api_.Crypto())
            .Flush();
        queue_unit_definition(nymID, serverID, unitID);

        return output;
    }

    auto [taskID, future] = api_.OTX().RegisterAccount(nymID, serverID, unitID);

    if (0 == taskID) { return output; }

    const auto [result, pReply] = future.get();
    success = (otx::LastReplyStatus::MessageSuccess == result);

    if (success) {
        assert_false(nullptr == pReply);

        const auto& reply = *pReply;
        accountID = api_.Factory().AccountIDFromBase58(reply.acct_id_->Bytes());
    }

    return output;
}

auto Pair::state_machine(const IssuerID& id) const -> void
{
    const auto& [localNymID, issuerNymID] = id;
    LogDetail()()("Local nym: ")(localNymID, api_.Crypto())(" Issuer Nym: ")(
        issuerNymID, api_.Crypto())
        .Flush();
    auto reason = api_.Factory().PasswordPrompt("Pairing state machine");
    auto it = state_.GetDetails(localNymID, issuerNymID);

    assert_true(state_.end() != it);

    auto& [mutex, serverID, serverNymID, status, trusted, offered, registeredAccounts, accountDetails, pending, needRename] =
        it->second;

    assert_false(nullptr == mutex);

    for (auto i = pending.begin(); i != pending.end();) {
        const auto& [task, future] = *i;
        const auto state = future.wait_for(10ms);

        if (std::future_status::ready == state) {
            const auto result = future.get();

            if (otx::LastReplyStatus::MessageSuccess == result.first) {
                LogTrace()()("Task ")(task)(" completed successfully.").Flush();
            } else {
                LogError()()("Task ")(task)(" failed.").Flush();
            }

            i = pending.erase(i);
        } else {
            ++i;
        }
    }

    if (0 < pending.size()) { return; }

    Lock lock(*mutex);
    const auto issuerNym = api_.Wallet().Nym(issuerNymID);

    if (false == bool(issuerNym)) {
        LogVerbose()()("Issuer nym not yet downloaded.").Flush();
        pending.emplace_back(queue_nym_download(localNymID, issuerNymID));
        status = Status::Error;

        return;
    }

    PAIR_SHUTDOWN();

    const auto& issuerClaims = issuerNym->Claims();
    serverID = issuerClaims.PreferredOTServer();

    if (serverID.empty()) {
        LogError()()(": Issuer nym does not advertise a server.").Flush();
        // Maybe there's a new version
        pending.emplace_back(queue_nym_download(localNymID, issuerNymID));
        status = Status::Error;

        return;
    }

    PAIR_SHUTDOWN();

    auto editor =
        api_.Wallet().Internal().mutable_Issuer(localNymID, issuerNymID);
    auto& issuer = editor.get();
    trusted = issuer.Paired();

    PAIR_SHUTDOWN();

    switch (status) {
        case Status::Error: {
            LogDetail()()(": First pass through state machine.").Flush();
            status = Status::Started;

            [[fallthrough]];
        }
        case Status::Started: {
            if (need_registration(localNymID, serverID)) {
                LogError()()(": Local nym not registered on issuer's notary.")
                    .Flush();

                try {
                    const auto contract =
                        api_.Wallet().Internal().Server(serverID);

                    PAIR_SHUTDOWN();

                    pending.emplace_back(
                        queue_nym_registration(localNymID, serverID, trusted));
                } catch (...) {
                    LogError()()(": Waiting on server contract.").Flush();
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

            LogDetail()()(": Local nym is registered on issuer's notary.")
                .Flush();

            if (serverNymID.empty()) {
                try {
                    serverNymID = api_.Wallet()
                                      .Internal()
                                      .Server(serverID)
                                      ->Signer()
                                      ->ID();
                } catch (...) {

                    return;
                }
            }

            PAIR_SHUTDOWN();

            check_rename(issuer, serverID, reason, needRename);

            PAIR_SHUTDOWN();

            check_store_secret(issuer);

            PAIR_SHUTDOWN();

            check_connection_info(issuer);

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
    const identifier::Nym& issuerNymID) const
    -> std::pair<bool, identifier::Generic>
{
    auto output = std::pair<bool, identifier::Generic>{};

    if (false == api::crypto::HaveHDKeys()) { return output; }

    auto reason =
        api_.Factory().PasswordPrompt("Backing up BIP-39 data to paired node");
    auto& [success, requestID] = output;

    auto setID = [&](const identifier::Generic& in) -> void {
        output.second = in;
    };
    const auto seedID = api_.Crypto().Seed().DefaultSeed().first;
    const auto phrase = api_.Crypto().Seed().Words(seedID, reason);
    const auto password = api_.Crypto().Seed().Passphrase(seedID, reason);
    const auto data = std::initializer_list<std::string_view>{phrase, password};
    auto [taskID, future] = api_.OTX().InitiateStoreSecret(
        localNymID,
        issuerNymID,
        contract::peer::SecretType::Bip39,
        data,
        setID);

    if (0 == taskID) { return output; }

    const auto result = std::get<0>(future.get());
    success = (otx::LastReplyStatus::MessageSuccess == result);

    return output;
}
}  // namespace opentxs::otx::client::implementation

#undef PAIR_SHUTDOWN
#undef MINIMUM_UNUSED_BAILMENTS

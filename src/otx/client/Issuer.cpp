// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"           // IWYU pragma: associated
#include "1_Internal.hpp"         // IWYU pragma: associated
#include "otx/client/Issuer.hpp"  // IWYU pragma: associated

#include <Bailment.pb.h>
#include <ConnectionInfo.pb.h>
#include <Issuer.pb.h>
#include <PeerReply.pb.h>
#include <PeerRequest.pb.h>
#include <PeerRequestHistory.pb.h>
#include <PeerRequestWorkflow.pb.h>
#include <UnitAccountMap.pb.h>
#include <cstdint>
#include <ctime>
#include <iosfwd>
#include <memory>
#include <sstream>  // IWYU pragma: keep
#include <string_view>
#include <type_traits>

#include "Proto.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/contract/peer/Peer.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/otx/client/Factory.hpp"
#include "internal/otx/client/Issuer.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/Issuer.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/contract/peer/PeerRequestType.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/claim/Data.hpp"
#include "opentxs/identity/wot/claim/Group.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/identity/wot/claim/Section.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/SectionType.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::factory
{
auto Issuer(
    const api::session::Factory& factory,
    const api::session::Wallet& wallet,
    const identifier::Nym& nymID,
    const proto::Issuer& serialized) -> otx::client::Issuer*
{
    using ReturnType = otx::client::implementation::Issuer;

    return new ReturnType(factory, wallet, nymID, serialized);
}

auto Issuer(
    const api::session::Factory& factory,
    const api::session::Wallet& wallet,
    const identifier::Nym& nymID,
    const identifier::Nym& issuerID) -> otx::client::Issuer*
{
    using ReturnType = otx::client::implementation::Issuer;

    return new ReturnType(factory, wallet, nymID, issuerID);
}
}  // namespace opentxs::factory

namespace opentxs::otx::client::implementation
{
Issuer::Issuer(
    const api::session::Factory& factory,
    const api::session::Wallet& wallet,
    const identifier::Nym& nymID,
    const identifier::Nym& issuerID)
    : factory_(factory)
    , wallet_(wallet)
    , version_(current_version_)
    , pairing_code_("")
    , paired_(Flag::Factory(false))
    , nym_id_(nymID)
    , issuer_id_(issuerID)
    , account_map_()
    , peer_requests_()
{
}

Issuer::Issuer(
    const api::session::Factory& factory,
    const api::session::Wallet& wallet,
    const identifier::Nym& nymID,
    const proto::Issuer& serialized)
    : factory_(factory)
    , wallet_(wallet)
    , version_(serialized.version())
    , pairing_code_(serialized.pairingcode())
    , paired_(Flag::Factory(serialized.paired()))
    , nym_id_(nymID)
    , issuer_id_(factory_.NymIDFromBase58(serialized.id()))
    , account_map_()
    , peer_requests_()
{
    Lock lock(lock_);

    for (const auto& it : serialized.accounts()) {
        const auto& type = it.type();
        const auto& unitID = it.unitdefinitionid();
        const auto& accountID = it.accountid();
        account_map_[ClaimToUnit(translate(type))].emplace(
            factory_.UnitIDFromBase58(unitID),
            factory_.IdentifierFromBase58(accountID));
    }

    for (const auto& history : serialized.peerrequests()) {
        const auto& type = history.type();

        for (const auto& workflow : history.workflow()) {
            peer_requests_[translate(type)].emplace(
                factory_.IdentifierFromBase58(workflow.requestid()),
                std::pair<identifier::Generic, bool>(
                    factory_.IdentifierFromBase58(workflow.replyid()),
                    workflow.used()));
        }
    }
}

auto Issuer::toString() const -> UnallocatedCString
{
    Lock lock(lock_);
    std::stringstream output{};
    output << "Connected issuer: " << issuer_id_.asBase58(Context().Crypto())
           << "\n";

    if (pairing_code_.empty()) {
        output << "* Not paired to this issuer\n";
    } else {
        output << "* Pairing code: " << pairing_code_ << "\n";
    }

    const auto nym = wallet_.Nym(issuer_id_);

    if (false == bool(nym)) {
        output << "* The credentials for the issuer nym are not yet downloaded."
               << "\n";

        return output.str();
    }

    const auto& issuerClaims = nym->Claims();
    const auto serverID = issuerClaims.PreferredOTServer();
    const auto contractSection =
        issuerClaims.Section(identity::wot::claim::SectionType::Contract);
    const auto haveAccounts = bool(contractSection);

    if (serverID.empty()) {
        output << "* Issuer nym does not advertise a server.\n";

        return output.str();
    } else {
        output << "* Server ID: " << serverID.asBase58(Context().Crypto())
               << "\n";
    }

    if (false == bool(haveAccounts)) {
        output << "* Issuer nym does not advertise any contracts.\n";

        return output.str();
    }

    output << "* Issued units:\n";

    for (const auto& [type, pGroup] : *contractSection) {
        OT_ASSERT(pGroup);

        const auto& group = *pGroup;

        for (const auto& [id, pClaim] : group) {
            OT_ASSERT(pClaim);

            const auto& notUsed [[maybe_unused]] = id;
            const auto& claim = *pClaim;
            const auto unitID = factory_.UnitIDFromBase58(claim.Value());
            output << " * "
                   << proto::TranslateItemType(
                          static_cast<std::uint32_t>(claim.Type()))
                   << ": " << claim.Value() << "\n";
            const auto accountSet = account_map_.find(ClaimToUnit(type));

            if (account_map_.end() == accountSet) { continue; }

            for (const auto& [unit, accountID] : accountSet->second) {
                if (unit == unitID) {
                    output << "  * Account ID: "
                           << accountID.asBase58(Context().Crypto()) << "\n";
                }
            }
        }
    }

    output << "* Peer requests:\n";

    for (const auto& [type, workflow] : peer_requests_) {
        output << "  * Type: ";

        switch (type) {
            case contract::peer::PeerRequestType::Bailment: {
                output << "bailment";
            } break;
            case contract::peer::PeerRequestType::OutBailment: {
                output << "outbailment";
            } break;
            case contract::peer::PeerRequestType::PendingBailment: {
                output << "pending bailment";
            } break;
            case contract::peer::PeerRequestType::ConnectionInfo: {
                output << "connection info";
            } break;
            case contract::peer::PeerRequestType::StoreSecret: {
                output << "store secret";
            } break;
            case contract::peer::PeerRequestType::VerificationOffer: {
                output << "verification offer";
            } break;
            case contract::peer::PeerRequestType::Faucet: {
                output << "faucet";
            } break;
            case contract::peer::PeerRequestType::Error:
            default: {
                OT_FAIL;
            }
        }

        output << "\n";

        for (const auto& [requestID, it] : workflow) {
            const auto& [replyID, used] = it;
            output << "    * Request: " << String::Factory(requestID)
                   << ", Reply: " << String::Factory(replyID) << " ";

            if (used) {
                output << "(used)";
            } else {
                output << "(unused)";
            }

            output << "\n";
        }
    }

    return output.str();
}

auto Issuer::AccountList(
    const UnitType type,
    const identifier::UnitDefinition& unitID) const
    -> UnallocatedSet<identifier::Generic>
{
    Lock lock(lock_);
    UnallocatedSet<identifier::Generic> output;
    auto accountSet = account_map_.find(type);
    const bool allUnits = unitID.empty();

    if (account_map_.end() == accountSet) { return output; }

    for (const auto& [unit, accountID] : accountSet->second) {
        if (allUnits || (unit == unitID)) { output.emplace(accountID); }
    }

    return output;
}

void Issuer::AddAccount(
    const UnitType type,
    const identifier::UnitDefinition& unitID,
    const identifier::Generic& accountID)
{
    Lock lock(lock_);
    account_map_[type].emplace(unitID, accountID);
}

auto Issuer::add_request(
    const Lock& lock,
    const contract::peer::PeerRequestType type,
    const identifier::Generic& requestID,
    const identifier::Generic& replyID) -> bool
{
    OT_ASSERT(verify_lock(lock));

    auto [found, it] = find_request(lock, type, requestID);
    const auto& notUsed [[maybe_unused]] = it;

    if (found) {
        LogError()(OT_PRETTY_CLASS())("Request ")(requestID)(" already exists.")
            .Flush();

        return false;
    }

    peer_requests_[type].emplace(
        requestID, std::pair<identifier::Generic, bool>(replyID, false));

    return true;
}

auto Issuer::AddReply(
    const contract::peer::PeerRequestType type,
    const identifier::Generic& requestID,
    const identifier::Generic& replyID) -> bool
{
    Lock lock(lock_);
    auto [found, it] = find_request(lock, type, requestID);
    auto& [reply, used] = it->second;

    if (false == found) {
        LogDetail()(OT_PRETTY_CLASS())("Request ")(requestID)(" not found.")
            .Flush();

        return add_request(lock, type, requestID, replyID);
    }

    reply = replyID;
    used = false;

    return true;
}

auto Issuer::AddRequest(
    const contract::peer::PeerRequestType type,
    const identifier::Generic& requestID) -> bool
{
    Lock lock(lock_);
    // ReplyID is blank because we don't know it yet.
    auto replyID = identifier::Generic{};

    return add_request(lock, type, requestID, replyID);
}

auto Issuer::BailmentInitiated(const identifier::UnitDefinition& unitID) const
    -> bool
{
    LogVerbose()(OT_PRETTY_CLASS())(
        "Searching for initiated bailment requests for unit ")(unitID)
        .Flush();
    Lock lock(lock_);
    std::size_t count{0};
    const auto requests = get_requests(
        lock,
        contract::peer::PeerRequestType::Bailment,
        RequestStatus::Requested);
    LogVerbose()(OT_PRETTY_CLASS())("Have ")(requests.size())(
        " initiated requests.")
        .Flush();

    for (const auto& [requestID, a, b] : requests) {
        const auto& replyID [[maybe_unused]] = a;
        const auto& isUsed [[maybe_unused]] = b;
        std::time_t notUsed{0};
        auto request = proto::PeerRequest{};
        auto loaded = wallet_.Internal().PeerRequest(
            nym_id_,
            requestID,
            otx::client::StorageBox::SENTPEERREQUEST,
            notUsed,
            request);
        if (false == loaded) {
            loaded = wallet_.Internal().PeerRequest(
                nym_id_,
                requestID,
                otx::client::StorageBox::FINISHEDPEERREQUEST,
                notUsed,
                request);
        }

        if (loaded) {
            const auto requestType =
                factory_.IdentifierFromBase58(request.bailment().unitid());

            if (unitID == requestType) {
                ++count;
            } else {
                LogVerbose()(OT_PRETTY_CLASS())("Request ")(
                    requestID)(" is wrong type (")(request.bailment().unitid())(
                    ")")
                    .Flush();
            }
        } else {
            LogVerbose()(OT_PRETTY_CLASS())("Failed to serialize request: ")(
                requestID)
                .Flush();
        }
    }

    return 0 != count;
}

auto Issuer::BailmentInstructions(
    const api::Session& client,
    const identifier::UnitDefinition& unitID,
    const bool onlyUnused) const -> UnallocatedVector<Issuer::BailmentDetails>
{
    Lock lock(lock_);
    UnallocatedVector<BailmentDetails> output{};
    const auto replies = get_requests(
        lock,
        contract::peer::PeerRequestType::Bailment,
        (onlyUnused) ? RequestStatus::Unused : RequestStatus::Replied);

    for (const auto& [requestID, replyID, isUsed] : replies) {
        std::time_t notUsed{0};
        const auto& notUsed2 [[maybe_unused]] = isUsed;
        auto request = proto::PeerRequest{};
        auto loaded = wallet_.Internal().PeerRequest(
            nym_id_,
            requestID,
            otx::client::StorageBox::FINISHEDPEERREQUEST,
            notUsed,
            request);

        if (false == loaded) {
            loaded = wallet_.Internal().PeerRequest(
                nym_id_,
                requestID,
                otx::client::StorageBox::SENTPEERREQUEST,
                notUsed,
                request);
        }

        if (loaded) {
            if (request.bailment().unitid() !=
                unitID.asBase58(Context().Crypto())) {
                continue;
            }

            auto reply = proto::PeerReply{};
            auto loadedreply = wallet_.Internal().PeerReply(
                nym_id_,
                replyID,
                otx::client::StorageBox::PROCESSEDPEERREPLY,
                reply);

            if (false == loadedreply) {
                reply = proto::PeerReply{};
                loadedreply = wallet_.Internal().PeerReply(
                    nym_id_,
                    replyID,
                    otx::client::StorageBox::INCOMINGPEERREPLY,
                    reply);
            }

            if (false == loadedreply) {
                LogVerbose()(OT_PRETTY_CLASS())("Failed to serialize reply: ")(
                    replyID)
                    .Flush();
            } else {
                auto nym = wallet_.Nym(issuer_id_);
                auto bailmentreply =
                    client.Factory().InternalSession().BailmentReply(
                        nym, reply);
                output.emplace_back(requestID, bailmentreply);
            }
        } else {
            LogVerbose()(OT_PRETTY_CLASS())("Failed to serialize request: ")(
                requestID)
                .Flush();
        }
    }

    return output;
}

auto Issuer::ConnectionInfo(
    const api::Session& client,
    const contract::peer::ConnectionInfoType type) const
    -> UnallocatedVector<Issuer::ConnectionDetails>
{
    LogVerbose()(OT_PRETTY_CLASS())("Searching for type ")(
        static_cast<std::uint32_t>(type))(
        " connection info requests (which have replies).")
        .Flush();
    Lock lock(lock_);
    UnallocatedVector<ConnectionDetails> output{};
    const auto replies = get_requests(
        lock,
        contract::peer::PeerRequestType::ConnectionInfo,
        RequestStatus::Replied);
    LogVerbose()(OT_PRETTY_CLASS())("Have ")(replies.size())(" total requests.")
        .Flush();

    for (const auto& [requestID, replyID, isUsed] : replies) {
        std::time_t notUsed{0};
        const auto& notUsed2 [[maybe_unused]] = isUsed;
        auto request = proto::PeerRequest{};
        auto loaded = wallet_.Internal().PeerRequest(
            nym_id_,
            requestID,
            otx::client::StorageBox::FINISHEDPEERREQUEST,
            notUsed,
            request);

        if (false == loaded) {
            loaded = wallet_.Internal().PeerRequest(
                nym_id_,
                requestID,
                otx::client::StorageBox::SENTPEERREQUEST,
                notUsed,
                request);
        }

        if (loaded) {
            if (type != translate(request.connectioninfo().type())) {
                LogVerbose()(OT_PRETTY_CLASS())("Request ")(
                    requestID)(" is wrong type (")(
                    request.connectioninfo().type())(")")
                    .Flush();

                continue;
            }

            auto reply = proto::PeerReply{};
            auto loadedreply = wallet_.Internal().PeerReply(
                nym_id_,
                replyID,
                otx::client::StorageBox::PROCESSEDPEERREPLY,
                reply);

            if (false == loadedreply) {
                reply = proto::PeerReply{};
                loadedreply = wallet_.Internal().PeerReply(
                    nym_id_,
                    replyID,
                    otx::client::StorageBox::INCOMINGPEERREPLY,
                    reply);
            }

            if (loadedreply) {
                auto nym = wallet_.Nym(issuer_id_);
                auto connectionreply =
                    client.Factory().InternalSession().ConnectionReply(
                        nym, reply);
                output.emplace_back(requestID, connectionreply);
            } else {
                LogVerbose()(OT_PRETTY_CLASS())(
                    ": Failed to serialize reply: ")(replyID)
                    .Flush();
            }
        } else {
            LogVerbose()(OT_PRETTY_CLASS())("Failed to serialize request: ")(
                requestID)
                .Flush();
        }
    }

    return output;
}

auto Issuer::ConnectionInfoInitiated(
    const contract::peer::ConnectionInfoType type) const -> bool
{
    LogVerbose()(OT_PRETTY_CLASS())("Searching for all type ")(
        static_cast<std::uint32_t>(type))(" connection info requests.")
        .Flush();
    Lock lock(lock_);
    std::size_t count{0};
    const auto requests = get_requests(
        lock,
        contract::peer::PeerRequestType::ConnectionInfo,
        RequestStatus::All);
    LogVerbose()(OT_PRETTY_CLASS())("Have ")(requests.size())(
        " total requests.")
        .Flush();

    for (const auto& [requestID, a, b] : requests) {
        const auto& replyID [[maybe_unused]] = a;
        const auto& isUsed [[maybe_unused]] = b;
        std::time_t notUsed{0};
        auto request = proto::PeerRequest{};
        auto loaded = wallet_.Internal().PeerRequest(
            nym_id_,
            requestID,
            otx::client::StorageBox::SENTPEERREQUEST,
            notUsed,
            request);

        if (false == loaded) {
            loaded = wallet_.Internal().PeerRequest(
                nym_id_,
                requestID,
                otx::client::StorageBox::FINISHEDPEERREQUEST,
                notUsed,
                request);
        }

        if (loaded) {

            if (type == translate(request.connectioninfo().type())) {
                ++count;
            } else {
                LogVerbose()(OT_PRETTY_CLASS())("Request ")(
                    requestID)(" is wrong type (")(
                    request.connectioninfo().type())(")")
                    .Flush();
            }
        } else {
            LogVerbose()(OT_PRETTY_CLASS())("Failed to serialize request: ")(
                requestID);
        }
    }

    return 0 != count;
}

auto Issuer::find_request(
    const Lock& lock,
    const contract::peer::PeerRequestType type,
    const identifier::Generic& requestID)
    -> std::pair<bool, Issuer::Workflow::iterator>
{
    OT_ASSERT(verify_lock(lock));

    auto& work = peer_requests_[type];
    auto it = work.find(requestID);

    return {work.end() != it, it};
}

auto Issuer::GetRequests(
    const contract::peer::PeerRequestType type,
    const Issuer::RequestStatus state) const
    -> UnallocatedSet<
        std::tuple<identifier::Generic, identifier::Generic, bool>>
{
    Lock lock(lock_);

    return get_requests(lock, type, state);
}

auto Issuer::get_requests(
    const Lock& lock,
    const contract::peer::PeerRequestType type,
    const Issuer::RequestStatus state) const
    -> UnallocatedSet<
        std::tuple<identifier::Generic, identifier::Generic, bool>>
{
    OT_ASSERT(verify_lock(lock));

    UnallocatedSet<std::tuple<identifier::Generic, identifier::Generic, bool>>
        output;

    if (Issuer::RequestStatus::None == state) { return output; }

    const auto map = peer_requests_.find(type);

    if (peer_requests_.end() == map) { return output; }

    for (const auto& [requestID, data] : map->second) {
        const auto& [replyID, used] = data;

        switch (state) {
            case Issuer::RequestStatus::Unused: {
                const bool exists = (false == replyID.empty());
                const bool unused = (false == used);

                if (exists && unused) {
                    output.emplace(requestID, replyID, used);
                }
            } break;
            case Issuer::RequestStatus::Replied: {
                if (false == replyID.empty()) {
                    output.emplace(requestID, replyID, used);
                }
            } break;
            case Issuer::RequestStatus::Requested: {
                if (replyID.empty()) {
                    output.emplace(requestID, identifier::Generic{}, false);
                }
            } break;
            case Issuer::RequestStatus::All: {
                output.emplace(requestID, replyID, used);
            } break;
            case Issuer::RequestStatus::None:
            default: {
            }
        }
    }

    return output;
}

auto Issuer::Paired() const -> bool { return paired_.get(); }

auto Issuer::PairingCode() const -> const UnallocatedCString&
{
    return pairing_code_;
}

auto Issuer::PrimaryServer() const -> identifier::Notary
{
    Lock lock(lock_);

    auto nym = wallet_.Nym(issuer_id_);

    if (false == bool(nym)) { return {}; }

    return nym->Claims().PreferredOTServer();
}

auto Issuer::RemoveAccount(
    const UnitType type,
    const identifier::UnitDefinition& unitID,
    const identifier::Generic& accountID) -> bool
{
    Lock lock(lock_);
    auto accountSet = account_map_.find(type);

    if (account_map_.end() == accountSet) { return false; }
    auto& accounts = accountSet->second;
    auto it = accounts.find({unitID, accountID});

    if (accounts.end() == it) { return false; }

    accounts.erase(it);

    return true;
}

auto Issuer::RequestTypes() const
    -> UnallocatedSet<contract::peer::PeerRequestType>
{
    Lock lock(lock_);
    UnallocatedSet<contract::peer::PeerRequestType> output{};

    for (const auto& [type, map] : peer_requests_) {
        const auto& notUsed [[maybe_unused]] = map;
        output.emplace(type);
    }

    return output;
}

auto Issuer::Serialize(proto::Issuer& output) const -> bool
{
    Lock lock(lock_);
    output.set_version(version_);
    output.set_id(issuer_id_.asBase58(Context().Crypto()));
    output.set_paired(paired_.get());
    output.set_pairingcode(pairing_code_);

    for (const auto& [type, accountSet] : account_map_) {
        for (const auto& [unitID, accountID] : accountSet) {
            auto& map = *output.add_accounts();
            map.set_version(version_);
            map.set_type(translate(UnitToClaim(type)));
            map.set_unitdefinitionid(unitID.asBase58(Context().Crypto()));
            map.set_accountid(accountID.asBase58(Context().Crypto()));
        }
    }

    for (const auto& [type, work] : peer_requests_) {
        auto& history = *output.add_peerrequests();
        history.set_version(version_);
        history.set_type(translate(type));

        for (const auto& [request, data] : work) {
            const auto& [reply, isUsed] = data;
            auto& workflow = *history.add_workflow();
            workflow.set_version(version_);
            workflow.set_requestid(request.asBase58(Context().Crypto()));
            workflow.set_replyid(reply.asBase58(Context().Crypto()));
            workflow.set_used(isUsed);
        }
    }

    OT_ASSERT(proto::Validate(output, VERBOSE));

    return true;
}

void Issuer::SetPaired(const bool paired) { paired_->Set(paired); }

void Issuer::SetPairingCode(const UnallocatedCString& code)
{
    Lock lock(lock_);
    pairing_code_ = code;
    paired_->On();
}

auto Issuer::SetUsed(
    const contract::peer::PeerRequestType type,
    const identifier::Generic& requestID,
    const bool isUsed) -> bool
{
    Lock lock(lock_);
    auto [found, it] = find_request(lock, type, requestID);
    auto& [reply, used] = it->second;
    const auto& notUsed [[maybe_unused]] = reply;

    if (false == found) { return false; }

    used = isUsed;

    return true;
}

auto Issuer::StoreSecretComplete() const -> bool
{
    Lock lock(lock_);
    const auto storeSecret = get_requests(
        lock,
        contract::peer::PeerRequestType::StoreSecret,
        RequestStatus::Replied);

    return 0 != storeSecret.size();
}

auto Issuer::StoreSecretInitiated() const -> bool
{
    Lock lock(lock_);
    const auto storeSecret = get_requests(
        lock, contract::peer::PeerRequestType::StoreSecret, RequestStatus::All);

    return 0 != storeSecret.size();
}

Issuer::~Issuer() = default;
}  // namespace opentxs::otx::client::implementation

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/client/Issuer.hpp"  // IWYU pragma: associated

#include <Issuer.pb.h>
#include <PeerRequestHistory.pb.h>
#include <PeerRequestWorkflow.pb.h>
#include <UnitAccountMap.pb.h>
#include <cstdint>
#include <ctime>
#include <memory>
#include <sstream>  // IWYU pragma: keep
#include <string_view>

#include "internal/core/String.hpp"
#include "internal/otx/client/Factory.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/Issuer.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/contract/peer/Types.internal.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/contract/peer/Reply.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/contract/peer/reply/Bailment.hpp"
#include "opentxs/core/contract/peer/reply/Connection.hpp"
#include "opentxs/core/contract/peer/request/Bailment.hpp"
#include "opentxs/core/contract/peer/request/Connection.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/claim/Data.hpp"
#include "opentxs/identity/wot/claim/Group.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/identity/wot/claim/Section.hpp"      // IWYU pragma: keep
#include "opentxs/identity/wot/claim/SectionType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/identity/wot/claim/Types.internal.hpp"
#include "opentxs/otx/client/StorageBox.hpp"  // IWYU pragma: keep
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto Issuer(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const api::session::Wallet& wallet,
    const identifier::Nym& nymID,
    const proto::Issuer& serialized) -> otx::client::Issuer*
{
    using ReturnType = otx::client::implementation::Issuer;

    return new ReturnType(crypto, factory, wallet, nymID, serialized);
}

auto Issuer(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const api::session::Wallet& wallet,
    const identifier::Nym& nymID,
    const identifier::Nym& issuerID) -> otx::client::Issuer*
{
    using ReturnType = otx::client::implementation::Issuer;

    return new ReturnType(crypto, factory, wallet, nymID, issuerID);
}
}  // namespace opentxs::factory

namespace opentxs::otx::client::implementation
{
Issuer::Issuer(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const api::session::Wallet& wallet,
    const identifier::Nym& nymID,
    const identifier::Nym& issuerID)
    : crypto_(crypto)
    , factory_(factory)
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
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const api::session::Wallet& wallet,
    const identifier::Nym& nymID,
    const proto::Issuer& serialized)
    : crypto_(crypto)
    , factory_(factory)
    , wallet_(wallet)
    , version_(serialized.version())
    , pairing_code_(serialized.pairingcode())
    , paired_(Flag::Factory(serialized.paired()))
    , nym_id_(nymID)
    , issuer_id_(factory_.NymIDFromBase58(serialized.id()))
    , account_map_()
    , peer_requests_()
{
    auto lock = Lock{lock_};

    for (const auto& it : serialized.accounts()) {
        const auto& type = it.type();
        const auto& unitID = it.unitdefinitionid();
        const auto& accountID = it.accountid();
        account_map_[ClaimToUnit(translate(type))].emplace(
            factory_.UnitIDFromBase58(unitID),
            factory_.AccountIDFromBase58(accountID));
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
    auto lock = Lock{lock_};
    std::stringstream output{};
    output << "Connected issuer: " << issuer_id_.asBase58(crypto_) << "\n";

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
        output << "* Server ID: " << serverID.asBase58(crypto_) << "\n";
    }

    if (false == bool(haveAccounts)) {
        output << "* Issuer nym does not advertise any contracts.\n";

        return output.str();
    }

    output << "* Issued units:\n";

    for (const auto& [type, pGroup] : *contractSection) {
        assert_false(nullptr == pGroup);

        const auto& group = *pGroup;

        for (const auto& [id, pClaim] : group) {
            assert_false(nullptr == pClaim);

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
                    output << "  * Account ID: " << accountID.asBase58(crypto_)
                           << "\n";
                }
            }
        }
    }

    output << "* Peer requests:\n";

    for (const auto& [type, workflow] : peer_requests_) {
        output << "  * Type: " << print(type) << '\n';

        for (const auto& [requestID, it] : workflow) {
            const auto& [replyID, used] = it;
            output << "    * Request: " << String::Factory(requestID, crypto_)
                   << ", Reply: " << String::Factory(replyID, crypto_) << " ";

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
    -> UnallocatedSet<identifier::Account>
{
    auto lock = Lock{lock_};
    UnallocatedSet<identifier::Account> output;
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
    const identifier::Account& accountID)
{
    auto lock = Lock{lock_};
    account_map_[type].emplace(unitID, accountID);
}

auto Issuer::add_request(
    const Lock& lock,
    const contract::peer::RequestType type,
    const identifier::Generic& requestID,
    const identifier::Generic& replyID) -> bool
{
    assert_true(verify_lock(lock));

    auto [found, it] = find_request(lock, type, requestID);
    const auto& notUsed [[maybe_unused]] = it;

    if (found) {
        LogError()()("Request ")(requestID, crypto_)(" already exists.")
            .Flush();

        return false;
    }

    peer_requests_[type].emplace(
        requestID, std::pair<identifier::Generic, bool>(replyID, false));

    return true;
}

auto Issuer::AddReply(
    const contract::peer::RequestType type,
    const identifier::Generic& requestID,
    const identifier::Generic& replyID) -> bool
{
    auto lock = Lock{lock_};
    auto [found, it] = find_request(lock, type, requestID);
    auto& [reply, used] = it->second;

    if (false == found) {
        LogDetail()()("Request ")(requestID, crypto_)(" not found.").Flush();

        return add_request(lock, type, requestID, replyID);
    }

    reply = replyID;
    used = false;

    return true;
}

auto Issuer::AddRequest(
    const contract::peer::RequestType type,
    const identifier::Generic& requestID) -> bool
{
    auto lock = Lock{lock_};
    // ReplyID is blank because we don't know it yet.
    auto replyID = identifier::Generic{};

    return add_request(lock, type, requestID, replyID);
}

auto Issuer::BailmentInitiated(const identifier::UnitDefinition& unitID) const
    -> bool
{
    LogVerbose()()("Searching for initiated bailment requests for unit ")(
        unitID, crypto_)
        .Flush();
    auto lock = Lock{lock_};
    std::size_t count{0};
    const auto requests = get_requests(
        lock, contract::peer::RequestType::Bailment, RequestStatus::Requested);
    LogVerbose()()("Have ")(requests.size())(" initiated requests.").Flush();

    for (const auto& [requestID, a, b] : requests) {
        const auto& replyID [[maybe_unused]] = a;
        const auto& isUsed [[maybe_unused]] = b;
        auto request = wallet_.PeerRequest(
            nym_id_, requestID, otx::client::StorageBox::SENTPEERREQUEST);

        if (false == request.IsValid()) {
            request = wallet_.PeerRequest(
                nym_id_,
                requestID,
                otx::client::StorageBox::FINISHEDPEERREQUEST);
        }

        if (request.IsValid()) {
            const auto& requestType = request.asBailment().Unit();

            if (unitID == requestType) {
                ++count;
            } else {
                LogVerbose()()("Request ")(requestID, crypto_)(
                    " is wrong type (")(requestType, crypto_)(")")
                    .Flush();
            }
        } else {
            LogVerbose()()("Failed to serialize request: ")(requestID, crypto_)
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
    auto lock = Lock{lock_};
    UnallocatedVector<BailmentDetails> output{};
    const auto replies = get_requests(
        lock,
        contract::peer::RequestType::Bailment,
        (onlyUnused) ? RequestStatus::Unused : RequestStatus::Replied);

    for (const auto& [requestID, replyID, isUsed] : replies) {
        auto request = wallet_.PeerRequest(
            nym_id_, requestID, otx::client::StorageBox::FINISHEDPEERREQUEST);

        if (false == request.IsValid()) {
            request = wallet_.PeerRequest(
                nym_id_, requestID, otx::client::StorageBox::SENTPEERREQUEST);
        }

        if (request.IsValid()) {
            if (request.asBailment().Unit() != unitID) { continue; }

            auto reply = wallet_.PeerReply(
                nym_id_, replyID, otx::client::StorageBox::PROCESSEDPEERREPLY);

            if (false == reply.IsValid()) {
                reply = wallet_.PeerReply(
                    nym_id_,
                    replyID,
                    otx::client::StorageBox::INCOMINGPEERREPLY);
            }

            if (false == reply.IsValid()) {
                LogVerbose()()("Failed to serialize reply: ")(replyID, crypto_)
                    .Flush();
            } else {
                auto nym = wallet_.Nym(issuer_id_);
                output.emplace_back(requestID, std::move(reply).asBailment());
            }
        } else {
            LogVerbose()()("Failed to serialize request: ")(requestID, crypto_)
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
    LogVerbose()()("Searching for type ")(static_cast<std::uint32_t>(type))(
        " connection info requests (which have replies).")
        .Flush();
    auto lock = Lock{lock_};
    UnallocatedVector<ConnectionDetails> output{};
    const auto replies = get_requests(
        lock,
        contract::peer::RequestType::ConnectionInfo,
        RequestStatus::Replied);
    LogVerbose()()("Have ")(replies.size())(" total requests.").Flush();

    for (const auto& [requestID, replyID, isUsed] : replies) {
        auto request = wallet_.Internal().PeerRequest(
            nym_id_, requestID, otx::client::StorageBox::FINISHEDPEERREQUEST);

        if (false == request.IsValid()) {
            request = wallet_.PeerRequest(
                nym_id_, requestID, otx::client::StorageBox::SENTPEERREQUEST);
        }

        if (request.IsValid()) {
            if (const auto kind = request.asConnection().Kind(); type != kind) {
                LogVerbose()()("Request ")(requestID, crypto_)(
                    " is wrong type (")(print(kind))(")")
                    .Flush();

                continue;
            }

            auto reply = wallet_.Internal().PeerReply(
                nym_id_, replyID, otx::client::StorageBox::PROCESSEDPEERREPLY);

            if (false == reply.IsValid()) {
                reply = wallet_.Internal().PeerReply(
                    nym_id_,
                    replyID,
                    otx::client::StorageBox::INCOMINGPEERREPLY);
            }

            if (reply.IsValid()) {
                auto nym = wallet_.Nym(issuer_id_);
                output.emplace_back(requestID, std::move(reply).asConnection());
            } else {
                LogVerbose()()(": Failed to serialize reply: ")(
                    replyID, crypto_)
                    .Flush();
            }
        } else {
            LogVerbose()()("Failed to serialize request: ")(requestID, crypto_)
                .Flush();
        }
    }

    return output;
}

auto Issuer::ConnectionInfoInitiated(
    const contract::peer::ConnectionInfoType type) const -> bool
{
    LogVerbose()()("Searching for all type ")(static_cast<std::uint32_t>(type))(
        " connection info requests.")
        .Flush();
    auto lock = Lock{lock_};
    std::size_t count{0};
    const auto requests = get_requests(
        lock, contract::peer::RequestType::ConnectionInfo, RequestStatus::All);
    LogVerbose()()("Have ")(requests.size())(" total requests.").Flush();

    for (const auto& [requestID, replyID, isUsed] : requests) {
        auto request = wallet_.Internal().PeerRequest(
            nym_id_, requestID, otx::client::StorageBox::SENTPEERREQUEST);

        if (false == request.IsValid()) {
            request = wallet_.Internal().PeerRequest(
                nym_id_,
                requestID,
                otx::client::StorageBox::FINISHEDPEERREQUEST);
        }

        if (request.IsValid()) {

            if (const auto kind = request.asConnection().Kind(); type == kind) {
                ++count;
            } else {
                LogVerbose()()("Request ")(requestID, crypto_)(
                    " is wrong type (")(print(kind))(")")
                    .Flush();
            }
        } else {
            LogVerbose()()("Failed to serialize request: ")(requestID, crypto_);
        }
    }

    return 0 != count;
}

auto Issuer::find_request(
    const Lock& lock,
    const contract::peer::RequestType type,
    const identifier::Generic& requestID)
    -> std::pair<bool, Issuer::Workflow::iterator>
{
    assert_true(verify_lock(lock));

    auto& work = peer_requests_[type];
    auto it = work.find(requestID);

    return {work.end() != it, it};
}

auto Issuer::GetRequests(
    const contract::peer::RequestType type,
    const Issuer::RequestStatus state) const
    -> UnallocatedSet<
        std::tuple<identifier::Generic, identifier::Generic, bool>>
{
    auto lock = Lock{lock_};

    return get_requests(lock, type, state);
}

auto Issuer::get_requests(
    const Lock& lock,
    const contract::peer::RequestType type,
    const Issuer::RequestStatus state) const
    -> UnallocatedSet<
        std::tuple<identifier::Generic, identifier::Generic, bool>>
{
    assert_true(verify_lock(lock));

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
    auto lock = Lock{lock_};

    auto nym = wallet_.Nym(issuer_id_);

    if (false == bool(nym)) { return {}; }

    return nym->Claims().PreferredOTServer();
}

auto Issuer::RemoveAccount(
    const UnitType type,
    const identifier::UnitDefinition& unitID,
    const identifier::Account& accountID) -> bool
{
    auto lock = Lock{lock_};
    auto accountSet = account_map_.find(type);

    if (account_map_.end() == accountSet) { return false; }
    auto& accounts = accountSet->second;
    auto it = accounts.find({unitID, accountID});

    if (accounts.end() == it) { return false; }

    accounts.erase(it);

    return true;
}

auto Issuer::RequestTypes() const -> UnallocatedSet<contract::peer::RequestType>
{
    auto lock = Lock{lock_};
    UnallocatedSet<contract::peer::RequestType> output{};

    for (const auto& [type, map] : peer_requests_) {
        const auto& notUsed [[maybe_unused]] = map;
        output.emplace(type);
    }

    return output;
}

auto Issuer::Serialize(proto::Issuer& output) const -> bool
{
    auto lock = Lock{lock_};
    output.set_version(version_);
    output.set_id(issuer_id_.asBase58(crypto_));
    output.set_paired(paired_.get());
    output.set_pairingcode(pairing_code_);

    for (const auto& [type, accountSet] : account_map_) {
        for (const auto& [unitID, accountID] : accountSet) {
            auto& map = *output.add_accounts();
            map.set_version(version_);
            map.set_type(translate(UnitToClaim(type)));
            map.set_unitdefinitionid(unitID.asBase58(crypto_));
            map.set_accountid(accountID.asBase58(crypto_));
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
            workflow.set_requestid(request.asBase58(crypto_));
            workflow.set_replyid(reply.asBase58(crypto_));
            workflow.set_used(isUsed);
        }
    }

    assert_true(proto::Validate(output, VERBOSE));

    return true;
}

void Issuer::SetPaired(const bool paired) { paired_->Set(paired); }

void Issuer::SetPairingCode(const UnallocatedCString& code)
{
    auto lock = Lock{lock_};
    pairing_code_ = code;
    paired_->On();
}

auto Issuer::SetUsed(
    const contract::peer::RequestType type,
    const identifier::Generic& requestID,
    const bool isUsed) -> bool
{
    auto lock = Lock{lock_};
    auto [found, it] = find_request(lock, type, requestID);
    auto& [reply, used] = it->second;
    const auto& notUsed [[maybe_unused]] = reply;

    if (false == found) { return false; }

    used = isUsed;

    return true;
}

auto Issuer::StoreSecretComplete() const -> bool
{
    auto lock = Lock{lock_};
    const auto storeSecret = get_requests(
        lock, contract::peer::RequestType::StoreSecret, RequestStatus::Replied);

    return 0 != storeSecret.size();
}

auto Issuer::StoreSecretInitiated() const -> bool
{
    auto lock = Lock{lock_};
    const auto storeSecret = get_requests(
        lock, contract::peer::RequestType::StoreSecret, RequestStatus::All);

    return 0 != storeSecret.size();
}

Issuer::~Issuer() = default;
}  // namespace opentxs::otx::client::implementation

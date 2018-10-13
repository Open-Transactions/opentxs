// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Issuer.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/contact/ContactSection.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"

#include <cstdint>
#include <mutex>

#include "Issuer.hpp"

#define CURRENT_VERSION 1

#define OT_METHOD "opentxs::api::client::implementation::Issuer::"

namespace opentxs
{
api::client::Issuer* Factory::Issuer(
    const api::Wallet& wallet,
    const Identifier& nymID,
    const proto::Issuer& serialized)
{
    return new api::client::implementation::Issuer(wallet, nymID, serialized);
}

api::client::Issuer* Factory::Issuer(
    const api::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& issuerID)
{
    return new api::client::implementation::Issuer(wallet, nymID, issuerID);
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
Issuer::Issuer(
    const api::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& issuerID)
    : wallet_(wallet)
    , version_(CURRENT_VERSION)
    , pairing_code_("")
    , paired_(Flag::Factory(false))
    , nym_id_(Identifier::Factory(nymID))
    , issuer_id_(Identifier::Factory(issuerID))
    , account_map_()
    , peer_requests_()
{
}

Issuer::Issuer(
    const api::Wallet& wallet,
    const Identifier& nymID,
    const proto::Issuer& serialized)
    : wallet_(wallet)
    , version_(serialized.version())
    , pairing_code_(serialized.pairingcode())
    , paired_(Flag::Factory(serialized.paired()))
    , nym_id_(Identifier::Factory(nymID))
    , issuer_id_(Identifier::Factory(serialized.id()))
    , account_map_()
    , peer_requests_()
{
    Lock lock(lock_);

    for (const auto& it : serialized.accounts()) {
        const auto& type = it.type();
        const auto& unitID = it.unitdefinitionid();
        const auto& accountID = it.accountid();
        account_map_[type].emplace(
            Identifier::Factory(unitID), Identifier::Factory(accountID));
    }

    for (const auto& history : serialized.peerrequests()) {
        const auto& type = history.type();

        for (const auto& workflow : history.workflow()) {
            peer_requests_[type].emplace(
                Identifier::Factory(workflow.requestid()),
                std::pair<OTIdentifier, bool>(
                    Identifier::Factory(workflow.replyid()), workflow.used()));
        }
    }
}

Issuer::operator std::string() const
{
    Lock lock(lock_);
    std::stringstream output{};
    output << "Connected issuer: " << issuer_id_->str() << "\n";

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
        issuerClaims.Section(proto::CONTACTSECTION_CONTRACT);
    const auto haveAccounts = bool(contractSection);

    if (serverID->empty()) {
        output << "* Issuer nym does not advertise a server.\n";

        return output.str();
    } else {
        output << "* Server ID: " << serverID->str() << "\n";
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

            const auto& notUsed[[maybe_unused]] = id;
            const auto& claim = *pClaim;
            const auto unitID = Identifier::Factory(claim.Value());
            output << " * "
                   << proto::TranslateItemType(
                          static_cast<std::uint32_t>(claim.Type()))
                   << ": " << claim.Value() << "\n";
            const auto accountSet = account_map_.find(type);

            if (account_map_.end() == accountSet) { continue; }

            for (const auto& [unit, accountID] : accountSet->second) {
                if (unit == unitID) {
                    output << "  * Account ID: " << accountID->str() << "\n";
                }
            }
        }
    }

    output << "* Peer requests:\n";

    for (const auto& [type, workflow] : peer_requests_) {
        output << "  * Type: ";

        switch (type) {
            case proto::PEERREQUEST_BAILMENT: {
                output << "bailment";
            } break;
            case proto::PEERREQUEST_OUTBAILMENT: {
                output << "outbailment";
            } break;
            case proto::PEERREQUEST_PENDINGBAILMENT: {
                output << "pending bailment";
            } break;
            case proto::PEERREQUEST_CONNECTIONINFO: {
                output << "connection info";
            } break;
            case proto::PEERREQUEST_STORESECRET: {
                output << "store secret";
            } break;
            case proto::PEERREQUEST_VERIFICATIONOFFER: {
                output << "verification offer";
            } break;
            case proto::PEERREQUEST_FAUCET: {
                output << "faucet";
            } break;
            default: {
                OT_FAIL
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

std::set<OTIdentifier> Issuer::AccountList(
    const proto::ContactItemType type,
    const Identifier& unitID) const
{
    Lock lock(lock_);
    std::set<OTIdentifier> output;
    auto accountSet = account_map_.find(type);
    const bool allUnits = unitID.empty();

    if (account_map_.end() == accountSet) { return output; }

    for (const auto& [unit, accountID] : accountSet->second) {
        if (allUnits || (unit == unitID)) { output.emplace(accountID); }
    }

    return output;
}

void Issuer::AddAccount(
    const proto::ContactItemType type,
    const Identifier& unitID,
    const Identifier& accountID)
{
    Lock lock(lock_);
    account_map_[type].emplace(unitID, accountID);
}

bool Issuer::add_request(
    const Lock& lock,
    const proto::PeerRequestType type,
    const Identifier& requestID,
    const Identifier& replyID)
{
    OT_ASSERT(verify_lock(lock))

    auto [found, it] = find_request(lock, type, requestID);
    const auto& notUsed[[maybe_unused]] = it;

    if (found) {
        otErr << OT_METHOD << __FUNCTION__ << ": Request "
              << String::Factory(requestID) << " already exists." << std::endl;

        return false;
    }

    peer_requests_[type].emplace(
        requestID, std::pair<OTIdentifier, bool>(replyID, false));

    return true;
}

bool Issuer::AddReply(
    const proto::PeerRequestType type,
    const Identifier& requestID,
    const Identifier& replyID)
{
    Lock lock(lock_);
    auto [found, it] = find_request(lock, type, requestID);
    auto& [reply, used] = it->second;

    if (false == found) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Request ")(requestID)(
            " not found.")
            .Flush();

        return add_request(lock, type, requestID, replyID);
    }

    reply = Identifier::Factory(replyID);
    used = false;

    return true;
}

bool Issuer::AddRequest(
    const proto::PeerRequestType type,
    const Identifier& requestID)
{
    Lock lock(lock_);
    // ReplyID is blank because we don't know it yet.
    auto replyID = Identifier::Factory();

    return add_request(lock, type, requestID, replyID);
}

bool Issuer::BailmentInitiated(const Identifier& unitID) const
{
    LogVerbose(OT_METHOD)(__FUNCTION__)(
        ": Searching for initiated bailment requests for unit ")(unitID)
        .Flush();
    Lock lock(lock_);
    std::size_t count{0};
    const auto requests = get_requests(
        lock, proto::PEERREQUEST_BAILMENT, RequestStatus::Requested);
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Have ")(requests.size())(
        " initiated requests.")
        .Flush();

    for (const auto& [requestID, a, b] : requests) {
        const auto& replyID[[maybe_unused]] = a;
        const auto& isUsed[[maybe_unused]] = b;
        std::time_t notUsed{0};
        auto request = wallet_.PeerRequest(
            nym_id_, requestID, StorageBox::SENTPEERREQUEST, notUsed);

        if (false == bool(request)) {
            request = wallet_.PeerRequest(
                nym_id_, requestID, StorageBox::FINISHEDPEERREQUEST, notUsed);
        }

        OT_ASSERT(request);

        const auto requestType =
            Identifier::Factory(request->bailment().unitid());

        if (unitID == requestType) {
            ++count;
        } else {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Request ")(requestID)(
                " is wrong type (")(request->bailment().unitid())(")")
                .Flush();
        }
    }

    return 0 != count;
}

std::vector<Issuer::BailmentDetails> Issuer::BailmentInstructions(
    const Identifier& unitID,
    const bool onlyUnused) const
{
    Lock lock(lock_);
    std::vector<BailmentDetails> output{};
    const auto replies = get_requests(
        lock,
        proto::PEERREQUEST_BAILMENT,
        (onlyUnused) ? RequestStatus::Unused : RequestStatus::Replied);

    for (const auto& [requestID, replyID, isUsed] : replies) {
        std::time_t notUsed{0};
        const auto& notUsed2[[maybe_unused]] = isUsed;
        auto request = wallet_.PeerRequest(
            nym_id_, requestID, StorageBox::FINISHEDPEERREQUEST, notUsed);

        if (false == bool(request)) {
            request = wallet_.PeerRequest(
                nym_id_, requestID, StorageBox::SENTPEERREQUEST, notUsed);
        }

        OT_ASSERT(request);

        if (request->bailment().unitid() != unitID.str()) { continue; }

        auto reply =
            wallet_.PeerReply(nym_id_, replyID, StorageBox::PROCESSEDPEERREPLY);

        if (false == bool(reply)) {
            reply = wallet_.PeerReply(
                nym_id_, replyID, StorageBox::INCOMINGPEERREPLY);
        }

        OT_ASSERT(reply);

        output.emplace_back(requestID, reply->bailment());
    }

    return output;
}

std::vector<Issuer::ConnectionDetails> Issuer::ConnectionInfo(
    const proto::ConnectionInfoType type) const
{
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Searching for type ")(
        static_cast<std::uint32_t>(type))(
        " connection info requests (which have replies).")
        .Flush();
    Lock lock(lock_);
    std::vector<ConnectionDetails> output{};
    const auto replies = get_requests(
        lock, proto::PEERREQUEST_CONNECTIONINFO, RequestStatus::Replied);
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Have ")(replies.size())(
        " total requests.")
        .Flush();

    for (const auto& [requestID, replyID, isUsed] : replies) {
        std::time_t notUsed{0};
        const auto& notUsed2[[maybe_unused]] = isUsed;
        auto request = wallet_.PeerRequest(
            nym_id_, requestID, StorageBox::FINISHEDPEERREQUEST, notUsed);

        if (false == bool(request)) {
            request = wallet_.PeerRequest(
                nym_id_, requestID, StorageBox::SENTPEERREQUEST, notUsed);
        }

        OT_ASSERT(request);

        if (type != request->connectioninfo().type()) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Request ")(requestID)(
                " is wrong type (")(request->connectioninfo().type())(")")
                .Flush();

            continue;
        }

        auto reply =
            wallet_.PeerReply(nym_id_, replyID, StorageBox::PROCESSEDPEERREPLY);

        if (false == bool(reply)) {
            reply = wallet_.PeerReply(
                nym_id_, replyID, StorageBox::INCOMINGPEERREPLY);
        }

        OT_ASSERT(reply);

        output.emplace_back(requestID, reply->connectioninfo());
    }

    return output;
}

bool Issuer::ConnectionInfoInitiated(const proto::ConnectionInfoType type) const
{
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Searching for all type ")(
        static_cast<std::uint32_t>(type))(" connection info requests.")
        .Flush();
    Lock lock(lock_);
    std::size_t count{0};
    const auto requests = get_requests(
        lock, proto::PEERREQUEST_CONNECTIONINFO, RequestStatus::All);
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Have ")(requests.size())(
        " total requests.")
        .Flush();

    for (const auto& [requestID, a, b] : requests) {
        const auto& replyID[[maybe_unused]] = a;
        const auto& isUsed[[maybe_unused]] = b;
        std::time_t notUsed{0};
        auto request = wallet_.PeerRequest(
            nym_id_, requestID, StorageBox::SENTPEERREQUEST, notUsed);

        if (false == bool(request)) {
            request = wallet_.PeerRequest(
                nym_id_, requestID, StorageBox::FINISHEDPEERREQUEST, notUsed);
        }

        OT_ASSERT(request);

        if (type == request->connectioninfo().type()) {
            ++count;
        } else {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Request ")(requestID)(
                " is wrong type (")(request->connectioninfo().type())(")")
                .Flush();
        }
    }

    return 0 != count;
}

std::pair<bool, Issuer::Workflow::iterator> Issuer::find_request(
    const Lock& lock,
    const proto::PeerRequestType type,
    const Identifier& requestID)
{
    OT_ASSERT(verify_lock(lock))

    auto& work = peer_requests_[type];
    auto it = work.find(requestID);

    return {work.end() != it, it};
}

std::set<std::tuple<OTIdentifier, OTIdentifier, bool>> Issuer::GetRequests(
    const proto::PeerRequestType type,
    const Issuer::RequestStatus state) const
{
    Lock lock(lock_);

    return get_requests(lock, type, state);
}

std::set<std::tuple<OTIdentifier, OTIdentifier, bool>> Issuer::get_requests(
    const Lock& lock,
    const proto::PeerRequestType type,
    const Issuer::RequestStatus state) const
{
    OT_ASSERT(verify_lock(lock));

    std::set<std::tuple<OTIdentifier, OTIdentifier, bool>> output;

    if (Issuer::RequestStatus::None == state) { return output; }

    const auto map = peer_requests_.find(type);

    if (peer_requests_.end() == map) { return output; }

    for (const auto& [requestID, data] : map->second) {
        const auto& [replyID, used] = data;

        switch (state) {
            case Issuer::RequestStatus::Unused: {
                const bool exists = (false == replyID->empty());
                const bool unused = (false == used);

                if (exists && unused) {
                    output.emplace(requestID, replyID, used);
                }
            } break;
            case Issuer::RequestStatus::Replied: {
                if (false == replyID->empty()) {
                    output.emplace(requestID, replyID, used);
                }
            } break;
            case Issuer::RequestStatus::Requested: {
                if (replyID->empty()) {
                    output.emplace(requestID, Identifier::Factory(), false);
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

const Identifier& Issuer::IssuerID() const { return issuer_id_; }

const Identifier& Issuer::LocalNymID() const { return nym_id_; }

bool Issuer::Paired() const { return paired_.get(); }

const std::string& Issuer::PairingCode() const { return pairing_code_; }

OTIdentifier Issuer::PrimaryServer() const
{
    Lock lock(lock_);

    auto nym = wallet_.Nym(issuer_id_);

    if (false == bool(nym)) { return Identifier::Factory(); }

    return nym->Claims().PreferredOTServer();
}

bool Issuer::RemoveAccount(
    const proto::ContactItemType type,
    const Identifier& unitID,
    const Identifier& accountID)
{
    Lock lock(lock_);
    auto accountSet = account_map_.find(type);

    if (account_map_.end() == accountSet) { return false; }
    const auto unitId = Identifier::Factory(unitID);
    const auto accountId = Identifier::Factory(accountID);
    auto& accounts = accountSet->second;
    auto it = accounts.find({unitId, accountId});

    if (accounts.end() == it) { return false; }

    accounts.erase(it);

    return true;
}

std::set<proto::PeerRequestType> Issuer::RequestTypes() const
{
    Lock lock(lock_);
    std::set<proto::PeerRequestType> output{};

    for (const auto& [type, map] : peer_requests_) {
        const auto& notUsed[[maybe_unused]] = map;
        output.emplace(type);
    }

    return output;
}

proto::Issuer Issuer::Serialize() const
{
    Lock lock(lock_);
    proto::Issuer output;
    output.set_version(version_);
    output.set_id(issuer_id_->str());
    output.set_paired(paired_.get());
    output.set_pairingcode(pairing_code_);

    for (const auto& [type, accountSet] : account_map_) {
        for (const auto& [unitID, accountID] : accountSet) {
            auto& map = *output.add_accounts();
            map.set_version(version_);
            map.set_type(type);
            map.set_unitdefinitionid(unitID->str());
            map.set_accountid(accountID->str());
        }
    }

    for (const auto& [type, work] : peer_requests_) {
        auto& history = *output.add_peerrequests();
        history.set_version(version_);
        history.set_type(type);

        for (const auto& [request, data] : work) {
            const auto& [reply, isUsed] = data;
            auto& workflow = *history.add_workflow();
            workflow.set_version(version_);
            workflow.set_requestid(request->str());
            workflow.set_replyid(reply->str());
            workflow.set_used(isUsed);
        }
    }

    OT_ASSERT(proto::Validate(output, VERBOSE))

    return output;
}

void Issuer::SetPaired(const bool paired) { paired_->Set(paired); }

void Issuer::SetPairingCode(const std::string& code)
{
    Lock lock(lock_);
    pairing_code_ = code;
    paired_->On();
}

bool Issuer::SetUsed(
    const proto::PeerRequestType type,
    const Identifier& requestID,
    const bool isUsed)
{
    Lock lock(lock_);
    auto [found, it] = find_request(lock, type, requestID);
    auto& [reply, used] = it->second;
    const auto& notUsed[[maybe_unused]] = reply;

    if (false == found) { return false; }

    used = isUsed;

    return true;
}

bool Issuer::StoreSecretComplete() const
{
    Lock lock(lock_);
    const auto storeSecret = get_requests(
        lock, proto::PEERREQUEST_STORESECRET, RequestStatus::Replied);

    return 0 != storeSecret.size();
}

bool Issuer::StoreSecretInitiated() const
{
    Lock lock(lock_);
    const auto storeSecret =
        get_requests(lock, proto::PEERREQUEST_STORESECRET, RequestStatus::All);

    return 0 != storeSecret.size();
}

Issuer::~Issuer() {}
}  // namespace opentxs::api::client::implementation

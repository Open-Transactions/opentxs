// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <functional>
#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "internal/otx/client/Issuer.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/peer/PeerRequestType.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/Issuer.pb.h"

namespace opentxs
{
namespace api
{
namespace session
{
class Wallet;
}  // namespace session

class Session;
}  // namespace api

namespace proto
{
class Issuer;
}  // namespace proto

class Factory;
}  // namespace opentxs

namespace opentxs::otx::client::implementation
{
class Issuer final : virtual public otx::client::Issuer, Lockable
{
public:
    auto toString() const -> std::string final;

    auto AccountList(
        const core::UnitType type,
        const identifier::UnitDefinition& unitID) const
        -> std::pmr::set<OTIdentifier> final;
    auto BailmentInitiated(const identifier::UnitDefinition& unitID) const
        -> bool final;
    auto BailmentInstructions(
        const api::Session& client,
        const identifier::UnitDefinition& unitID,
        const bool onlyUnused = true) const
        -> std::pmr::vector<BailmentDetails> final;
    auto ConnectionInfo(
        const api::Session& client,
        const contract::peer::ConnectionInfoType type) const
        -> std::pmr::vector<ConnectionDetails> final;
    auto ConnectionInfoInitiated(
        const contract::peer::ConnectionInfoType type) const -> bool final;
    auto GetRequests(
        const contract::peer::PeerRequestType type,
        const RequestStatus state = RequestStatus::All) const
        -> std::pmr::set<std::tuple<OTIdentifier, OTIdentifier, bool>> final;
    auto IssuerID() const -> const identifier::Nym& final { return issuer_id_; }
    auto LocalNymID() const -> const identifier::Nym& final { return nym_id_; }
    auto Paired() const -> bool final;
    auto PairingCode() const -> const std::string& final;
    auto PrimaryServer() const -> OTServerID final;
    auto RequestTypes() const
        -> std::pmr::set<contract::peer::PeerRequestType> final;
    auto Serialize(proto::Issuer&) const -> bool final;
    auto StoreSecretComplete() const -> bool final;
    auto StoreSecretInitiated() const -> bool final;

    void AddAccount(
        const core::UnitType type,
        const identifier::UnitDefinition& unitID,
        const Identifier& accountID) final;
    auto AddReply(
        const contract::peer::PeerRequestType type,
        const Identifier& requestID,
        const Identifier& replyID) -> bool final;
    auto AddRequest(
        const contract::peer::PeerRequestType type,
        const Identifier& requestID) -> bool final;
    auto RemoveAccount(
        const core::UnitType type,
        const identifier::UnitDefinition& unitID,
        const Identifier& accountID) -> bool final;
    void SetPaired(const bool paired) final;
    void SetPairingCode(const std::string& code) final;
    auto SetUsed(
        const contract::peer::PeerRequestType type,
        const Identifier& requestID,
        const bool isUsed = true) -> bool final;

    Issuer(
        const api::session::Wallet& wallet,
        const identifier::Nym& nymID,
        const proto::Issuer& serialized);
    Issuer(
        const api::session::Wallet& wallet,
        const identifier::Nym& nymID,
        const identifier::Nym& issuerID);

    ~Issuer() final;

private:
    using Workflow = std::pmr::map<OTIdentifier, std::pair<OTIdentifier, bool>>;
    using WorkflowMap =
        std::pmr::map<contract::peer::PeerRequestType, Workflow>;
    using UnitAccountPair = std::pair<OTUnitID, OTIdentifier>;

    const api::session::Wallet& wallet_;
    VersionNumber version_{0};
    std::string pairing_code_{""};
    mutable OTFlag paired_;
    const OTNymID nym_id_;
    const OTNymID issuer_id_;
    std::pmr::map<core::UnitType, std::pmr::set<UnitAccountPair>> account_map_;
    WorkflowMap peer_requests_;

    auto find_request(
        const Lock& lock,
        const contract::peer::PeerRequestType type,
        const Identifier& requestID) -> std::pair<bool, Workflow::iterator>;
    auto get_requests(
        const Lock& lock,
        const contract::peer::PeerRequestType type,
        const RequestStatus state = RequestStatus::All) const
        -> std::pmr::set<std::tuple<OTIdentifier, OTIdentifier, bool>>;

    auto add_request(
        const Lock& lock,
        const contract::peer::PeerRequestType type,
        const Identifier& requestID,
        const Identifier& replyID) -> bool;

    Issuer() = delete;
    Issuer(const Issuer&) = delete;
    Issuer(Issuer&&) = delete;
    auto operator=(const Issuer&) -> Issuer& = delete;
    auto operator=(Issuer&&) -> Issuer& = delete;
};
}  // namespace opentxs::otx::client::implementation

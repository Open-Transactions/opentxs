// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <functional>
#include <iosfwd>
#include <tuple>
#include <utility>

#include "internal/otx/client/Issuer.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/Lockable.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/peer/PeerRequestType.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/Issuer.pb.h"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::client::implementation
{
class Issuer final : virtual public otx::client::Issuer, Lockable
{
public:
    auto toString() const -> UnallocatedCString final;

    auto AccountList(
        const UnitType type,
        const identifier::UnitDefinition& unitID) const
        -> UnallocatedSet<OTIdentifier> final;
    auto BailmentInitiated(const identifier::UnitDefinition& unitID) const
        -> bool final;
    auto BailmentInstructions(
        const api::Session& client,
        const identifier::UnitDefinition& unitID,
        const bool onlyUnused = true) const
        -> UnallocatedVector<BailmentDetails> final;
    auto ConnectionInfo(
        const api::Session& client,
        const contract::peer::ConnectionInfoType type) const
        -> UnallocatedVector<ConnectionDetails> final;
    auto ConnectionInfoInitiated(
        const contract::peer::ConnectionInfoType type) const -> bool final;
    auto GetRequests(
        const contract::peer::PeerRequestType type,
        const RequestStatus state = RequestStatus::All) const
        -> UnallocatedSet<std::tuple<OTIdentifier, OTIdentifier, bool>> final;
    auto IssuerID() const -> const identifier::Nym& final { return issuer_id_; }
    auto LocalNymID() const -> const identifier::Nym& final { return nym_id_; }
    auto Paired() const -> bool final;
    auto PairingCode() const -> const UnallocatedCString& final;
    auto PrimaryServer() const -> OTNotaryID final;
    auto RequestTypes() const
        -> UnallocatedSet<contract::peer::PeerRequestType> final;
    auto Serialize(proto::Issuer&) const -> bool final;
    auto StoreSecretComplete() const -> bool final;
    auto StoreSecretInitiated() const -> bool final;

    void AddAccount(
        const UnitType type,
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
        const UnitType type,
        const identifier::UnitDefinition& unitID,
        const Identifier& accountID) -> bool final;
    void SetPaired(const bool paired) final;
    void SetPairingCode(const UnallocatedCString& code) final;
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
    using Workflow =
        UnallocatedMap<OTIdentifier, std::pair<OTIdentifier, bool>>;
    using WorkflowMap =
        UnallocatedMap<contract::peer::PeerRequestType, Workflow>;
    using UnitAccountPair = std::pair<OTUnitID, OTIdentifier>;

    static constexpr auto current_version_ = VersionNumber{1};

    const api::session::Wallet& wallet_;
    VersionNumber version_{0};
    UnallocatedCString pairing_code_{""};
    mutable OTFlag paired_;
    const OTNymID nym_id_;
    const OTNymID issuer_id_;
    UnallocatedMap<UnitType, UnallocatedSet<UnitAccountPair>> account_map_;
    WorkflowMap peer_requests_;

    auto find_request(
        const Lock& lock,
        const contract::peer::PeerRequestType type,
        const Identifier& requestID) -> std::pair<bool, Workflow::iterator>;
    auto get_requests(
        const Lock& lock,
        const contract::peer::PeerRequestType type,
        const RequestStatus state = RequestStatus::All) const
        -> UnallocatedSet<std::tuple<OTIdentifier, OTIdentifier, bool>>;

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

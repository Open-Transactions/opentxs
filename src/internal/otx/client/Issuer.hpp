// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <tuple>

#include "opentxs/Version.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/peer/BailmentReply.hpp"
#include "opentxs/core/contract/peer/ConnectionReply.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace proto
{
class Issuer;
}  // namespace proto

namespace identifier
{
class Nym;
class UnitDefinition;
}  // namespace identifier
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::client
{
class Issuer
{
public:
    using BailmentDetails = std::pair<OTIdentifier, OTBailmentReply>;
    using ConnectionDetails = std::pair<OTIdentifier, OTConnectionReply>;

    enum class RequestStatus : std::int32_t {
        All = -1,
        None = 0,
        Requested = 1,
        Replied = 2,
        Unused = 3,
    };

    virtual auto toString() const -> UnallocatedCString = 0;

    virtual auto AccountList(
        const UnitType type,
        const identifier::UnitDefinition& unitID) const
        -> UnallocatedSet<OTIdentifier> = 0;
    virtual auto BailmentInitiated(
        const identifier::UnitDefinition& unitID) const -> bool = 0;
    virtual auto BailmentInstructions(
        const api::Session& client,
        const identifier::UnitDefinition& unitID,
        const bool onlyUnused = true) const
        -> UnallocatedVector<BailmentDetails> = 0;
    virtual auto ConnectionInfo(
        const api::Session& client,
        const contract::peer::ConnectionInfoType type) const
        -> UnallocatedVector<ConnectionDetails> = 0;
    virtual auto ConnectionInfoInitiated(
        const contract::peer::ConnectionInfoType type) const -> bool = 0;
    virtual auto GetRequests(
        const contract::peer::PeerRequestType type,
        const RequestStatus state = RequestStatus::All) const
        -> UnallocatedSet<std::tuple<OTIdentifier, OTIdentifier, bool>> = 0;
    virtual auto IssuerID() const -> const identifier::Nym& = 0;
    virtual auto LocalNymID() const -> const identifier::Nym& = 0;
    virtual auto Paired() const -> bool = 0;
    virtual auto PairingCode() const -> const UnallocatedCString& = 0;
    virtual auto PrimaryServer() const -> OTNotaryID = 0;
    virtual auto RequestTypes() const
        -> UnallocatedSet<contract::peer::PeerRequestType> = 0;
    virtual auto Serialize(proto::Issuer&) const -> bool = 0;
    virtual auto StoreSecretComplete() const -> bool = 0;
    virtual auto StoreSecretInitiated() const -> bool = 0;

    virtual void AddAccount(
        const UnitType type,
        const identifier::UnitDefinition& unitID,
        const Identifier& accountID) = 0;
    virtual auto AddReply(
        const contract::peer::PeerRequestType type,
        const Identifier& requestID,
        const Identifier& replyID) -> bool = 0;
    virtual auto AddRequest(
        const contract::peer::PeerRequestType type,
        const Identifier& requestID) -> bool = 0;
    virtual auto RemoveAccount(
        const UnitType type,
        const identifier::UnitDefinition& unitID,
        const Identifier& accountID) -> bool = 0;
    virtual void SetPaired(const bool paired) = 0;
    virtual void SetPairingCode(const UnallocatedCString& code) = 0;
    virtual auto SetUsed(
        const contract::peer::PeerRequestType type,
        const Identifier& requestID,
        const bool isUsed = true) -> bool = 0;

    virtual ~Issuer() = default;

protected:
    Issuer() = default;

private:
    Issuer(const Issuer&) = delete;
    Issuer(Issuer&&) = delete;
    auto operator=(const Issuer&) -> Issuer& = delete;
    auto operator=(Issuer&&) -> Issuer& = delete;
};
}  // namespace opentxs::otx::client

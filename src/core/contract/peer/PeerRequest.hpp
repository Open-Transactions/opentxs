// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/contract/Signable.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/contract/peer/BailmentNotice.hpp"
#include "opentxs/core/contract/peer/BailmentRequest.hpp"
#include "opentxs/core/contract/peer/ConnectionRequest.hpp"
#include "opentxs/core/contract/peer/OutBailmentRequest.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/contract/peer/PeerRequestType.hpp"
#include "opentxs/core/contract/peer/StoreSecret.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace contract
{
namespace peer
{
namespace request
{
class Bailment;
class BailmentNotice;
class Connection;
class Outbailment;
class StoreSecret;
}  // namespace request
}  // namespace peer
}  // namespace contract

namespace identifier
{
class Notary;
}  // namespace identifier

namespace proto
{
class Signature;
}  // namespace proto

class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::implementation
{
class Request : virtual public peer::Request,
                public opentxs::contract::implementation::Signable
{
public:
    static auto Finish(Request& contract, const PasswordPrompt& reason) -> bool;

    auto asBailment() const noexcept -> const request::Bailment& override;
    auto asBailmentNotice() const noexcept
        -> const request::BailmentNotice& override;
    auto asConnection() const noexcept -> const request::Connection& override;
    auto asOutbailment() const noexcept -> const request::Outbailment& override;
    auto asStoreSecret() const noexcept -> const request::StoreSecret& override;

    auto Alias() const noexcept -> UnallocatedCString final { return Name(); }
    auto Initiator() const -> const identifier::Nym& final
    {
        return initiator_;
    }
    auto Name() const noexcept -> UnallocatedCString final
    {
        return id_->str();
    }
    auto Recipient() const -> const identifier::Nym& final
    {
        return recipient_;
    }
    auto Serialize() const noexcept -> OTData final;
    auto Serialize(SerializedType&) const -> bool final;
    auto Server() const -> const identifier::Notary& final { return server_; }
    auto Type() const -> PeerRequestType final { return type_; }
    auto SetAlias(const UnallocatedCString&) noexcept -> bool final
    {
        return false;
    }

    ~Request() override = default;

protected:
    virtual auto IDVersion(const Lock& lock) const -> SerializedType;
    auto validate(const Lock& lock) const -> bool final;
    auto verify_signature(const Lock& lock, const proto::Signature& signature)
        const -> bool final;

    Request(
        const api::Session& api,
        const Nym_p& nym,
        VersionNumber version,
        const identifier::Nym& recipient,
        const identifier::Notary& serverID,
        const PeerRequestType& type,
        const UnallocatedCString& conditions = {});
    Request(
        const api::Session& api,
        const Nym_p& nym,
        const SerializedType& serialized,
        const UnallocatedCString& conditions = {});
    Request(const Request&) noexcept;

private:
    using ot_super = Signable;

    const OTNymID initiator_;
    const OTNymID recipient_;
    const OTNotaryID server_;
    const OTIdentifier cookie_;
    const PeerRequestType type_;

    static auto GetID(const api::Session& api, const SerializedType& contract)
        -> OTIdentifier;
    static auto FinalizeContract(
        Request& contract,
        const PasswordPrompt& reason) -> bool;

    auto contract(const Lock& lock) const -> SerializedType;
    auto GetID(const Lock& lock) const -> OTIdentifier final;
    auto SigVersion(const Lock& lock) const -> SerializedType;

    auto update_signature(const Lock& lock, const PasswordPrompt& reason)
        -> bool final;

    Request() = delete;
};
}  // namespace opentxs::contract::peer::implementation

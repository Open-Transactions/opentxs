// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "core/contract/Signable.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/contract/peer/BailmentReply.hpp"
#include "opentxs/core/contract/peer/ConnectionReply.hpp"
#include "opentxs/core/contract/peer/NoticeAcknowledgement.hpp"
#include "opentxs/core/contract/peer/OutBailmentReply.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequestType.hpp"
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
namespace reply
{
class Acknowledgement;
class Bailment;
class Connection;
class Outbailment;
}  // namespace reply
}  // namespace peer
}  // namespace contract

namespace proto
{
class PeerRequest;
class Signature;
}  // namespace proto

class Factory;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::implementation
{
class Reply : virtual public peer::Reply,
              public opentxs::contract::implementation::Signable
{
public:
    static auto Finish(Reply& contract, const PasswordPrompt& reason) -> bool;
    static auto LoadRequest(
        const api::Session& api,
        const Nym_p& nym,
        const Identifier& requestID,
        proto::PeerRequest& request) -> bool;

    auto asAcknowledgement() const noexcept
        -> const reply::Acknowledgement& override;
    auto asBailment() const noexcept -> const reply::Bailment& override;
    auto asConnection() const noexcept -> const reply::Connection& override;
    auto asOutbailment() const noexcept -> const reply::Outbailment& override;

    auto Alias() const noexcept -> UnallocatedCString final { return Name(); }
    auto Name() const noexcept -> UnallocatedCString final
    {
        return id_->str();
    }
    auto Serialize() const noexcept -> OTData final;
    auto Serialize(SerializedType&) const -> bool override;
    auto Server() const -> const identifier::Notary& final { return server_; }
    auto SetAlias(const UnallocatedCString&) noexcept -> bool final
    {
        return false;
    }
    auto Type() const -> PeerRequestType final { return type_; }

    ~Reply() override = default;

protected:
    virtual auto IDVersion(const Lock& lock) const -> SerializedType;
    auto validate(const Lock& lock) const -> bool final;
    auto verify_signature(const Lock& lock, const proto::Signature& signature)
        const -> bool final;

    Reply(
        const api::Session& api,
        const Nym_p& nym,
        const VersionNumber version,
        const identifier::Nym& initiator,
        const identifier::Notary& server,
        const PeerRequestType& type,
        const Identifier& request,
        const UnallocatedCString& conditions = {});
    Reply(
        const api::Session& api,
        const Nym_p& nym,
        const SerializedType& serialized,
        const UnallocatedCString& conditions = {});
    Reply(const Reply&) noexcept;

private:
    const OTNymID initiator_;
    const OTNymID recipient_;
    const OTNotaryID server_;
    const OTIdentifier cookie_;
    const PeerRequestType type_;

    static auto GetID(const api::Session& api, const SerializedType& contract)
        -> OTIdentifier;
    static auto FinalizeContract(Reply& contract, const PasswordPrompt& reason)
        -> bool;

    auto contract(const Lock& lock) const -> SerializedType;
    auto GetID(const Lock& lock) const -> OTIdentifier final;
    auto SigVersion(const Lock& lock) const -> SerializedType;

    auto update_signature(const Lock& lock, const PasswordPrompt& reason)
        -> bool final;

    Reply() = delete;
    Reply(Reply&&) = delete;
    auto operator=(const Reply&) -> Reply& = delete;
    auto operator=(Reply&&) -> Reply& = delete;
};
}  // namespace opentxs::contract::peer::implementation

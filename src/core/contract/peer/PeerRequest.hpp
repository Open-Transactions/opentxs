// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/contract/Signable.hpp"
#include "internal/core/contract/peer/BailmentNotice.hpp"
#include "internal/core/contract/peer/BailmentRequest.hpp"
#include "internal/core/contract/peer/ConnectionRequest.hpp"
#include "internal/core/contract/peer/OutBailmentRequest.hpp"
#include "internal/core/contract/peer/PeerRequest.hpp"
#include "internal/core/contract/peer/StoreSecret.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/contract/peer/PeerRequestType.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
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

class ByteArray;
class PasswordPrompt;
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
        return id_.asBase58(api_.Crypto());
    }
    auto Recipient() const -> const identifier::Nym& final
    {
        return recipient_;
    }
    auto Serialize() const noexcept -> ByteArray final;
    auto Serialize(SerializedType&) const -> bool final;
    auto Server() const -> const identifier::Notary& final { return server_; }
    auto Type() const -> PeerRequestType final { return type_; }
    auto SetAlias(const UnallocatedCString&) noexcept -> bool final
    {
        return false;
    }

    Request() = delete;

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

    const identifier::Nym initiator_;
    const identifier::Nym recipient_;
    const identifier::Notary server_;
    const identifier::Generic cookie_;
    const PeerRequestType type_;

    static auto GetID(const api::Session& api, const SerializedType& contract)
        -> identifier::Generic;
    static auto FinalizeContract(
        Request& contract,
        const PasswordPrompt& reason) -> bool;

    auto contract(const Lock& lock) const -> SerializedType;
    auto GetID(const Lock& lock) const -> identifier::Generic final;
    auto SigVersion(const Lock& lock) const -> SerializedType;

    auto update_signature(const Lock& lock, const PasswordPrompt& reason)
        -> bool final;
};
}  // namespace opentxs::contract::peer::implementation

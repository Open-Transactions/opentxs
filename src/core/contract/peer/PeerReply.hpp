// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/contract/Signable.hpp"
#include "internal/core/contract/peer/BailmentReply.hpp"
#include "internal/core/contract/peer/ConnectionReply.hpp"
#include "internal/core/contract/peer/NoticeAcknowledgement.hpp"
#include "internal/core/contract/peer/OutBailmentReply.hpp"
#include "internal/core/contract/peer/PeerReply.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
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

namespace proto
{
class PeerRequest;
class Signature;
}  // namespace proto

class ByteArray;
class PasswordPrompt;
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
        const identifier::Generic& requestID,
        proto::PeerRequest& request) -> bool;

    auto asAcknowledgement() const noexcept
        -> const reply::Acknowledgement& override;
    auto asBailment() const noexcept -> const reply::Bailment& override;
    auto asConnection() const noexcept -> const reply::Connection& override;
    auto asOutbailment() const noexcept -> const reply::Outbailment& override;

    auto Alias() const noexcept -> UnallocatedCString final { return Name(); }
    auto Name() const noexcept -> UnallocatedCString final
    {
        return id_.asBase58(api_.Crypto());
    }
    auto Serialize() const noexcept -> ByteArray final;
    auto Serialize(SerializedType&) const -> bool override;
    auto Server() const -> const identifier::Notary& final { return server_; }
    auto SetAlias(const UnallocatedCString&) noexcept -> bool final
    {
        return false;
    }
    auto Type() const -> PeerRequestType final { return type_; }

    Reply() = delete;
    Reply(const Reply&) noexcept;
    Reply(Reply&&) = delete;
    auto operator=(const Reply&) -> Reply& = delete;
    auto operator=(Reply&&) -> Reply& = delete;

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
        const identifier::Generic& request,
        const UnallocatedCString& conditions = {});
    Reply(
        const api::Session& api,
        const Nym_p& nym,
        const SerializedType& serialized,
        const UnallocatedCString& conditions = {});

private:
    const identifier::Nym initiator_;
    const identifier::Nym recipient_;
    const identifier::Notary server_;
    const identifier::Generic cookie_;
    const PeerRequestType type_;

    static auto GetID(const api::Session& api, const SerializedType& contract)
        -> identifier::Generic;
    static auto FinalizeContract(Reply& contract, const PasswordPrompt& reason)
        -> bool;

    auto contract(const Lock& lock) const -> SerializedType;
    auto GetID(const Lock& lock) const -> identifier::Generic final;
    auto SigVersion(const Lock& lock) const -> SerializedType;

    auto update_signature(const Lock& lock, const PasswordPrompt& reason)
        -> bool final;
};
}  // namespace opentxs::contract::peer::implementation

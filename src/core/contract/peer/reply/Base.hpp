// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "core/contract/Signable.hpp"
#include "internal/core/contract/peer/reply/Acknowledgement.hpp"
#include "internal/core/contract/peer/reply/Bailment.hpp"
#include "internal/core/contract/peer/reply/Base.hpp"
#include "internal/core/contract/peer/reply/Connection.hpp"
#include "internal/core/contract/peer/reply/Faucet.hpp"
#include "internal/core/contract/peer/reply/Outbailment.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace proto
{
class PeerRequest;
class Signature;
}  // namespace proto

class PasswordPrompt;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::reply::implementation
{
class Reply
    : virtual public internal::Reply,
      public opentxs::contract::implementation::Signable<identifier::Generic>
{
public:
    static auto Finish(Reply& contract, const PasswordPrompt& reason) -> bool;
    static auto LoadRequest(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Generic& requestID,
        proto::PeerRequest& request) -> bool;

    auto Alias() const noexcept -> UnallocatedCString final;
    auto Alias(alloc::Strategy alloc) const noexcept -> CString final;
    auto asAcknowledgement() const noexcept
        -> const internal::Acknowledgement& override;
    auto asBailment() const noexcept -> const internal::Bailment& override;
    auto asConnection() const noexcept -> const internal::Connection& override;
    auto asFaucet() const noexcept -> const internal::Faucet& override;
    auto asOutbailment() const noexcept
        -> const internal::Outbailment& override;
    auto Initiator() const -> const identifier::Nym& final
    {
        return initiator_;
    }
    auto Recipient() const -> const identifier::Nym& final
    {
        return recipient_;
    }
    auto Serialize(Writer&& out) const noexcept -> bool final;
    auto Serialize(SerializedType&) const -> bool override;
    auto Server() const -> const identifier::Notary& final { return server_; }
    auto SetAlias(std::string_view) noexcept -> bool final { return false; }
    auto Type() const -> RequestType final { return type_; }

    Reply() = delete;
    Reply(const Reply&) noexcept;
    Reply(Reply&&) = delete;
    auto operator=(const Reply&) -> Reply& = delete;
    auto operator=(Reply&&) -> Reply& = delete;

    ~Reply() override = default;

protected:
    virtual auto IDVersion() const -> SerializedType;
    auto validate() const -> bool final;
    auto verify_signature(const proto::Signature& signature) const
        -> bool final;

    Reply(
        const api::Session& api,
        const Nym_p& nym,
        const VersionNumber version,
        const identifier::Nym& initiator,
        const identifier::Notary& server,
        const RequestType& type,
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
    const RequestType type_;

    static auto GetID(const api::Session& api, const SerializedType& contract)
        -> identifier_type;
    static auto FinalizeContract(Reply& contract, const PasswordPrompt& reason)
        -> bool;

    auto calculate_id() const -> identifier_type final;
    auto contract() const -> SerializedType;
    auto sig_version() const -> SerializedType;

    auto update_signature(const PasswordPrompt& reason) -> bool final;
};
}  // namespace opentxs::contract::peer::reply::implementation

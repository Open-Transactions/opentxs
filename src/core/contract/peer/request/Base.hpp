// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "core/contract/Signable.hpp"
#include "internal/core/contract/peer/request/Bailment.hpp"
#include "internal/core/contract/peer/request/BailmentNotice.hpp"
#include "internal/core/contract/peer/request/Base.hpp"
#include "internal/core/contract/peer/request/Connection.hpp"
#include "internal/core/contract/peer/request/Faucet.hpp"
#include "internal/core/contract/peer/request/Outbailment.hpp"
#include "internal/core/contract/peer/request/StoreSecret.hpp"
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
class Signature;
}  // namespace proto

class PasswordPrompt;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request::implementation
{
class Request
    : virtual public internal::Request,
      public opentxs::contract::implementation::Signable<identifier::Generic>
{
public:
    static auto Finish(Request& contract, const PasswordPrompt& reason) -> bool;

    auto Alias() const noexcept -> UnallocatedCString final;
    auto Alias(alloc::Strategy alloc) const noexcept -> CString final;
    auto asBailment() const noexcept -> const internal::Bailment& override;
    auto asBailmentNotice() const noexcept
        -> const internal::BailmentNotice& override;
    auto asConnection() const noexcept -> const internal::Connection& override;
    auto asFaucet() const noexcept -> const internal::Faucet& override;
    auto asOutbailment() const noexcept
        -> const internal::Outbailment& override;
    auto asStoreSecret() const noexcept
        -> const internal::StoreSecret& override;
    auto Initiator() const -> const identifier::Nym& final
    {
        return initiator_;
    }
    auto Recipient() const -> const identifier::Nym& final
    {
        return recipient_;
    }
    auto Serialize(Writer&& out) const noexcept -> bool final;
    auto Serialize(SerializedType&) const -> bool final;
    auto Server() const -> const identifier::Notary& final { return server_; }
    auto Type() const -> RequestType final { return type_; }

    auto SetAlias(std::string_view) noexcept -> bool final { return false; }

    Request() = delete;

    ~Request() override = default;

protected:
    virtual auto IDVersion() const -> SerializedType;
    auto validate() const -> bool final;
    auto verify_signature(const proto::Signature& signature) const
        -> bool final;

    Request(
        const api::Session& api,
        const Nym_p& nym,
        VersionNumber version,
        const identifier::Nym& recipient,
        const identifier::Notary& serverID,
        const RequestType& type,
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
    const RequestType type_;

    static auto GetID(const api::Session& api, const SerializedType& contract)
        -> identifier_type;
    static auto FinalizeContract(
        Request& contract,
        const PasswordPrompt& reason) -> bool;

    auto calculate_id() const -> identifier_type final;
    auto contract() const -> SerializedType;
    auto SigVersion() const -> SerializedType;

    auto update_signature(const PasswordPrompt& reason) -> bool final;
};
}  // namespace opentxs::contract::peer::request::implementation

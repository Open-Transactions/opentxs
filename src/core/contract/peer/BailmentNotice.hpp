// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/contract/peer/PeerRequest.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/contract/peer/BailmentNotice.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
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

namespace identifier
{
class Nym;
}  // namespace identifier

class Factory;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request::implementation
{
class BailmentNotice final : public request::BailmentNotice,
                             public peer::implementation::Request
{
public:
    BailmentNotice(
        const api::Session& api,
        const Nym_p& nym,
        const SerializedType& serialized);
    BailmentNotice(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Notary& serverID,
        const Identifier& requestID,
        const UnallocatedCString& txid,
        const Amount& amount);

    ~BailmentNotice() final = default;

    auto asBailmentNotice() const noexcept
        -> const request::BailmentNotice& final
    {
        return *this;
    }

private:
    friend opentxs::Factory;

    static constexpr auto current_version_ = VersionNumber{6};

    const OTUnitID unit_;
    const OTNotaryID server_;
    const OTIdentifier requestID_;
    const UnallocatedCString txid_;
    const Amount amount_;

    auto clone() const noexcept -> BailmentNotice* final
    {
        return new BailmentNotice(*this);
    }
    auto IDVersion(const Lock& lock) const -> SerializedType final;

    BailmentNotice() = delete;
    BailmentNotice(const BailmentNotice&);
    BailmentNotice(BailmentNotice&&) = delete;
    auto operator=(const BailmentNotice&) -> BailmentNotice& = delete;
    auto operator=(BailmentNotice&&) -> BailmentNotice& = delete;
};
}  // namespace opentxs::contract::peer::request::implementation

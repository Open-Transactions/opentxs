// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "core/contract/peer/PeerRequest.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/contract/peer/OutBailmentRequest.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
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

namespace proto
{
class PeerRequest;
}  // namespace proto

class Factory;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request::implementation
{
class Outbailment final : public request::Outbailment,
                          public peer::implementation::Request
{
public:
    Outbailment(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Notary& serverID,
        const Amount& amount,
        const UnallocatedCString& terms);
    Outbailment(
        const api::Session& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized);

    ~Outbailment() final = default;

    auto asOutbailment() const noexcept -> const request::Outbailment& final
    {
        return *this;
    }

private:
    friend opentxs::Factory;

    static constexpr auto current_version_ = VersionNumber{4};

    const OTUnitID unit_;
    const OTNotaryID server_;
    const Amount amount_;

    auto clone() const noexcept -> Outbailment* final
    {
        return new Outbailment(*this);
    }
    auto IDVersion(const Lock& lock) const -> SerializedType final;

    Outbailment() = delete;
    Outbailment(const Outbailment&);
    Outbailment(Outbailment&&) = delete;
    auto operator=(const Outbailment&) -> Outbailment& = delete;
    auto operator=(Outbailment&&) -> Outbailment& = delete;
};
}  // namespace opentxs::contract::peer::request::implementation

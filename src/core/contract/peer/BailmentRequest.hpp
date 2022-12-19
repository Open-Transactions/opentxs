// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/contract/peer/PeerRequest.hpp"
#include "internal/core/contract/peer/BailmentRequest.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request::implementation
{
class Bailment final : public request::Bailment,
                       public peer::implementation::Request
{
public:
    auto asBailment() const noexcept -> const request::Bailment& final
    {
        return *this;
    }

    auto ServerID() const -> const identifier::Notary& final { return server_; }
    auto UnitID() const -> const identifier::UnitDefinition& final
    {
        return unit_;
    }

    Bailment(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Notary& serverID);
    Bailment(
        const api::Session& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized);
    Bailment() = delete;
    Bailment(const Bailment&);
    Bailment(Bailment&&) = delete;
    auto operator=(const Bailment&) -> Bailment& = delete;
    auto operator=(Bailment&&) -> Bailment& = delete;

    ~Bailment() final = default;

private:
    friend opentxs::Factory;

    static constexpr auto current_version_ = VersionNumber{4};

    const identifier::UnitDefinition unit_;
    const identifier::Notary server_;

    auto clone() const noexcept -> Bailment* final
    {
        return new Bailment(*this);
    }
    auto IDVersion(const Lock& lock) const -> SerializedType final;
};
}  // namespace opentxs::contract::peer::request::implementation

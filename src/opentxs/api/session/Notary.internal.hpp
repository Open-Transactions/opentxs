// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/session/Notary.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class UnitDefinition;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::internal
{
class Notary : virtual public api::internal::Session
{
public:
    virtual auto DropIncoming(const int count) const -> void = 0;
    virtual auto DropOutgoing(const int count) const -> void = 0;
    virtual auto GetAdminNym() const -> UnallocatedCString = 0;
    virtual auto GetAdminPassword() const -> UnallocatedCString = 0;
    virtual auto GetOwnerName() const -> std::string_view = 0;
    virtual auto GetPrivateMint(
        const identifier::UnitDefinition& unitid,
        std::uint32_t series) const noexcept -> otx::blind::Mint& = 0;
    virtual auto GetPublicMint(const identifier::UnitDefinition& unitID)
        const noexcept -> otx::blind::Mint& = 0;
    virtual auto GetUserTerms() const -> std::string_view = 0;
    virtual auto ID() const -> const identifier::Notary& = 0;
    virtual auto InprocEndpoint() const -> UnallocatedCString = 0;
    virtual auto NymID() const -> const identifier::Nym& = 0;
    virtual auto Server() const -> opentxs::server::Server& = 0;
    virtual auto SetMintKeySize(const std::size_t size) const -> void = 0;
    virtual auto UpdateMint(const identifier::UnitDefinition& unitID) const
        -> void = 0;

    virtual auto CheckMint(const identifier::UnitDefinition& unit) noexcept
        -> void = 0;
    virtual auto Init(std::shared_ptr<internal::Notary> me) -> void = 0;
    virtual auto Start(std::shared_ptr<internal::Notary> api) noexcept
        -> void = 0;

    ~Notary() override = default;
};
}  // namespace opentxs::api::session::internal

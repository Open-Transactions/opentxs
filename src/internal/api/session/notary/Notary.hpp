// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/api/session/Session.hpp"
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
class Notary : virtual public session::Notary, virtual public Session
{
public:
    virtual auto InprocEndpoint() const -> UnallocatedCString = 0;
    auto InternalNotary() const noexcept
        -> const session::internal::Notary& final
    {
        return *this;
    }

    virtual auto CheckMint(const identifier::UnitDefinition& unit) noexcept
        -> void = 0;
    auto InternalNotary() noexcept -> session::internal::Notary& final
    {
        return *this;
    }

    ~Notary() override = default;
};
}  // namespace opentxs::api::session::internal

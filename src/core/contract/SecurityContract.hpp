// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <UnitDefinition.pb.h>

#include "core/contract/Unit.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/contract/SecurityContract.hpp"
#include "opentxs/core/contract/UnitType.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace display
{
class Definition;
}  // namespace display

class Factory;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::unit::implementation
{
class Security final : public unit::Security,
                       public contract::implementation::Unit
{
public:
    auto Type() const -> contract::UnitType final
    {
        return contract::UnitType::Security;
    }

    Security(
        const api::Session& api,
        const Nym_p& nym,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const opentxs::UnitType unitOfAccount,
        const VersionNumber version,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement);
    Security(
        const api::Session& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized);
    Security(Security&&) = delete;
    auto operator=(const Security&) -> Security& = delete;
    auto operator=(Security&&) -> Security& = delete;

    ~Security() final = default;

private:
    friend opentxs::Factory;

    auto clone() const noexcept -> Security* final
    {
        return new Security(*this);
    }
    auto IDVersion(const Lock& lock) const -> proto::UnitDefinition final;

    Security(const Security&);
};
}  // namespace opentxs::contract::unit::implementation

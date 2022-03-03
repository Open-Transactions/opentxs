// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "Proto.hpp"
#include "core/contract/Unit.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/contract/CurrencyContract.hpp"
#include "opentxs/core/contract/UnitType.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/UnitDefinition.pb.h"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace display
{
class Definition;
}  // namespace display

class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::unit::implementation
{
class Currency final : public unit::Currency,
                       public contract::implementation::Unit
{
public:
    auto Type() const -> contract::UnitType final
    {
        return contract::UnitType::Currency;
    }

    Currency(
        const api::Session& api,
        const Nym_p& nym,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const opentxs::UnitType unitOfAccount,
        const VersionNumber version,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement);
    Currency(
        const api::Session& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized);

    ~Currency() final = default;

private:
    auto clone() const noexcept -> Currency* final
    {
        return new Currency(*this);
    }
    auto IDVersion(const Lock& lock) const -> proto::UnitDefinition final;

    Currency(const Currency&);
    Currency(Currency&&) = delete;
    auto operator=(const Currency&) -> Currency& = delete;
    auto operator=(Currency&&) -> Currency& = delete;
};
}  // namespace opentxs::contract::unit::implementation

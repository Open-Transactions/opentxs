// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/protobuf/UnitDefinition.pb.h>

#include "core/contract/Unit.hpp"
#include "internal/core/contract/CurrencyContract.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/contract/Types.hpp"
#include "opentxs/contract/UnitDefinitionType.hpp"  // IWYU pragma: keep
#include "opentxs/core/Amount.hpp"
#include "opentxs/identity/Types.hpp"
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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::unit::implementation
{
class Currency final : public unit::Currency,
                       public contract::implementation::Unit
{
public:
    auto Type() const -> contract::UnitDefinitionType final
    {
        return contract::UnitDefinitionType::Currency;
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
        const protobuf::UnitDefinition serialized);
    Currency(Currency&&) = delete;
    auto operator=(const Currency&) -> Currency& = delete;
    auto operator=(Currency&&) -> Currency& = delete;

    ~Currency() final = default;

private:
    auto IDVersion() const -> protobuf::UnitDefinition final;

    Currency(const Currency&);
};
}  // namespace opentxs::contract::unit::implementation

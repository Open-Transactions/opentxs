// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/CurrencyContract.hpp"  // IWYU pragma: associated

#include <Signature.pb.h>
#include <UnitDefinition.pb.h>
#include <memory>
#include <string_view>

#include "2_Factory.hpp"
#include "core/contract/Signable.hpp"
#include "core/contract/Unit.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/UnitDefinition.hpp"

namespace opentxs
{
auto Factory::CurrencyContract(
    const api::Session& api,
    const Nym_p& nym,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const opentxs::UnitType unitOfAccount,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason,
    const display::Definition& displayDefinition,
    const Amount& redemptionIncrement) noexcept
    -> std::shared_ptr<contract::unit::Currency>
{
    using ReturnType = contract::unit::implementation::Currency;

    auto output = std::make_shared<ReturnType>(
        api,
        nym,
        shortname,
        terms,
        unitOfAccount,
        version,
        displayDefinition,
        redemptionIncrement);

    if (false == bool(output)) { return {}; }

    auto& contract = *output;

    if (contract.Nym()) {
        auto serialized = contract.SigVersion();
        auto sig = std::make_shared<proto::Signature>();

        if (!contract.update_signature(reason)) { return {}; }
    }

    if (!contract.validate()) { return {}; }

    return output;
}

auto Factory::CurrencyContract(
    const api::Session& api,
    const Nym_p& nym,
    const proto::UnitDefinition serialized) noexcept
    -> std::shared_ptr<contract::unit::Currency>
{
    using ReturnType = contract::unit::implementation::Currency;

    if (false == proto::Validate<ReturnType::SerializedType>(
                     serialized, VERBOSE, true)) {

        return {};
    }

    auto output = std::make_shared<ReturnType>(api, nym, serialized);

    if (false == bool(output)) { return {}; }

    auto& contract = *output;

    if (false == contract.validate()) { return {}; }

    return output;
}
}  // namespace opentxs

namespace opentxs::contract::unit::implementation
{
using namespace std::literals;

Currency::Currency(
    const api::Session& api,
    const Nym_p& nym,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const opentxs::UnitType unitOfAccount,
    const VersionNumber version,
    const display::Definition& displayDefinition,
    const Amount& redemptionIncrement)
    : Unit(
          api,
          nym,
          shortname,
          terms,
          unitOfAccount,
          version,
          displayDefinition,
          redemptionIncrement)
{
    first_time_init();
}

Currency::Currency(
    const api::Session& api,
    const Nym_p& nym,
    const proto::UnitDefinition serialized)
    : Unit(api, nym, serialized)
{
    init_serialized();
}

Currency::Currency(const Currency& rhs)
    : Unit(rhs)
{
}

auto Currency::IDVersion() const -> SerializedType
{
    auto contract = Unit::IDVersion();

    return contract;
}
}  // namespace opentxs::contract::unit::implementation

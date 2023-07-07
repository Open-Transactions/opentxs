// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/SecurityContract.hpp"  // IWYU pragma: associated

#include <ContractEnums.pb.h>
#include <EquityParams.pb.h>
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
auto Factory::SecurityContract(
    const api::Session& api,
    const Nym_p& nym,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const opentxs::UnitType unitOfAccount,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason,
    const display::Definition& displayDefinition,
    const Amount& redemptionIncrement) noexcept
    -> std::shared_ptr<contract::unit::Security>
{
    using ReturnType = contract::unit::implementation::Security;
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

    if (false == contract.validate()) { return {}; }

    return output;
}

auto Factory::SecurityContract(
    const api::Session& api,
    const Nym_p& nym,
    const proto::UnitDefinition serialized) noexcept
    -> std::shared_ptr<contract::unit::Security>
{
    using ReturnType = contract::unit::implementation::Security;

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

Security::Security(
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

Security::Security(
    const api::Session& api,
    const Nym_p& nym,
    const proto::UnitDefinition serialized)
    : Unit(api, nym, serialized)
{
    init_serialized();
}

Security::Security(const Security& rhs)
    : Unit(rhs)
{
}

auto Security::IDVersion() const -> proto::UnitDefinition
{
    auto contract = Unit::IDVersion();

    auto& security = *contract.mutable_security();
    security.set_version(1);
    security.set_type(proto::EQUITYTYPE_SHARES);

    return contract;
}
}  // namespace opentxs::contract::unit::implementation

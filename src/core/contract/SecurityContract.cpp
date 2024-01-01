// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/SecurityContract.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ContractEnums.pb.h>
#include <opentxs/protobuf/EquityParams.pb.h>
#include <opentxs/protobuf/Signature.pb.h>
#include <opentxs/protobuf/UnitDefinition.pb.h>
#include <memory>
#include <string_view>

#include "core/contract/Signable.hpp"
#include "core/contract/Unit.hpp"
#include "opentxs/internal.factory.hpp"
#include "opentxs/protobuf/syntax/Types.internal.tpp"
#include "opentxs/protobuf/syntax/UnitDefinition.hpp"
#include "opentxs/util/Log.hpp"

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

    if (contract.Signer()) {
        auto serialized = contract.SigVersion();
        auto sig = std::make_shared<protobuf::Signature>();

        if (!contract.update_signature(reason)) { return {}; }
    }

    if (false == contract.validate()) { return {}; }

    return output;
}

auto Factory::SecurityContract(
    const api::Session& api,
    const Nym_p& nym,
    const protobuf::UnitDefinition serialized) noexcept
    -> std::shared_ptr<contract::unit::Security>
{
    using ReturnType = contract::unit::implementation::Security;

    if (false == protobuf::syntax::check(LogError(), serialized, true)) {
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
    const protobuf::UnitDefinition serialized)
    : Unit(api, nym, serialized)
{
    init_serialized();
}

Security::Security(const Security& rhs)
    : Unit(rhs)
{
}

auto Security::IDVersion() const -> protobuf::UnitDefinition
{
    auto contract = Unit::IDVersion();

    auto& security = *contract.mutable_security();
    security.set_version(1);
    security.set_type(protobuf::EQUITYTYPE_SHARES);

    return contract;
}
}  // namespace opentxs::contract::unit::implementation

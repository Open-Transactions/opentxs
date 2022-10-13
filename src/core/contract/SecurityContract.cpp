// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "core/contract/SecurityContract.hpp"  // IWYU pragma: associated

#include <ContractEnums.pb.h>
#include <EquityParams.pb.h>
#include <Signature.pb.h>
#include <UnitDefinition.pb.h>
#include <memory>

#include "2_Factory.hpp"
#include "core/contract/Unit.hpp"
#include "internal/core/contract/Unit.hpp"
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
    Lock lock(contract.lock_);

    if (contract.nym_) {
        auto serialized = contract.SigVersion(lock);
        auto sig = std::make_shared<proto::Signature>();

        if (!contract.update_signature(lock, reason)) { return {}; }
    }

    if (!contract.validate(lock)) { return {}; }

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
    Lock lock(contract.lock_);

    if (!contract.validate(lock)) { return {}; }

    return output;
}
}  // namespace opentxs

namespace opentxs::contract::unit::implementation
{
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
    Lock lock(lock_);
    first_time_init(lock);
}

Security::Security(
    const api::Session& api,
    const Nym_p& nym,
    const proto::UnitDefinition serialized)
    : Unit(api, nym, serialized)
{
    Lock lock(lock_);
    init_serialized(lock);
}

Security::Security(const Security& rhs)
    : Unit(rhs)
{
}

auto Security::IDVersion(const Lock& lock) const -> proto::UnitDefinition
{
    auto contract = Unit::IDVersion(lock);

    auto& security = *contract.mutable_security();
    security.set_version(1);
    security.set_type(proto::EQUITYTYPE_SHARES);

    return contract;
}
}  // namespace opentxs::contract::unit::implementation

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/BasketContract.hpp"  // IWYU pragma: associated

#include <BasketItem.pb.h>
#include <BasketParams.pb.h>
#include <Signature.pb.h>
#include <UnitDefinition.pb.h>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>

#include "2_Factory.hpp"
#include "core/contract/Unit.hpp"
#include "internal/core/contract/BasketContract.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/UnitDefinition.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
// Unlike the other factory functions, this one does not produce a complete,
// valid contract. This is used on the client side to produce a template for
// the server, which then finalizes the contract.
auto Factory::BasketContract(
    const api::Session& api,
    const Nym_p& nym,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const std::uint64_t weight,
    const opentxs::UnitType unitOfAccount,
    const VersionNumber version,
    const display::Definition& displayDefinition,
    const Amount& redemptionIncrement) noexcept
    -> std::shared_ptr<contract::unit::Basket>
{
    using ReturnType = opentxs::contract::unit::implementation::Basket;

    return std::make_shared<ReturnType>(
        api,
        nym,
        shortname,
        terms,
        weight,
        unitOfAccount,
        version,
        displayDefinition,
        redemptionIncrement);
}

auto Factory::BasketContract(
    const api::Session& api,
    const Nym_p& nym,
    const proto::UnitDefinition serialized) noexcept
    -> std::shared_ptr<contract::unit::Basket>
{
    using ReturnType = opentxs::contract::unit::implementation::Basket;

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

namespace opentxs::contract::unit
{
auto Basket::CalculateBasketID(
    const api::Session& api,
    const proto::UnitDefinition& serialized) -> identifier::Generic
{
    auto contract(serialized);
    contract.clear_id();
    contract.clear_issuer();
    contract.clear_issuer_nym();

    for (auto& item : *contract.mutable_basket()->mutable_item()) {
        item.clear_account();
    }

    return contract::implementation::Unit::GetID(api, contract);
}

auto Basket::FinalizeTemplate(
    const api::Session& api,
    const Nym_p& nym,
    proto::UnitDefinition& serialized,
    const PasswordPrompt& reason) -> bool
{
    using ReturnType = opentxs::contract::unit::implementation::Basket;
    auto contract = std::make_unique<ReturnType>(api, nym, serialized);

    if (!contract) { return false; }

    Lock lock(contract->lock_);

    try {
        contract->first_time_init(lock);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_STATIC(Basket))(e.what()).Flush();

        return false;
    }

    if (contract->nym_) {
        proto::UnitDefinition basket = contract->SigVersion(lock);
        std::shared_ptr<proto::Signature> sig =
            std::make_shared<proto::Signature>();
        if (contract->update_signature(lock, reason)) {
            lock.unlock();
            if (false == contract->Serialize(serialized, true)) {
                LogError()(OT_PRETTY_STATIC(Basket))(
                    "Failed to serialize unit definition.")
                    .Flush();

                return false;
            }

            return proto::Validate(serialized, VERBOSE, false);
        }
    }

    return false;
}
}  // namespace opentxs::contract::unit

namespace opentxs::contract::unit::implementation
{
Basket::Basket(
    const api::Session& api,
    const Nym_p& nym,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const std::uint64_t weight,
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
    , subcontracts_()
    , weight_(weight)
{
}

Basket::Basket(
    const api::Session& api,
    const Nym_p& nym,
    const proto::UnitDefinition serialized)
    : Unit(api, nym, serialized)
    , subcontracts_()
    , weight_(0)
{
    if (serialized.has_basket()) {

        if (serialized.basket().has_weight()) {
            weight_ = serialized.basket().weight();
        }

        for (const auto& item : serialized.basket().item()) {
            subcontracts_.insert(
                {item.unit(), {item.account(), item.weight()}});
        }
    }
}

Basket::Basket(const Basket& rhs)
    : Unit(rhs)
    , subcontracts_(rhs.subcontracts_)
    , weight_(rhs.weight_)
{
}

auto Basket::BasketID() const -> identifier::Generic
{
    Lock lock(lock_);

    return GetID(api_, BasketIDVersion(lock));
}

auto Basket::IDVersion(const Lock& lock) const -> SerializedType
{
    auto contract = Unit::IDVersion(lock);

    auto* basket = contract.mutable_basket();
    basket->set_version(1);
    basket->set_weight(weight_);

    // determinism here depends on the defined ordering of UnallocatedMap
    for (const auto& item : subcontracts_) {
        auto* serialized = basket->add_item();
        serialized->set_version(1);
        serialized->set_unit(item.first);
        serialized->set_account(item.second.first);
        serialized->set_weight(item.second.second);
    }

    return contract;
}

auto Basket::BasketIDVersion(const Lock& lock) const -> SerializedType
{
    auto contract = Unit::SigVersion(lock);

    for (auto& item : *(contract.mutable_basket()->mutable_item())) {
        item.clear_account();
    }

    return contract;
}
}  // namespace opentxs::contract::unit::implementation

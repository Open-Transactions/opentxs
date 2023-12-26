// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/BasketContract.hpp"  // IWYU pragma: associated

#include <BasketItem.pb.h>
#include <BasketParams.pb.h>
#include <Signature.pb.h>
#include <UnitDefinition.pb.h>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "core/contract/Signable.hpp"
#include "core/contract/Unit.hpp"
#include "internal/core/contract/BasketContract.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/UnitDefinition.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/internal.factory.hpp"
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

    if (false == contract.validate()) { return {}; }

    return output;
}
}  // namespace opentxs

namespace opentxs::contract::unit
{
using namespace std::literals;

auto Basket::CalculateBasketID(
    const api::Session& api,
    const proto::UnitDefinition& serialized) -> identifier_type
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

    try {
        contract->first_time_init();
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }

    if (contract->Signer()) {
        auto basket = contract->SigVersion();
        auto sig = std::make_shared<proto::Signature>();

        if (contract->update_signature(reason)) {
            if (false == contract->Serialize(serialized, true)) {
                LogError()()("Failed to serialize unit definition.").Flush();

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
    , subcontracts_([&] {
        auto out = decltype(subcontracts_){};

        if (serialized.has_basket()) {
            for (const auto& item : serialized.basket().item()) {
                out.insert({item.unit(), {item.account(), item.weight()}});
            }
        }

        return out;
    }())
    , weight_([&] {
        auto out = decltype(weight_){};

        if (serialized.has_basket()) { out = serialized.basket().weight(); }

        return out;
    }())
{
}

Basket::Basket(const Basket& rhs)
    : Unit(rhs)
    , subcontracts_(rhs.subcontracts_)
    , weight_(rhs.weight_)
{
}

auto Basket::BasketID() const -> identifier_type
{
    return GetID(api_, BasketIDVersion());
}

auto Basket::IDVersion() const -> SerializedType
{
    auto contract = Unit::IDVersion();

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

auto Basket::BasketIDVersion() const -> SerializedType
{
    auto contract = Unit::SigVersion();

    for (auto& item : *(contract.mutable_basket()->mutable_item())) {
        item.clear_account();
    }

    return contract;
}
}  // namespace opentxs::contract::unit::implementation

// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/contact/ContactItem.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <opentxs/protobuf/ContactItem.pb.h>
#include <span>
#include <utility>

#include "opentxs/identity/wot/claim/internal.factory.hpp"
#include "opentxs/protobuf/Types.internal.tpp"

namespace ottest
{
ContactItem::ContactItem()
    : nym_id_(client_1_.Factory().NymIDFromRandom())
    , contact_item_(claim_to_contact_item(client_1_.Factory().Claim(
          {nym_id_},
          opentxs::identity::wot::claim::SectionType::Identifier,
          opentxs::identity::wot::claim::ClaimType::Employee,
          "testValue",
          active_)))
{
}
}  // namespace ottest

namespace ottest
{
auto claim_to_contact_item(const ot::identity::wot::Claim& claim) noexcept
    -> ot::identity::wot::claim::Item
{
    return opentxs::factory::ContactItem(claim, {});
}

auto deserialize_contact_item(
    const opentxs::api::Session& api,
    const opentxs::identity::wot::Claimant& claimant,
    opentxs::identity::wot::claim::SectionType section,
    opentxs::ReadView bytes) noexcept -> opentxs::identity::wot::claim::Item
{
    const auto proto =
        opentxs::protobuf::Factory<opentxs::protobuf::ContactItem>(
            bytes.data(), bytes.size());

    return opentxs::factory::ContactItem(api, proto, claimant, section, {});
}

auto modify_item(
    const opentxs::identity::wot::claim::Item& item,
    std::optional<std::string_view> value,
    std::optional<opentxs::ReadView> subtype,
    std::optional<opentxs::Time> start,
    std::optional<opentxs::Time> end) noexcept
    -> opentxs::identity::wot::claim::Item
{
    return claim_to_contact_item(item.asClaim().CreateModified(
        std::move(value),
        std::move(subtype),
        std::move(start),
        std::move(end)));
}
}  // namespace ottest

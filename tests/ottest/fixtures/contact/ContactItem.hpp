// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <optional>
#include <string_view>

#include "ottest/fixtures/common/OneClientSession.hpp"

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT ContactItem : public OneClientSession
{
public:
    static constexpr auto active_ = {
        opentxs::identity::wot::claim::Attribute::Active};

    ContactItem();

    const opentxs::identifier::Nym nym_id_;
    const opentxs::identity::wot::claim::Item contact_item_;
};

OPENTXS_EXPORT auto claim_to_contact_item(
    const opentxs::identity::wot::Claim&) noexcept
    -> opentxs::identity::wot::claim::Item;
OPENTXS_EXPORT auto deserialize_contact_item(
    const opentxs::api::Session& api,
    const opentxs::identity::wot::Claimant& claimant,
    opentxs::identity::wot::claim::SectionType section,
    opentxs::ReadView bytes) noexcept -> opentxs::identity::wot::claim::Item;
OPENTXS_EXPORT auto modify_item(
    const opentxs::identity::wot::claim::Item& item,
    std::optional<std::string_view> value = std::nullopt,
    std::optional<opentxs::ReadView> subtype = std::nullopt,
    std::optional<opentxs::Time> start = std::nullopt,
    std::optional<opentxs::Time> end = std::nullopt) noexcept
    -> opentxs::identity::wot::claim::Item;
}  // namespace ottest

// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <memory>
#include <string_view>

#include "ottest/fixtures/common/OneClientSession.hpp"

namespace ottest
{
namespace ot = opentxs;
namespace claim = ot::identity::wot::claim;
using namespace std::literals;

class OPENTXS_EXPORT ContactData : public OneClientSession
{
public:
    static constexpr auto active_attr_ = {
        opentxs::identity::wot::claim::Attribute::Active};

    const ot::identifier::Nym nym_id_1_;
    const ot::identifier::Nym nym_id_2_;
    const ot::identifier::Nym nym_id_3_;
    const ot::identifier::Nym nym_id_4_;
    const claim::Data contact_data_;
    const std::shared_ptr<claim::Item> active_contact_item_;

    using CallbackType1 = claim::Data (*)(
        const claim::Data&,
        const ot::UnallocatedCString&,
        const ot::UnitType,
        const bool,
        const bool);
    using CallbackType2 = claim::Data (*)(
        const claim::Data&,
        const ot::UnallocatedCString&,
        const bool,
        const bool);

    void testAddItemMethod(
        const CallbackType1 contactDataMethod,
        claim::SectionType sectionName,
        opentxs::VersionNumber version =
            opentxs::identity::wot::claim::DefaultVersion(),
        opentxs::VersionNumber targetVersion = 0);

    void testAddItemMethod2(
        const CallbackType2 contactDataMethod,
        claim::SectionType sectionName,
        claim::ClaimType itemType,
        opentxs::VersionNumber version =
            opentxs::identity::wot::claim::DefaultVersion(),
        opentxs::VersionNumber targetVersion = 0);

    ContactData();
};
}  // namespace ottest

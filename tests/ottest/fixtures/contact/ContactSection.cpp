// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/contact/ContactSection.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <memory>
#include <span>

#include "ottest/fixtures/contact/ContactItem.hpp"

namespace ottest
{
namespace ot = opentxs;

ContactSection::ContactSection()
    : nym_id_(client_1_.Factory().NymIDFromRandom())
    , contact_section_(
          dynamic_cast<const ot::api::session::Client&>(client_1_),
          ot::UnallocatedCString("testContactSectionNym1"),
          opentxs::identity::wot::claim::DefaultVersion(),
          opentxs::identity::wot::claim::DefaultVersion(),
          ot::identity::wot::claim::SectionType::Identifier,
          ot::identity::wot::claim::Section::GroupMap{})
    , contact_group_(new ot::identity::wot::claim::Group(
          ot::UnallocatedCString("testContactGroupNym1"),
          ot::identity::wot::claim::SectionType::Identifier,
          ot::identity::wot::claim::ClaimType::Employee,
          {}))
    , active_contact_item_(std::make_shared<ot::identity::wot::claim::Item>(
          claim_to_contact_item(client_1_.Factory().Claim(
              nym_id_,
              ot::identity::wot::claim::SectionType::Identifier,
              ot::identity::wot::claim::ClaimType::Employee,
              "activeContactItemValue",
              active_attr_))))
{
}
}  // namespace ottest

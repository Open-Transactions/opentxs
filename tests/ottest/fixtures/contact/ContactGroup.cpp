// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/contact/ContactGroup.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <memory>
#include <span>

#include "ottest/fixtures/contact/ContactItem.hpp"

namespace ottest
{
ContactGroup::ContactGroup()
    : nym_id_(client_1_.Factory().NymIDFromRandom())
    , contact_group_(
          nym_id_.asBase58(client_1_.Crypto()),
          ot::identity::wot::claim::SectionType::Identifier,
          ot::identity::wot::claim::ClaimType::Employee,
          {})
    , primary_(std::make_shared<ot::identity::wot::claim::Item>(
          claim_to_contact_item(client_1_.Factory().Claim(
              {nym_id_},
              opentxs::identity::wot::claim::SectionType::Identifier,
              opentxs::identity::wot::claim::ClaimType::Employee,
              "primaryContactItemValue",
              primary_attr_))))
    , active_(std::make_shared<ot::identity::wot::claim::Item>(
          claim_to_contact_item(client_1_.Factory().Claim(
              {nym_id_},
              opentxs::identity::wot::claim::SectionType::Identifier,
              opentxs::identity::wot::claim::ClaimType::Employee,
              "activeContactItemValue",
              active_attr_))))
{
}
}  // namespace ottest

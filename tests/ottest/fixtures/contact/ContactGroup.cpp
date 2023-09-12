// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/contact/ContactGroup.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <memory>

#include "internal/identity/wot/claim/Types.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
namespace ot = opentxs;

ContactGroup::ContactGroup()
    : api_(OTTestEnvironment::GetOT().StartClientSession(0))
    , contact_group_(
          ot::UnallocatedCString("testContactGroupNym1"),
          ot::identity::wot::claim::SectionType::Identifier,
          ot::identity::wot::claim::ClaimType::Employee,
          {})
    , primary_(new ot::identity::wot::claim::Item(
          dynamic_cast<const ot::api::session::Client&>(api_),
          ot::UnallocatedCString("primaryContactItem"),
          opentxs::CONTACT_CONTACT_DATA_VERSION,
          opentxs::CONTACT_CONTACT_DATA_VERSION,
          ot::identity::wot::claim::SectionType::Identifier,
          ot::identity::wot::claim::ClaimType::Employee,
          ot::UnallocatedCString("primaryContactItemValue"),
          {ot::identity::wot::claim::Attribute::Primary},
          {},
          {},
          ""))
    , active_(new ot::identity::wot::claim::Item(
          dynamic_cast<const ot::api::session::Client&>(api_),
          ot::UnallocatedCString("activeContactItem"),
          opentxs::CONTACT_CONTACT_DATA_VERSION,
          opentxs::CONTACT_CONTACT_DATA_VERSION,
          ot::identity::wot::claim::SectionType::Identifier,
          ot::identity::wot::claim::ClaimType::Employee,
          ot::UnallocatedCString("activeContactItemValue"),
          {ot::identity::wot::claim::Attribute::Active},
          {},
          {},
          ""))
{
}
}  // namespace ottest

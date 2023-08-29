// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/contact/ContactItem.hpp"  // IWYU pragma: associated

#include "internal/identity/wot/claim/Types.hpp"
#include "opentxs/opentxs.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
namespace ot = opentxs;

ContactItem::ContactItem()
    : api_(OTTestEnvironment::GetOT().StartClientSession(0))
    , contact_item_(
          dynamic_cast<const ot::api::session::Client&>(api_),
          ot::UnallocatedCString("testNym"),
          opentxs::CONTACT_CONTACT_DATA_VERSION,
          opentxs::CONTACT_CONTACT_DATA_VERSION,
          ot::identity::wot::claim::SectionType::Identifier,
          ot::identity::wot::claim::ClaimType::Employee,
          ot::UnallocatedCString("testValue"),
          {ot::identity::wot::claim::Attribute::Active},
          {},
          {},
          "")
    , nym_id_(api_.Factory().NymIDFromRandom())
{
}
}  // namespace ottest

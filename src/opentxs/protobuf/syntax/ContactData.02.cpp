// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::ContactSectionName

#include "opentxs/protobuf/syntax/ContactData.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/syntax/VerifyContacts.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_2(
    const ContactData& input,
    const Log& log,
    const ClaimType indexed) -> bool
{
    return version_1(input, log, indexed);
}
}  // namespace opentxs::protobuf::inline syntax

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::ContactItemAttribute

#include "opentxs/protobuf/syntax/ContactItem.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/contact/Types.internal.hpp"
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_5(
    const ContactItem& input,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool
{
    return version_1(input, log, indexed, parentVersion);
}

auto version_5(const ContactItem& input, const Log& log) -> bool
{
    return version_1(input, log);
}
}  // namespace opentxs::protobuf::inline syntax

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/contact/Types.internal.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ContactItemAttribute.pb.h>

namespace opentxs::protobuf::contact
{
auto AllowedItemAttributes() noexcept -> const ItemAttributeMap&
{
    static const auto output = ItemAttributeMap{
        {1, {CITEMATTR_ACTIVE, CITEMATTR_PRIMARY}},
        {2, {CITEMATTR_ACTIVE, CITEMATTR_PRIMARY}},
        {3, {CITEMATTR_ACTIVE, CITEMATTR_PRIMARY, CITEMATTR_LOCAL}},
        {4, {CITEMATTR_ACTIVE, CITEMATTR_PRIMARY, CITEMATTR_LOCAL}},
        {5, {CITEMATTR_ACTIVE, CITEMATTR_PRIMARY, CITEMATTR_LOCAL}},
        {6, {CITEMATTR_ACTIVE, CITEMATTR_PRIMARY, CITEMATTR_LOCAL}},
    };

    return output;
}
}  // namespace opentxs::protobuf::contact

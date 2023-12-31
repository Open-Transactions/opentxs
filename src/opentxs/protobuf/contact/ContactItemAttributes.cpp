// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/contact/Types.internal.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ContactItemAttribute.pb.h>

namespace opentxs::protobuf::contact
{
auto ContactItemAttributes() noexcept -> const EnumTranslation&
{
    static const auto output = EnumTranslation{
        {{CITEMATTR_ACTIVE, "en"}, "Active"},
        {{CITEMATTR_PRIMARY, "en"}, "Primary"},
        {{CITEMATTR_LOCAL, "en"}, "Local"}};

    return output;
}
}  // namespace opentxs::protobuf::contact

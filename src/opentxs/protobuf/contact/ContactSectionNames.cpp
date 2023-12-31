// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/contact/Types.internal.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ContactSectionName.pb.h>

namespace opentxs::protobuf::contact
{
auto ContactSectionNames() noexcept -> const EnumTranslation&
{
    static const auto output = EnumTranslation{
        {{CONTACTSECTION_SCOPE, "en"}, "Scope"},
        {{CONTACTSECTION_IDENTIFIER, "en"}, "Identifier"},
        {{CONTACTSECTION_ADDRESS, "en"}, "Address"},
        {{CONTACTSECTION_COMMUNICATION, "en"}, "Communication"},
        {{CONTACTSECTION_PROFILE, "en"}, "Profile"},
        {{CONTACTSECTION_RELATIONSHIP, "en"}, "Relationships"},
        {{CONTACTSECTION_DESCRIPTOR, "en"}, "Descriptor"},
        {{CONTACTSECTION_EVENT, "en"}, "Event"},
        {{CONTACTSECTION_CONTRACT, "en"}, "Contracts"},
        {{CONTACTSECTION_PROCEDURE, "en"}, "Procedures"}};

    return output;
}
}  // namespace opentxs::protobuf::contact

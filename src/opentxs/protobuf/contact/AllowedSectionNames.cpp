// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/contact/Types.internal.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ContactSectionName.pb.h>

namespace opentxs::protobuf::contact
{
auto AllowedSectionNames() noexcept -> const ContactSectionMap&
{
    static const auto output = ContactSectionMap{
        {1,
         {CONTACTSECTION_SCOPE,
          CONTACTSECTION_IDENTIFIER,
          CONTACTSECTION_ADDRESS,
          CONTACTSECTION_COMMUNICATION,
          CONTACTSECTION_PROFILE,
          CONTACTSECTION_RELATIONSHIP,
          CONTACTSECTION_DESCRIPTOR,
          CONTACTSECTION_EVENT,
          CONTACTSECTION_CONTRACT}},
        {2,
         {CONTACTSECTION_SCOPE,
          CONTACTSECTION_IDENTIFIER,
          CONTACTSECTION_ADDRESS,
          CONTACTSECTION_COMMUNICATION,
          CONTACTSECTION_PROFILE,
          CONTACTSECTION_RELATIONSHIP,
          CONTACTSECTION_DESCRIPTOR,
          CONTACTSECTION_EVENT,
          CONTACTSECTION_CONTRACT}},
        {3,
         {CONTACTSECTION_SCOPE,
          CONTACTSECTION_IDENTIFIER,
          CONTACTSECTION_ADDRESS,
          CONTACTSECTION_COMMUNICATION,
          CONTACTSECTION_PROFILE,
          CONTACTSECTION_RELATIONSHIP,
          CONTACTSECTION_DESCRIPTOR,
          CONTACTSECTION_EVENT,
          CONTACTSECTION_CONTRACT,
          CONTACTSECTION_PROCEDURE}},
        {4,
         {CONTACTSECTION_SCOPE,
          CONTACTSECTION_IDENTIFIER,
          CONTACTSECTION_ADDRESS,
          CONTACTSECTION_COMMUNICATION,
          CONTACTSECTION_PROFILE,
          CONTACTSECTION_RELATIONSHIP,
          CONTACTSECTION_DESCRIPTOR,
          CONTACTSECTION_EVENT,
          CONTACTSECTION_CONTRACT,
          CONTACTSECTION_PROCEDURE}},
        {5,
         {CONTACTSECTION_SCOPE,
          CONTACTSECTION_IDENTIFIER,
          CONTACTSECTION_ADDRESS,
          CONTACTSECTION_COMMUNICATION,
          CONTACTSECTION_PROFILE,
          CONTACTSECTION_RELATIONSHIP,
          CONTACTSECTION_DESCRIPTOR,
          CONTACTSECTION_EVENT,
          CONTACTSECTION_CONTRACT,
          CONTACTSECTION_PROCEDURE}},
        {6,
         {CONTACTSECTION_SCOPE,
          CONTACTSECTION_IDENTIFIER,
          CONTACTSECTION_ADDRESS,
          CONTACTSECTION_COMMUNICATION,
          CONTACTSECTION_PROFILE,
          CONTACTSECTION_RELATIONSHIP,
          CONTACTSECTION_DESCRIPTOR,
          CONTACTSECTION_EVENT,
          CONTACTSECTION_CONTRACT,
          CONTACTSECTION_PROCEDURE}},
    };

    return output;
}
}  // namespace opentxs::protobuf::contact

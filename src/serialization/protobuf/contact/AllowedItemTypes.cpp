// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/Contact.hpp"  // IWYU pragma: associated

#include <ContactItemType.pb.h>  // IWYU pragma: keep
#include <ContactSectionName.pb.h>

namespace opentxs::proto
{
auto AllowedItemTypes() noexcept -> const ContactItemMap&
{
    static const auto output = ContactItemMap{
        {{1, CONTACTSECTION_SCOPE},
         {
#include "serialization/protobuf/contact/alloweditemtypes/scope.1"  // IWYU pragma: keep
         }},
        {{2, CONTACTSECTION_SCOPE},
         {
#include "serialization/protobuf/contact/alloweditemtypes/scope.2"  // IWYU pragma: keep
         }},
        {{3, CONTACTSECTION_SCOPE},
         {
#include "serialization/protobuf/contact/alloweditemtypes/scope.3"  // IWYU pragma: keep
         }},
        {{4, CONTACTSECTION_SCOPE},
         {
#include "serialization/protobuf/contact/alloweditemtypes/scope.4"  // IWYU pragma: keep
         }},
        {{5, CONTACTSECTION_SCOPE},
         {
#include "serialization/protobuf/contact/alloweditemtypes/scope.5"  // IWYU pragma: keep
         }},
        {{6, CONTACTSECTION_SCOPE},
         {
#include "serialization/protobuf/contact/alloweditemtypes/scope.6"  // IWYU pragma: keep
         }},
        {{1, CONTACTSECTION_IDENTIFIER},
         {
#include "serialization/protobuf/contact/alloweditemtypes/identifier.1"  // IWYU pragma: keep
         }},
        {{2, CONTACTSECTION_IDENTIFIER},
         {
#include "serialization/protobuf/contact/alloweditemtypes/identifier.2"  // IWYU pragma: keep
         }},
        {{3, CONTACTSECTION_IDENTIFIER},
         {
#include "serialization/protobuf/contact/alloweditemtypes/identifier.3"  // IWYU pragma: keep
         }},
        {{4, CONTACTSECTION_IDENTIFIER},
         {
#include "serialization/protobuf/contact/alloweditemtypes/identifier.4"  // IWYU pragma: keep
         }},
        {{5, CONTACTSECTION_IDENTIFIER},
         {
#include "serialization/protobuf/contact/alloweditemtypes/identifier.5"  // IWYU pragma: keep
         }},
        {{6, CONTACTSECTION_IDENTIFIER},
         {
#include "serialization/protobuf/contact/alloweditemtypes/identifier.6"  // IWYU pragma: keep
         }},
        {{1, CONTACTSECTION_ADDRESS},
         {
#include "serialization/protobuf/contact/alloweditemtypes/address.1"  // IWYU pragma: keep
         }},
        {{2, CONTACTSECTION_ADDRESS},
         {
#include "serialization/protobuf/contact/alloweditemtypes/address.2"  // IWYU pragma: keep
         }},
        {{3, CONTACTSECTION_ADDRESS},
         {
#include "serialization/protobuf/contact/alloweditemtypes/address.3"  // IWYU pragma: keep
         }},
        {{4, CONTACTSECTION_ADDRESS},
         {
#include "serialization/protobuf/contact/alloweditemtypes/address.4"  // IWYU pragma: keep
         }},
        {{5, CONTACTSECTION_ADDRESS},
         {
#include "serialization/protobuf/contact/alloweditemtypes/address.5"  // IWYU pragma: keep
         }},
        {{6, CONTACTSECTION_ADDRESS},
         {
#include "serialization/protobuf/contact/alloweditemtypes/address.6"  // IWYU pragma: keep
         }},
        {{1, CONTACTSECTION_COMMUNICATION},
         {
#include "serialization/protobuf/contact/alloweditemtypes/communication.1"  // IWYU pragma: keep
         }},
        {{2, CONTACTSECTION_COMMUNICATION},
         {
#include "serialization/protobuf/contact/alloweditemtypes/communication.2"  // IWYU pragma: keep
         }},
        {{3, CONTACTSECTION_COMMUNICATION},
         {
#include "serialization/protobuf/contact/alloweditemtypes/communication.3"  // IWYU pragma: keep
         }},
        {{4, CONTACTSECTION_COMMUNICATION},
         {
#include "serialization/protobuf/contact/alloweditemtypes/communication.4"  // IWYU pragma: keep
         }},
        {{5, CONTACTSECTION_COMMUNICATION},
         {
#include "serialization/protobuf/contact/alloweditemtypes/communication.5"  // IWYU pragma: keep
         }},
        {{6, CONTACTSECTION_COMMUNICATION},
         {
#include "serialization/protobuf/contact/alloweditemtypes/communication.6"  // IWYU pragma: keep
         }},
        {{1, CONTACTSECTION_PROFILE},
         {
#include "serialization/protobuf/contact/alloweditemtypes/profile.1"  // IWYU pragma: keep
         }},
        {{2, CONTACTSECTION_PROFILE},
         {
#include "serialization/protobuf/contact/alloweditemtypes/profile.2"  // IWYU pragma: keep
         }},
        {{3, CONTACTSECTION_PROFILE},
         {
#include "serialization/protobuf/contact/alloweditemtypes/profile.3"  // IWYU pragma: keep
         }},
        {{4, CONTACTSECTION_PROFILE},
         {
#include "serialization/protobuf/contact/alloweditemtypes/profile.4"  // IWYU pragma: keep
         }},
        {{5, CONTACTSECTION_PROFILE},
         {
#include "serialization/protobuf/contact/alloweditemtypes/profile.5"  // IWYU pragma: keep
         }},
        {{6, CONTACTSECTION_PROFILE},
         {
#include "serialization/protobuf/contact/alloweditemtypes/profile.6"  // IWYU pragma: keep
         }},
        {{1, CONTACTSECTION_RELATIONSHIP},
         {
#include "serialization/protobuf/contact/alloweditemtypes/relationship.1"  // IWYU pragma: keep
         }},
        {{2, CONTACTSECTION_RELATIONSHIP},
         {
#include "serialization/protobuf/contact/alloweditemtypes/relationship.2"  // IWYU pragma: keep
         }},
        {{3, CONTACTSECTION_RELATIONSHIP},
         {
#include "serialization/protobuf/contact/alloweditemtypes/relationship.3"  // IWYU pragma: keep
         }},
        {{4, CONTACTSECTION_RELATIONSHIP},
         {
#include "serialization/protobuf/contact/alloweditemtypes/relationship.4"  // IWYU pragma: keep
         }},
        {{5, CONTACTSECTION_RELATIONSHIP},
         {
#include "serialization/protobuf/contact/alloweditemtypes/relationship.5"  // IWYU pragma: keep
         }},
        {{6, CONTACTSECTION_RELATIONSHIP},
         {
#include "serialization/protobuf/contact/alloweditemtypes/relationship.6"  // IWYU pragma: keep
         }},
        {{1, CONTACTSECTION_DESCRIPTOR},
         {
#include "serialization/protobuf/contact/alloweditemtypes/descriptor.1"  // IWYU pragma: keep
         }},
        {{2, CONTACTSECTION_DESCRIPTOR},
         {
#include "serialization/protobuf/contact/alloweditemtypes/descriptor.2"  // IWYU pragma: keep
         }},
        {{3, CONTACTSECTION_DESCRIPTOR},
         {
#include "serialization/protobuf/contact/alloweditemtypes/descriptor.3"  // IWYU pragma: keep
         }},
        {{4, CONTACTSECTION_DESCRIPTOR},
         {
#include "serialization/protobuf/contact/alloweditemtypes/descriptor.4"  // IWYU pragma: keep
         }},
        {{5, CONTACTSECTION_DESCRIPTOR},
         {
#include "serialization/protobuf/contact/alloweditemtypes/descriptor.5"  // IWYU pragma: keep
         }},
        {{6, CONTACTSECTION_DESCRIPTOR},
         {
#include "serialization/protobuf/contact/alloweditemtypes/descriptor.6"  // IWYU pragma: keep
         }},
        {{1, CONTACTSECTION_EVENT},
         {
#include "serialization/protobuf/contact/alloweditemtypes/event.1"  // IWYU pragma: keep
         }},
        {{2, CONTACTSECTION_EVENT},
         {
#include "serialization/protobuf/contact/alloweditemtypes/event.2"  // IWYU pragma: keep
         }},
        {{3, CONTACTSECTION_EVENT},
         {
#include "serialization/protobuf/contact/alloweditemtypes/event.3"  // IWYU pragma: keep
         }},
        {{4, CONTACTSECTION_EVENT},
         {
#include "serialization/protobuf/contact/alloweditemtypes/event.4"  // IWYU pragma: keep
         }},
        {{5, CONTACTSECTION_EVENT},
         {
#include "serialization/protobuf/contact/alloweditemtypes/event.5"  // IWYU pragma: keep
         }},
        {{6, CONTACTSECTION_EVENT},
         {
#include "serialization/protobuf/contact/alloweditemtypes/event.6"  // IWYU pragma: keep
         }},
        {{1, CONTACTSECTION_CONTRACT},
         {
#include "serialization/protobuf/contact/alloweditemtypes/contract.1"  // IWYU pragma: keep
         }},
        {{2, CONTACTSECTION_CONTRACT},
         {
#include "serialization/protobuf/contact/alloweditemtypes/contract.2"  // IWYU pragma: keep
         }},
        {{3, CONTACTSECTION_CONTRACT},
         {
#include "serialization/protobuf/contact/alloweditemtypes/contract.3"  // IWYU pragma: keep
         }},
        {{4, CONTACTSECTION_CONTRACT},
         {
#include "serialization/protobuf/contact/alloweditemtypes/contract.4"  // IWYU pragma: keep
         }},
        {{5, CONTACTSECTION_CONTRACT},
         {
#include "serialization/protobuf/contact/alloweditemtypes/contract.5"  // IWYU pragma: keep
         }},
        {{6, CONTACTSECTION_CONTRACT},
         {
#include "serialization/protobuf/contact/alloweditemtypes/contract.6"  // IWYU pragma: keep
         }},
        {{3, CONTACTSECTION_PROCEDURE},
         {
#include "serialization/protobuf/contact/alloweditemtypes/procedure.3"  // IWYU pragma: keep
         }},
        {{4, CONTACTSECTION_PROCEDURE},
         {
#include "serialization/protobuf/contact/alloweditemtypes/procedure.4"  // IWYU pragma: keep
         }},
        {{5, CONTACTSECTION_PROCEDURE},
         {
#include "serialization/protobuf/contact/alloweditemtypes/procedure.5"  // IWYU pragma: keep
         }},
        {{6, CONTACTSECTION_PROCEDURE},
         {
#include "serialization/protobuf/contact/alloweditemtypes/procedure.6"  // IWYU pragma: keep
         }},
    };

    return output;
}
}  // namespace opentxs::proto

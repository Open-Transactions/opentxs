// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace ottest
{
class User;
struct Counter;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ot = opentxs;

namespace ottest
{
struct OPENTXS_EXPORT ContactListRow {
    bool check_contact_id_{};
    ot::UnallocatedCString contact_id_index_{};
    ot::UnallocatedCString name_{};
    ot::UnallocatedCString section_{};
    ot::UnallocatedCString image_{};
};

struct OPENTXS_EXPORT ContactListData {
    ot::UnallocatedVector<ContactListRow> rows_{};
};

OPENTXS_EXPORT auto check_contact_list(
    const User& user,
    const ContactListData& expected) noexcept -> bool;
OPENTXS_EXPORT auto check_contact_list_qt(
    const User& user,
    const ContactListData& expected) noexcept -> bool;
OPENTXS_EXPORT auto check_messagable_list(
    const User& user,
    const ContactListData& expected) noexcept -> bool;
OPENTXS_EXPORT auto check_messagable_list_qt(
    const User& user,
    const ContactListData& expected) noexcept -> bool;
OPENTXS_EXPORT auto contact_list_add_contact(
    const User& user,
    const ot::UnallocatedCString& label,
    const ot::UnallocatedCString& paymentCode,
    const ot::UnallocatedCString& nymID) noexcept -> ot::UnallocatedCString;
OPENTXS_EXPORT auto contact_list_rename_contact(
    const User& user,
    const ot::UnallocatedCString& contactID,
    const ot::UnallocatedCString& newname) noexcept -> bool;
OPENTXS_EXPORT auto init_contact_list(
    const User& user,
    Counter& counter) noexcept -> void;
OPENTXS_EXPORT auto init_messagable_list(
    const User& user,
    Counter& counter) noexcept -> void;
}  // namespace ottest

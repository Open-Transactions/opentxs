// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace ottest
{
class User;
struct Counter;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
namespace ot = opentxs;

constexpr auto words_{"response seminar brave tip suit recall often sound "
                      "stick owner lottery motion"};
constexpr auto name_{"Alice"};
constexpr auto payment_code_1_{
    "PM8TJS2JxQ5ztXUpBBRnpTbcUXbUHy2T1abfrb3KkAAtMEGNbey4oumH7Hc578WgQJhPjBxteQ"
    "5GHHToTYHE3A1w6p7tU6KSoFmWBVbFGjKPisZDbP97"};
constexpr auto payment_code_2_{
    "PM8TJfV1DQD6VScd5AWsSax8RgK9cUREe939M1d85MwGCKJukyghX6B5E7kqcCyEYu6Tu1ZvdG"
    "8aWh6w8KGhSfjgL8fBKuZS6aUjhV9xLV1R16CcgWhw"};

constexpr auto bob_{"Bob"};
constexpr auto chris_{"Chris"};
constexpr auto daniel_{"Daniel"};
constexpr auto payment_code_3_{
    "PD1kEC92CeshFRQ3V78XPAGmE1ZWy3YR4Ptsjxw8SxHgZvFVkwqjf"};

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

class OPENTXS_EXPORT ContactList : public ::testing::Test
{
public:
    static const User alice_;

    const ot::api::session::Client& api_;
    ot::PasswordPrompt reason_;
    const ot::PaymentCode bob_payment_code_;
    const ot::PaymentCode chris_payment_code_;

    ContactList();
};
}  // namespace ottest

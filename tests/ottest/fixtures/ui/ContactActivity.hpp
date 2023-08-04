// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <optional>

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
struct OPENTXS_EXPORT ContactActivityRow {
    bool loading_{};
    bool pending_{};
    bool outgoing_{};
    int polarity_{};
    ot::Amount amount_{};
    ot::UnallocatedCString display_amount_{};
    ot::UnallocatedCString from_{};
    ot::UnallocatedCString text_{};
    ot::UnallocatedCString memo_{};
    ot::otx::client::StorageBox type_{};
    std::optional<ot::Time> timestamp_{};
    ot::UnallocatedCString txid_{};
};

struct OPENTXS_EXPORT ContactActivityData {
    bool can_message_{};
    ot::UnallocatedCString thread_id_{};
    ot::UnallocatedCString display_name_{};
    ot::UnallocatedCString draft_{};
    ot::UnallocatedCString participants_{};
    ot::UnallocatedMap<ot::UnitType, ot::UnallocatedCString> payment_codes_{};
    ot::UnallocatedVector<ContactActivityRow> rows_{};
};

OPENTXS_EXPORT auto contact_activity_request_faucet(
    const User& user,
    const User& contact) noexcept -> bool;
OPENTXS_EXPORT auto contact_activity_send_message(
    const User& user,
    const User& contact) noexcept -> bool;
OPENTXS_EXPORT auto contact_activity_send_message(
    const User& user,
    const User& contact,
    const ot::UnallocatedCString& messasge) noexcept -> bool;
OPENTXS_EXPORT auto check_contact_activity(
    const User& user,
    const ot::identifier::Generic& contact,
    const ContactActivityData& expected) noexcept -> bool;
OPENTXS_EXPORT auto check_contact_activity_qt(
    const User& user,
    const ot::identifier::Generic& contact,
    const ContactActivityData& expected) noexcept -> bool;
OPENTXS_EXPORT auto init_contact_activity(
    const User& user,
    const User& contact,
    Counter& counter) noexcept -> void;
}  // namespace ottest

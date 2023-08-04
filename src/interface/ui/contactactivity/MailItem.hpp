// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "interface/ui/contactactivity/ContactActivityItem.hpp"
#include "internal/interface/ui/UI.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
class MailItem final : public ContactActivityItem
{
public:
    MailItem(
        const ContactActivityInternalInterface& parent,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const ContactActivityRowID& rowID,
        const ContactActivitySortKey& sortKey,
        CustomData& custom) noexcept;
    MailItem() = delete;
    MailItem(const MailItem&) = delete;
    MailItem(MailItem&&) = delete;
    auto operator=(const MailItem&) -> MailItem& = delete;
    auto operator=(MailItem&&) -> MailItem& = delete;

    ~MailItem() final;
};
}  // namespace opentxs::ui::implementation

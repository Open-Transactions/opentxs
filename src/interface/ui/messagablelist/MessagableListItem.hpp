// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "interface/ui/contactlist/ContactListItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace ui
{
class MessagableListItem;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
class MessagableListItem final : public implementation::ContactListItem
{
public:
    MessagableListItem(
        const ContactListInternalInterface& parent,
        const api::session::Client& api,
        const ContactListRowID& rowID,
        const ContactListSortKey& key) noexcept;
    ~MessagableListItem() final = default;

private:
    using ot_super = implementation::ContactListItem;

    auto calculate_section(const Lock& lock) const noexcept
        -> UnallocatedCString final
    {
        return translate_section(lock);
    }

    MessagableListItem() = delete;
    MessagableListItem(const MessagableListItem&) = delete;
    MessagableListItem(MessagableListItem&&) = delete;
    auto operator=(const MessagableListItem&) -> MessagableListItem& = delete;
    auto operator=(MessagableListItem&&) -> MessagableListItem& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::MessagableListItem>;

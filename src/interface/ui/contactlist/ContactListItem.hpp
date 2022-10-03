// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iosfwd>

#include "interface/ui/base/Row.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/interface/ui/ContactListItem.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

class QVariant;

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
class ContactListItem;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using ContactListItemRow =
    Row<ContactListRowInternal, ContactListInternalInterface, ContactListRowID>;

class ContactListItem : public ContactListItemRow
{
public:
    const api::session::Client& api_;

    auto ContactID() const noexcept -> UnallocatedCString final;
    auto DisplayName() const noexcept -> UnallocatedCString final;
    auto ImageURI() const noexcept -> UnallocatedCString final;
    auto Section() const noexcept -> UnallocatedCString final;

    auto init_contact_list() noexcept -> void;

    ContactListItem(
        const ContactListInternalInterface& parent,
        const api::session::Client& api,
        const ContactListRowID& rowID,
        const ContactListSortKey& key) noexcept;
    ContactListItem() = delete;
    ContactListItem(const ContactListItem&) = delete;
    ContactListItem(ContactListItem&&) = delete;
    auto operator=(const ContactListItem&) -> ContactListItem& = delete;
    auto operator=(ContactListItem&&) -> ContactListItem& = delete;

    ~ContactListItem() override = default;

protected:
    ContactListSortKey key_;

    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void override;
    auto translate_section(const Lock&) const noexcept -> UnallocatedCString;

    using ContactListItemRow::reindex;
    auto reindex(const ContactListSortKey&, CustomData&) noexcept
        -> bool override;
    virtual auto reindex(
        const Lock&,
        const ContactListSortKey&,
        CustomData&) noexcept -> bool;

private:
    UnallocatedCString section_;

    auto calculate_section() const noexcept -> UnallocatedCString;
    virtual auto calculate_section(const Lock&) const noexcept
        -> UnallocatedCString;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::ContactListItem>;

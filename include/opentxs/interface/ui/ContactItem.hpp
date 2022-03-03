// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "ListRow.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
class ContactItem;
}  // namespace ui

using OTUIContactItem = SharedPimpl<ui::ContactItem>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT ContactItem : virtual public ListRow
{
public:
    virtual auto ClaimID() const noexcept -> UnallocatedCString = 0;
    virtual auto IsActive() const noexcept -> bool = 0;
    virtual auto IsPrimary() const noexcept -> bool = 0;
    virtual auto Value() const noexcept -> UnallocatedCString = 0;

    ~ContactItem() override = default;

protected:
    ContactItem() noexcept = default;

private:
    ContactItem(const ContactItem&) = delete;
    ContactItem(ContactItem&&) = delete;
    auto operator=(const ContactItem&) -> ContactItem& = delete;
    auto operator=(ContactItem&&) -> ContactItem& = delete;
};
}  // namespace opentxs::ui

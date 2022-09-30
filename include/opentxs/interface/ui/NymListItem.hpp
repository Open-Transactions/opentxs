// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ListRow.hpp"
#include "opentxs/Export.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class NymListItem;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
 This model describes a single Nym from the NymList.
 */
class OPENTXS_EXPORT NymListItem : virtual public ListRow
{
public:
    /// Returns the display name for this Nym.
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    /// Returns the NymID for this Nym.
    virtual auto NymID() const noexcept -> UnallocatedCString = 0;

    NymListItem(const NymListItem&) = delete;
    NymListItem(NymListItem&&) = delete;
    auto operator=(const NymListItem&) -> NymListItem& = delete;
    auto operator=(NymListItem&&) -> NymListItem& = delete;

    ~NymListItem() override = default;

protected:
    NymListItem() noexcept = default;
};
}  // namespace opentxs::ui

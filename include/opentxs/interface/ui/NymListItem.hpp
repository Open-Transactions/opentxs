// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "ListRow.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
class NymListItem;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT NymListItem : virtual public ListRow
{
public:
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    virtual auto NymID() const noexcept -> UnallocatedCString = 0;

    ~NymListItem() override = default;

protected:
    NymListItem() noexcept = default;

private:
    NymListItem(const NymListItem&) = delete;
    NymListItem(NymListItem&&) = delete;
    auto operator=(const NymListItem&) -> NymListItem& = delete;
    auto operator=(NymListItem&&) -> NymListItem& = delete;
};
}  // namespace opentxs::ui

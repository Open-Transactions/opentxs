// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/interface/ui/List.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
class NymList;
class NymListItem;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT NymList : virtual public List
{
public:
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::NymListItem> = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::NymListItem> = 0;

    NymList(const NymList&) = delete;
    NymList(NymList&&) = delete;
    auto operator=(const NymList&) -> NymList& = delete;
    auto operator=(NymList&&) -> NymList& = delete;

    ~NymList() override = default;

protected:
    NymList() noexcept = default;
};
}  // namespace opentxs::ui

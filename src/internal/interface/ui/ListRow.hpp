// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/interface/ui/Widget.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::ui
{
class ListRow : virtual public Widget
{
public:
    virtual auto Last() const noexcept -> bool = 0;
    virtual auto Valid() const noexcept -> bool = 0;

    ListRow(const ListRow&) = delete;
    ListRow(ListRow&&) = delete;
    auto operator=(const ListRow&) -> ListRow& = delete;
    auto operator=(ListRow&&) -> ListRow& = delete;

    ~ListRow() override = default;

protected:
    ListRow() noexcept = default;
};
}  // namespace opentxs::ui

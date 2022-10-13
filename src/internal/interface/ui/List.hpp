// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cassert>
#include <limits>

#include "internal/interface/ui/Widget.hpp"

namespace opentxs::ui
{
class List : virtual public Widget
{
public:
    List(const List&) = delete;
    List(List&&) = delete;
    auto operator=(const List&) -> List& = delete;
    auto operator=(List&&) -> List& = delete;

    ~List() override = default;

protected:
    List() noexcept = default;
};
}  // namespace opentxs::ui

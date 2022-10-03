// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT Widget
{
public:
    virtual void ClearCallbacks() const noexcept = 0;
    virtual void SetCallback(SimpleCallback cb) const noexcept = 0;
    virtual auto WidgetID() const noexcept -> identifier::Generic = 0;

    Widget(const Widget&) = delete;
    Widget(Widget&&) = delete;
    auto operator=(const Widget&) -> Widget& = delete;
    auto operator=(Widget&&) -> Widget& = delete;

    virtual ~Widget() = default;

protected:
    Widget() noexcept = default;
};
}  // namespace opentxs::ui

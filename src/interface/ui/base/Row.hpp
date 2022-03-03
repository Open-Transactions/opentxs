// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "interface/ui/base/RowType.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/util/Lockable.hpp"

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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
template <typename InterfaceType, typename ParentType, typename IdentifierType>
class Row : public RowType<InterfaceType, ParentType, IdentifierType>,
            public Widget,
            public Lockable
{
public:
    auto AddChildren(implementation::CustomData&& data) noexcept -> void final
    {
    }

protected:
    Row(const ParentType& parent,
        const api::session::Client& api,
        const IdentifierType id,
        const bool valid) noexcept
        : RowType<InterfaceType, ParentType, IdentifierType>(parent, id, valid)
        , Widget(api, parent.WidgetID())
    {
    }
    Row() = delete;
    Row(const Row&) = delete;
    Row(Row&&) = delete;
    auto operator=(const Row&) -> Row& = delete;
    auto operator=(Row&&) -> Row& = delete;

    ~Row() override = default;
};
}  // namespace opentxs::ui::implementation

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "interface/ui/base/List.hpp"
#include "interface/ui/base/RowType.hpp"

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
template <typename ListTemplate, typename RowTemplate, typename SortKey>
class Combined : public ListTemplate, public RowTemplate
{
public:
    auto AddChildren(implementation::CustomData&& data) noexcept -> void final
    {
        ListTemplate::AddChildrenToList(std::move(data));
    }

protected:
    SortKey key_;

    Combined(
        const api::session::Client& api,
        const typename ListTemplate::ListPrimaryID::interface_type& primaryID,
        const Identifier& widgetID,
        const typename RowTemplate::RowParentType& parent,
        const typename RowTemplate::RowIdentifierType id,
        const SortKey key,
        const bool reverseSort = false) noexcept
        : ListTemplate(
              api,
              primaryID,
              widgetID,
              reverseSort,
              true,
              {},
              parent.GetQt())
        , RowTemplate(parent, id, true)
        , key_(key)
    {
    }

    ~Combined() override = default;

private:
    auto qt_parent() noexcept -> internal::Row* final { return this; }

    Combined() = delete;
    Combined(const Combined&) = delete;
    Combined(Combined&&) = delete;
    auto operator=(const Combined&) -> Combined& = delete;
    auto operator=(Combined&&) -> Combined& = delete;
};
}  // namespace opentxs::ui::implementation

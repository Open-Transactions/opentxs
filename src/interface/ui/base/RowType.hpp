// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/interface/ui/UI.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
namespace internal
{
struct Row;
}  // namespace internal
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
template <typename InterfaceType, typename ParentType, typename IdentifierType>
class RowType : virtual public InterfaceType
{
public:
    using RowInterfaceType = InterfaceType;
    using RowParentType = ParentType;
    using RowIdentifierType = IdentifierType;

    auto index() const noexcept -> std::ptrdiff_t final { return row_index_; }
    auto Last() const noexcept -> bool final { return parent_.last(row_id_); }
    auto Valid() const noexcept -> bool final { return valid_; }

protected:
    const ParentType& parent_;
    const IdentifierType row_id_;
    const std::ptrdiff_t row_index_;
    const bool valid_;

    RowType(
        const ParentType& parent,
        const IdentifierType id,
        const bool valid) noexcept
        : parent_(parent)
        , row_id_(id)
        , row_index_(internal::Row::next_index())
        , valid_(valid)
    {
    }

    ~RowType() override = default;

private:
    RowType() = delete;
    RowType(const RowType&) = delete;
    RowType(RowType&&) = delete;
    auto operator=(const RowType&) -> RowType& = delete;
    auto operator=(RowType&&) -> RowType& = delete;
};
}  // namespace opentxs::ui::implementation

// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/ui/Items.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <cstdlib>
#include <iosfwd>
#include <memory>
#include <utility>

#include "internal/interface/ui/UI.hpp"

namespace ottest
{
auto Value::operator==(const Value& rhs) const noexcept -> bool
{
    return data_ == rhs.data_;
}
auto Value::operator==(const ValueType& rhs) const noexcept -> bool
{
    return data_ == rhs;
}

auto Value::ClearCallbacks() const noexcept -> void {}

auto Value::index() const noexcept -> std::ptrdiff_t { return row_index_; }

auto Value::Last() const noexcept -> bool { return false; }

auto Value::SetCallback(opentxs::SimpleCallback) const noexcept -> void {}

auto Value::Valid() const noexcept -> bool { return false; }

auto Value::WidgetID() const noexcept -> opentxs::identifier::Generic
{
    abort();
}

auto Value::AddChildren(opentxs::ui::implementation::CustomData&& data) noexcept
    -> void
{
}

Value::Value(const char* in) noexcept
    : data_(in)
    , row_index_(next_index())
{
}

Value::Value(Value&& rhs) noexcept
    : data_(std::move(rhs.data_))
    , row_index_(rhs.row_index_)
{
}

Value::Value(const Value& rhs) noexcept
    : data_(rhs.data_)
    , row_index_(rhs.row_index_)
{
}
}  // namespace ottest

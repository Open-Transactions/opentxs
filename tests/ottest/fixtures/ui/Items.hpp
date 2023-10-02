// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdlib>
#include <functional>
#include <iosfwd>
#include <memory>
#include <optional>
#include <utility>

#include "interface/ui/base/Items.hpp"
#include "internal/interface/ui/UI.hpp"

namespace ot = opentxs;

namespace ottest
{
using Key = ot::UnallocatedCString;
using ValueType = ot::UnallocatedCString;
using ID = int;
using Active = ot::UnallocatedVector<ID>;

struct OPENTXS_EXPORT Value final : public opentxs::ui::internal::Row {
    ValueType data_;

    auto operator==(const Value& rhs) const noexcept -> bool;
    auto operator==(const ValueType& rhs) const noexcept -> bool;

    auto ClearCallbacks() const noexcept -> void final;
    auto index() const noexcept -> std::ptrdiff_t final;
    auto Last() const noexcept -> bool final;
    auto SetCallback(opentxs::SimpleCallback) const noexcept -> void final;
    auto Valid() const noexcept -> bool final;
    auto WidgetID() const noexcept -> opentxs::identifier::Generic final;

    auto AddChildren(opentxs::ui::implementation::CustomData&& data) noexcept
        -> void final;

    Value(const char* in) noexcept;
    Value(Value&& rhs) noexcept;
    Value(const Value& rhs) noexcept;

private:
    const std::ptrdiff_t row_index_;
};

using Type =
    opentxs::ui::implementation::ListItems<ID, Key, std::shared_ptr<Value>>;

struct OPENTXS_EXPORT Data {
    Key key_;
    ID id_;
    Value value_;
};

using Vector = ot::UnallocatedVector<Data>;
}  // namespace ottest

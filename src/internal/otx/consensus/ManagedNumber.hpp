// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/util/Numbers.hpp"

namespace opentxs::otx::context
{
class ManagedNumber;

auto operator<(const ManagedNumber& lhs, const ManagedNumber& rhs) noexcept
    -> bool;

class ManagedNumber
{
public:
    class Imp;

    auto SetSuccess(const bool value = true) const -> void;
    auto Valid() const -> bool;
    auto Value() const -> TransactionNumber;

    auto swap(ManagedNumber& rhs) noexcept -> void;

    ManagedNumber(Imp* imp) noexcept;
    ManagedNumber(const ManagedNumber&) = delete;
    ManagedNumber(ManagedNumber&& rhs) noexcept;
    auto operator=(const ManagedNumber&) -> ManagedNumber& = delete;
    auto operator=(ManagedNumber&&) noexcept -> ManagedNumber&;

    ~ManagedNumber();

private:
    Imp* imp_;
    friend auto operator<(const ManagedNumber&, const ManagedNumber&) noexcept
        -> bool;
};
}  // namespace opentxs::otx::context

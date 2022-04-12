// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/util/Numbers.hpp"

namespace opentxs::otx::context
{
class ManagedNumber;

OPENTXS_EXPORT auto operator<(
    const ManagedNumber& lhs,
    const ManagedNumber& rhs) noexcept -> bool;

class OPENTXS_EXPORT ManagedNumber
{
public:
    class Imp;
    void SetSuccess(const bool value = true) const;
    auto Valid() const -> bool;
    auto Value() const -> TransactionNumber;

    virtual auto swap(ManagedNumber& rhs) noexcept -> void;

    ManagedNumber(Imp* imp) noexcept;
    ManagedNumber(const ManagedNumber&) = delete;
    ManagedNumber(ManagedNumber&& rhs) noexcept;
    auto operator=(const ManagedNumber&) -> ManagedNumber& = delete;
    auto operator=(ManagedNumber&&) noexcept -> ManagedNumber&;

    virtual ~ManagedNumber();

private:
    Imp* imp_;
    friend auto operator<(const ManagedNumber&, const ManagedNumber&) noexcept
        -> bool;
};
}  // namespace opentxs::otx::context

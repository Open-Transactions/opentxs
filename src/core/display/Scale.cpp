// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "opentxs/core/display/Scale.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "core/display/Scale.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
namespace opentxs::display
{
Scale::Scale(
    std::string_view prefix,
    std::string_view suffix,
    Vector<Ratio>&& ratios,
    const OptionalInt defaultMinDecimals,
    const OptionalInt defaultMaxDecimals) noexcept
    // NOLINTBEGIN(clang-analyzer-core.StackAddressEscape)
    : imp_(std::make_unique<Imp>(
               prefix,
               suffix,
               std::move(ratios),
               defaultMinDecimals,
               defaultMaxDecimals)
               .release())
// NOLINTEND(clang-analyzer-core.StackAddressEscape)
{
    OT_ASSERT(imp_);
}

Scale::Scale() noexcept
    : Scale(std::make_unique<Imp>().release())
{
}

Scale::Scale(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp);
}

Scale::Scale(const Scale& rhs) noexcept
    : imp_(std::make_unique<Imp>(*rhs.imp_).release())
{
    OT_ASSERT(imp_);
}

Scale::Scale(Scale&& rhs) noexcept
    : Scale()
{
    swap(rhs);
}

auto Scale::DefaultMinDecimals() const noexcept -> OptionalInt
{
    return imp_->default_min_;
}

auto Scale::DefaultMaxDecimals() const noexcept -> OptionalInt
{
    return imp_->default_max_;
}

auto Scale::Format(
    const Amount& amount,
    const OptionalInt minDecimals,
    const OptionalInt maxDecimals) const noexcept(false) -> UnallocatedCString
{
    return imp_->format(amount, minDecimals, maxDecimals);
}

auto Scale::Import(const std::string_view formatted) const noexcept(false)
    -> Amount
{
    return imp_->Import(formatted);
}

auto Scale::MaximumDecimals() const noexcept -> std::uint8_t
{
    return imp_->MaximumDecimals();
}

auto Scale::Prefix() const noexcept -> std::string_view
{
    return imp_->prefix_;
}

auto Scale::Ratios() const noexcept -> const Vector<Ratio>&
{
    return imp_->ratios_;
}

auto Scale::Suffix() const noexcept -> std::string_view
{
    return imp_->suffix_;
}

auto Scale::swap(Scale& rhs) noexcept -> void { std::swap(imp_, rhs.imp_); }

Scale::~Scale()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::display
// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

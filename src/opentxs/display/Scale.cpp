// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/display/Scale.hpp"  // IWYU pragma: associated

#include <memory>
#include <sstream>
#include <utility>

#include "opentxs/core/Amount.hpp"
#include "opentxs/display/ScalePrivate.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
namespace opentxs::display
{
Scale::Scale(ScalePrivate* imp) noexcept
    : imp_(imp)
{
    assert_false(nullptr == imp);
}

Scale::Scale() noexcept
    : Scale(std::make_unique<ScalePrivate>().release())
{
}

Scale::Scale(const Scale& rhs) noexcept
    : Scale(std::make_unique<ScalePrivate>(*rhs.imp_).release())
{
}

Scale::Scale(Scale&& rhs) noexcept
    : Scale(std::exchange(rhs.imp_, nullptr))
{
}

auto Scale::DefaultMinDecimals() const noexcept -> DecimalPlaces
{
    return imp_->DefaultMinDecimals();
}

auto Scale::DefaultMaxDecimals() const noexcept -> DecimalPlaces
{
    return imp_->DefaultMaxDecimals();
}

auto Scale::Format(
    const Amount& amount,
    const DecimalPlaces minDecimals,
    const DecimalPlaces maxDecimals) const noexcept -> UnallocatedCString
{
    return imp_->Format(amount, minDecimals, maxDecimals, {}).str();
}

auto Scale::Format(
    const Amount& amount,
    alloc::Strategy alloc,
    const DecimalPlaces minDecimals,
    const DecimalPlaces maxDecimals) const noexcept -> CString
{
    return CString{
        imp_->Format(amount, minDecimals, maxDecimals, alloc).str(),
        alloc.result_};
}

auto Scale::Import(const std::string_view formatted) const noexcept
    -> std::optional<Amount>
{
    return imp_->Import(formatted);
}

auto Scale::MaximumDecimals() const noexcept -> DecimalCount
{
    return imp_->MaximumDecimals();
}

auto Scale::Prefix() const noexcept -> std::string_view
{
    return imp_->Prefix();
}

auto Scale::Ratios() const noexcept -> std::span<const Ratio>
{
    return imp_->Ratios();
}

auto Scale::Suffix() const noexcept -> std::string_view
{
    return imp_->Suffix();
}

auto Scale::swap(Scale& rhs) noexcept -> void { std::swap(imp_, rhs.imp_); }

Scale::~Scale()
{
    if ((nullptr != imp_) && (imp_->RuntimeAllocated())) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::display
// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

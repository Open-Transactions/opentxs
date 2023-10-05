// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/display/Definition.hpp"  // IWYU pragma: associated

#include <memory>
#include <optional>
#include <utility>

#include "core/display/DefinitionPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Amount.hpp"  // IWYU pragma: keep
#include "opentxs/core/display/Scale.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::display
{
Definition::Definition(DefinitionPrivate* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(imp_);
}

Definition::Definition() noexcept
    : Definition(std::make_unique<DefinitionPrivate>().release())
{
}

Definition::Definition(const Definition& rhs) noexcept
    : imp_(std::make_unique<DefinitionPrivate>(*rhs.imp_).release())
{
    OT_ASSERT(imp_);
}

Definition::Definition(Definition&& rhs) noexcept
    : Definition(std::exchange(rhs.imp_, nullptr))
{
}

auto Definition::operator=(const Definition& rhs) noexcept -> Definition&
{
    if (&rhs != this) {
        imp_ = std::make_unique<DefinitionPrivate>(*rhs.imp_).release();
    }

    return *this;
}

auto Definition::operator=(Definition&& rhs) noexcept -> Definition&
{
    if (&rhs != this) { std::swap(imp_, rhs.imp_); }

    return *this;
}

auto Definition::AtomicScale() const noexcept -> ScaleIndex
{
    return imp_->AtomicScale();
}

auto Definition::DefaultScale() const noexcept -> ScaleIndex
{
    return imp_->DefaultScale();
}

auto Definition::Format(
    const Amount& amount,
    const SpecifiedScale scale,
    const DecimalPlaces minDecimals,
    const DecimalPlaces maxDecimals) const noexcept -> UnallocatedCString
{
    return Scale(scale.value_or(imp_->DefaultScale()))
        .Format(amount, minDecimals, maxDecimals);
}

auto Definition::Format(
    const Amount& amount,
    alloc::Strategy alloc,
    const SpecifiedScale scale,
    const DecimalPlaces minDecimals,
    const DecimalPlaces maxDecimals) const noexcept -> CString
{
    return Scale(scale.value_or(imp_->DefaultScale()))
        .Format(amount, alloc, minDecimals, maxDecimals);
}

auto Definition::Import(
    const std::string_view formatted,
    const SpecifiedScale scale) const noexcept -> std::optional<Amount>
{
    return imp_->Import(formatted, scale.value_or(imp_->DefaultScale()));
}

auto Definition::Scale(ScaleIndex scale) const noexcept -> display::Scale
{
    return imp_->Scale(scale);
}

auto Definition::ScaleCount() const noexcept -> ScaleIndex
{
    return imp_->ScaleCount();
}

auto Definition::ScaleName(ScaleIndex scale) const noexcept -> std::string_view
{
    return imp_->ScaleName(scale);
}

auto Definition::ShortName() const noexcept -> std::string_view
{
    return imp_->ShortName();
}

auto Definition::swap(Definition& rhs) noexcept -> void
{
    std::swap(imp_, rhs.imp_);
}

Definition::~Definition()
{
    if ((nullptr != imp_) && imp_->RuntimeAllocated()) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::display

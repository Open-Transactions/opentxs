// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/util/PasswordPrompt.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/LogMacros.hpp"
#include "internal/util/PMR.hpp"
#include "util/PasswordPromptPrivate.hpp"

namespace opentxs
{
PasswordPrompt::PasswordPrompt(PasswordPromptPrivate* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

PasswordPrompt::PasswordPrompt(PasswordPrompt&& rhs) noexcept
    : PasswordPrompt(std::exchange(rhs.imp_, nullptr))
{
}

PasswordPrompt::PasswordPrompt(
    PasswordPrompt&& rhs,
    allocator_type alloc) noexcept
    : imp_(nullptr)
{
    pmr::move_construct(imp_, rhs.imp_, alloc);
}

auto PasswordPrompt::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto PasswordPrompt::get_deleter() noexcept -> delete_function
{
    return pmr::make_deleter(imp_);
}

auto PasswordPrompt::GetDisplayString() const noexcept -> std::string_view
{
    return imp_->GetDisplayString();
}

auto PasswordPrompt::Internal() const noexcept
    -> const internal::PasswordPrompt&
{
    return *imp_;
}

auto PasswordPrompt::Internal() noexcept -> internal::PasswordPrompt&
{
    return *imp_;
}

PasswordPrompt::~PasswordPrompt() { pmr::destroy(imp_); }
}  // namespace opentxs

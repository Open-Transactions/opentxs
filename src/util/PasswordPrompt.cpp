// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/util/PasswordPrompt.hpp"  // IWYU pragma: associated

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/util/Allocator.hpp"
#include "util/PasswordPromptPrivate.hpp"

namespace opentxs
{
PasswordPrompt::PasswordPrompt(PasswordPromptPrivate* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

PasswordPrompt::PasswordPrompt(PasswordPrompt&& rhs) noexcept
    : PasswordPrompt(rhs.imp_)
{
    rhs.imp_ = nullptr;
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

PasswordPrompt::~PasswordPrompt()
{
    if (nullptr != imp_) {
        // TODO c++20
        auto alloc = alloc::PMR<PasswordPromptPrivate>{imp_->get_allocator()};
        alloc.destroy(imp_);
        alloc.deallocate(imp_, 1_uz);
        imp_ = nullptr;
    }
}
}  // namespace opentxs

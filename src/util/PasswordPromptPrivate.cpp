// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "util/PasswordPromptPrivate.hpp"  // IWYU pragma: associated

#include "internal/util/P0330.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Secret.hpp"

namespace opentxs
{
PasswordPromptPrivate::PasswordPromptPrivate(
    const api::Session& api,
    std::string_view display,
    allocator_type alloc) noexcept
    : api_(api)
    , display_(display, alloc)
    , password_(api.Factory().Secret(0_uz))
{
}

auto PasswordPromptPrivate::ClearPassword() noexcept -> bool
{
    password_.clear();

    return true;
}

auto PasswordPromptPrivate::get_allocator() const noexcept -> allocator_type
{
    return display_.get_allocator();
}

auto PasswordPromptPrivate::GetDisplayString() const noexcept
    -> std::string_view
{
    return display_;
}

auto PasswordPromptPrivate::Password() const noexcept -> const Secret&
{
    return password_;
}

auto PasswordPromptPrivate::SetPassword(const Secret& password) noexcept -> bool
{
    password_ = password;

    return true;
}

PasswordPromptPrivate::~PasswordPromptPrivate() = default;
}  // namespace opentxs

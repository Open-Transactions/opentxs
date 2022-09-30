// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "opentxs/util/PasswordPrompt.hpp"  // IWYU pragma: associated

#include <memory>

#include "2_Factory.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Secret.hpp"

namespace opentxs
{
auto Factory::PasswordPrompt(
    const api::Session& api,
    const UnallocatedCString& text) -> opentxs::PasswordPrompt*
{
    return new opentxs::PasswordPrompt(api, text);
}

PasswordPrompt::PasswordPrompt(
    const api::Session& api,
    const UnallocatedCString& display) noexcept
    : api_(api)
    , display_(display)
    , password_(api.Factory().Secret(0))
{
}

PasswordPrompt::PasswordPrompt(const PasswordPrompt& rhs) noexcept
    : api_(rhs.api_)
    , display_(rhs.GetDisplayString())
    , password_(rhs.password_)
{
}

auto PasswordPrompt::ClearPassword() -> bool
{
    password_->clear();

    return true;
}

auto PasswordPrompt::GetDisplayString() const -> const char*
{
    return display_.c_str();
}

auto PasswordPrompt::SetPassword(const Secret& password) -> bool
{
    password_ = password;

    return true;
}

auto PasswordPrompt::Password() const -> const Secret& { return password_; }

PasswordPrompt::~PasswordPrompt() = default;
}  // namespace opentxs

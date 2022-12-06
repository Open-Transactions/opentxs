// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/util/PasswordCaller.hpp"  // IWYU pragma: associated

#include "internal/util/LogMacros.hpp"
#include "opentxs/util/PasswordCallback.hpp"
#include "opentxs/util/PasswordPrompt.hpp"

namespace opentxs
{
PasswordCaller::PasswordCaller()
    : callback_(nullptr)
{
}

auto PasswordCaller::AskOnce(
    const PasswordPrompt& prompt,
    Secret& output,
    std::string_view key) noexcept -> void
{
    OT_ASSERT(callback_);

    callback_->runOne(output, prompt.GetDisplayString(), key);
}

auto PasswordCaller::AskTwice(
    const PasswordPrompt& prompt,
    Secret& output,
    std::string_view key) noexcept -> void
{
    OT_ASSERT(callback_);

    callback_->runTwo(output, prompt.GetDisplayString(), key);
}

auto PasswordCaller::HaveCallback() const noexcept -> bool
{
    return nullptr != callback_;
}

auto PasswordCaller::SetCallback(PasswordCallback* callback) noexcept -> void
{
    callback_ = callback;
}

PasswordCaller::~PasswordCaller() { callback_ = nullptr; }
}  // namespace opentxs

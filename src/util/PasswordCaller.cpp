// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
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

void PasswordCaller::AskOnce(
    const PasswordPrompt& prompt,
    Secret& output,
    const UnallocatedCString& key)
{
    OT_ASSERT(callback_);

    callback_->runOne(prompt.GetDisplayString(), output, key);
}

void PasswordCaller::AskTwice(
    const PasswordPrompt& prompt,
    Secret& output,
    const UnallocatedCString& key)
{
    OT_ASSERT(callback_);

    callback_->runTwo(prompt.GetDisplayString(), output, key);
}

auto PasswordCaller::HaveCallback() const -> bool
{
    return nullptr != callback_;
}

void PasswordCaller::SetCallback(PasswordCallback* callback)
{
    callback_ = callback;
}

PasswordCaller::~PasswordCaller() { callback_ = nullptr; }
}  // namespace opentxs

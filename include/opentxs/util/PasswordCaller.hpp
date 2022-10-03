// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/Export.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class PasswordCallback;
class PasswordPrompt;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class OPENTXS_EXPORT PasswordCaller
{
public:
    auto HaveCallback() const noexcept -> bool;

    auto AskOnce(
        const PasswordPrompt& prompt,
        Secret& output,
        std::string_view key) noexcept -> void;
    auto AskTwice(
        const PasswordPrompt& prompt,
        Secret& output,
        std::string_view key) noexcept -> void;
    auto SetCallback(PasswordCallback* callback) noexcept -> void;

    PasswordCaller();
    PasswordCaller(const PasswordCaller&) = delete;
    PasswordCaller(PasswordCaller&&) = delete;
    auto operator=(const PasswordCaller&) -> PasswordCaller& = delete;
    auto operator=(PasswordCaller&&) -> PasswordCaller& = delete;

    ~PasswordCaller();

private:
    PasswordCallback* callback_;
};
}  // namespace opentxs

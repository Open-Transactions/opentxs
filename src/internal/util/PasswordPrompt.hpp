// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::internal
{
class PasswordPrompt
{
public:
    virtual auto API() const noexcept -> const api::Session& = 0;
    virtual auto Password() const noexcept -> const Secret& = 0;

    virtual auto ClearPassword() noexcept -> bool = 0;
    virtual auto SetPassword(const Secret& password) noexcept -> bool = 0;

    PasswordPrompt(const PasswordPrompt&) = delete;
    PasswordPrompt(PasswordPrompt&&) noexcept;
    auto operator=(const PasswordPrompt&) -> const PasswordPrompt& = delete;
    auto operator=(const PasswordPrompt&&) -> const PasswordPrompt& = delete;

    virtual ~PasswordPrompt() = default;

protected:
    PasswordPrompt() = default;
};
}  // namespace opentxs::internal

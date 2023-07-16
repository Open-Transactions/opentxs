// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "internal/util/PMR.hpp"
#include "internal/util/PasswordPrompt.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
class PasswordPromptPrivate final : public internal::PasswordPrompt,
                                    public opentxs::Allocated
{
public:
    auto API() const noexcept -> const api::Session& final { return api_; }
    auto clone(allocator_type alloc) const noexcept -> PasswordPromptPrivate*
    {
        return pmr::clone(this, {alloc});
    }
    auto get_allocator() const noexcept -> allocator_type final;
    auto GetDisplayString() const noexcept -> std::string_view;
    auto Password() const noexcept -> const Secret& final;

    auto ClearPassword() noexcept -> bool final;
    auto get_deleter() noexcept -> delete_function final
    {
        return make_deleter(this);
    }
    auto SetPassword(const Secret& password) noexcept -> bool final;

    PasswordPromptPrivate(
        const api::Session& api,
        std::string_view display,
        allocator_type alloc) noexcept;
    PasswordPromptPrivate() = delete;
    PasswordPromptPrivate(
        const PasswordPromptPrivate& rhs,
        allocator_type alloc) noexcept;
    PasswordPromptPrivate(const PasswordPromptPrivate&&) = delete;
    auto operator=(const PasswordPromptPrivate&)
        -> const PasswordPromptPrivate& = delete;
    auto operator=(const PasswordPromptPrivate&&)
        -> const PasswordPromptPrivate& = delete;

    ~PasswordPromptPrivate() final;

private:
    const api::Session& api_;
    CString display_;
    Secret password_;
};
}  // namespace opentxs

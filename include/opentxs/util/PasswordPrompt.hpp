// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace internal
{
class PasswordPrompt;
}  // namespace internal

class PasswordPromptPrivate;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class OPENTXS_EXPORT PasswordPrompt final : public Allocated
{
public:
    auto get_allocator() const noexcept -> allocator_type final;
    auto GetDisplayString() const noexcept -> std::string_view;
    OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::PasswordPrompt&;

    auto get_deleter() noexcept -> delete_function final;
    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::PasswordPrompt&;

    OPENTXS_NO_EXPORT PasswordPrompt(PasswordPromptPrivate* imp) noexcept;
    PasswordPrompt() = delete;
    PasswordPrompt(const PasswordPrompt&) = delete;
    PasswordPrompt(PasswordPrompt&& rhs) noexcept;
    PasswordPrompt(PasswordPrompt&& rhs, allocator_type alloc) noexcept;
    auto operator=(const PasswordPrompt&) -> const PasswordPrompt& = delete;
    auto operator=(const PasswordPrompt&&) -> const PasswordPrompt& = delete;

    ~PasswordPrompt() final;

private:
    PasswordPromptPrivate* imp_;
};
}  // namespace opentxs

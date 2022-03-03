// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/session/UI.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::internal
{
class UI : virtual public opentxs::api::session::UI
{
public:
    virtual auto ActivateUICallback(const Identifier& widget) const noexcept
        -> void = 0;
    virtual auto ClearUICallbacks(const Identifier& widget) const noexcept
        -> void = 0;
    auto Internal() const noexcept -> const internal::UI& final
    {
        return *this;
    }
    virtual auto RegisterUICallback(
        const Identifier& widget,
        const SimpleCallback& cb) const noexcept -> void = 0;

    virtual auto Init() noexcept -> void = 0;
    auto Internal() noexcept -> internal::UI& final { return *this; }
    virtual auto Shutdown() noexcept -> void = 0;

    ~UI() override = default;

protected:
    UI() = default;

private:
    UI(const UI&) = delete;
    UI(UI&&) = delete;
    UI& operator=(const UI&) = delete;
    UI& operator=(UI&&) = delete;
};
}  // namespace opentxs::api::session::internal

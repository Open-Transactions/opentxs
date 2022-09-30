// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/session/UI.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::internal
{
class UI : virtual public opentxs::api::session::UI
{
public:
    virtual auto ActivateUICallback(
        const identifier::Generic& widget) const noexcept -> void = 0;
    virtual auto ClearUICallbacks(
        const identifier::Generic& widget) const noexcept -> void = 0;
    auto Internal() const noexcept -> const internal::UI& final
    {
        return *this;
    }
    virtual auto RegisterUICallback(
        const identifier::Generic& widget,
        const SimpleCallback& cb) const noexcept -> void = 0;

    virtual auto Init() noexcept -> void = 0;
    auto Internal() noexcept -> internal::UI& final { return *this; }
    virtual auto Shutdown() noexcept -> void = 0;

    UI(const UI&) = delete;
    UI(UI&&) = delete;
    auto operator=(const UI&) -> UI& = delete;
    auto operator=(UI&&) -> UI& = delete;

    ~UI() override = default;

protected:
    UI() = default;
};
}  // namespace opentxs::api::session::internal

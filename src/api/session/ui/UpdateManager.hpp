// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::ui
{
struct UpdateManager {
    auto ActivateUICallback(const Identifier& widget) const noexcept -> void;
    auto ClearUICallbacks(const Identifier& widget) const noexcept -> void;
    auto RegisterUICallback(const Identifier& widget, const SimpleCallback& cb)
        const noexcept -> void;

    UpdateManager(const api::session::Client& api) noexcept;

    ~UpdateManager();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::api::session::ui

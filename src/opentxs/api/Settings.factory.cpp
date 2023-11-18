// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/internal.factory.hpp"  // IWYU pragma: associated

#include "opentxs/api/SettingsPrivate.hpp"

namespace opentxs::factory
{
auto Settings(
    const api::internal::Paths& legacy,
    const std::filesystem::path& path) noexcept -> api::internal::Settings*
{
    using ReturnType = api::SettingsPrivate;

    return std::make_unique<ReturnType>(legacy, path).release();
}
}  // namespace opentxs::factory

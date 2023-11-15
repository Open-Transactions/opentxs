// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/Legacy.hpp"  // IWYU pragma: associated

namespace opentxs::api::imp
{
auto Legacy::get_home_platform() noexcept -> UnallocatedCString
{
    auto home = UnallocatedCString{getenv("USERPROFILE")};

    if (false == home.empty()) { return std::move(home); }

    const auto drive = UnallocatedCString{getenv("HOMEDRIVE")};
    const auto path = UnallocatedCString{getenv("HOMEPATH")};

    if ((false == drive.empty()) && (false == path.empty())) {

        return drive + path;
    }

    return {};
}

auto Legacy::get_suffix() noexcept -> fs::path
{
    return get_suffix("OpenTransactions");
}

auto Legacy::prepend() noexcept -> UnallocatedCString { return {}; }

auto Legacy::use_dot() noexcept -> bool { return false; }
}  // namespace opentxs::api::imp

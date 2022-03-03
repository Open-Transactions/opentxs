// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/util/Container.hpp"
#include "opentxs/util/storage/Driver.hpp"

namespace opentxs::storage
{
class Plugin : public virtual Driver
{
public:
    auto EmptyBucket(const bool bucket) const -> bool override = 0;
    auto LoadRoot() const -> UnallocatedCString override = 0;
    auto StoreRoot(const bool commit, const UnallocatedCString& hash) const
        -> bool override = 0;

    ~Plugin() override = default;

protected:
    Plugin() = default;

private:
    Plugin(const Plugin&) = delete;
    Plugin(Plugin&&) = delete;
    auto operator=(const Plugin&) -> Plugin& = delete;
    auto operator=(Plugin&&) -> Plugin& = delete;
};
}  // namespace opentxs::storage

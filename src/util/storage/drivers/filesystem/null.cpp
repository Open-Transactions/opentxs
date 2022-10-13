// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated

#include "internal/util/storage/drivers/Factory.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/storage/Plugin.hpp"

namespace opentxs::factory
{
auto StorageFSArchive(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Storage& parent,
    const storage::Config& config,
    const Flag& bucket,
    const UnallocatedCString& folder,
    crypto::symmetric::Key& key) noexcept -> std::unique_ptr<storage::Plugin>
{
    return {};
}

auto StorageFSGC(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Storage& parent,
    const storage::Config& config,
    const Flag& bucket) noexcept -> std::unique_ptr<storage::Plugin>
{
    return nullptr;
}
}  // namespace opentxs::factory

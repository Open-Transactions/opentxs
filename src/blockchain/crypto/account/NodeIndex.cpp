// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/crypto/account/NodeIndex.hpp"  // IWYU pragma: associated

#include <functional>
#include <memory>
#include <utility>

#include "opentxs/core/Data.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::crypto::account
{
NodeIndex::NodeIndex() noexcept
    : map_()
{
}

auto NodeIndex::Add(
    const api::Crypto& api,
    const identifier::Account& id,
    crypto::Subaccount* node) noexcept -> bool
{
    if (nullptr == node) {
        LogError()()("invalid subaccount").Flush();

        return false;
    }

    auto handle = map_.lock();
    auto& map = *handle;

    if (map.contains(id)) {
        LogError()()("subaccount ")(id, api)(" already exists").Flush();

        return false;
    }

    return map.try_emplace(id, node).second;
}

auto NodeIndex::Find(const identifier::Account& id) const noexcept
    -> crypto::Subaccount*
{
    auto handle = map_.lock_shared();
    const auto& map = *handle;

    if (auto i = map.find(id); map.end() != i) {

        return i->second;
    } else {

        return nullptr;
    }
}
}  // namespace opentxs::blockchain::crypto::account

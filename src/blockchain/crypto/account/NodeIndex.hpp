// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_shared_guarded.h>
#include <shared_mutex>

#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace blockchain
{
namespace crypto
{
class Subaccount;
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::account
{
class NodeIndex
{
public:
    auto Find(const identifier::Account& id) const noexcept
        -> crypto::Subaccount*;

    [[nodiscard]] auto Add(
        const api::Crypto& api,
        const identifier::Account& id,
        crypto::Subaccount* node) noexcept -> bool;

    NodeIndex() noexcept;

private:
    using Map = opentxs::Map<identifier::Account, crypto::Subaccount*>;
    using Guarded = libguarded::shared_guarded<Map, std::shared_mutex>;

    Guarded map_;
};
}  // namespace opentxs::blockchain::crypto::account

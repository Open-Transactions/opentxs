// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <memory>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::base
{
class SyncClient
{
public:
    auto Endpoint() const noexcept -> const UnallocatedCString&;

    SyncClient(const api::Session& api, const Type chain) noexcept;

    ~SyncClient();

private:
    class Imp;

    Imp* imp_;

    SyncClient() = delete;
    SyncClient(const SyncClient&) = delete;
    SyncClient(SyncClient&&) = delete;
    auto operator=(const SyncClient&) -> SyncClient& = delete;
    auto operator=(SyncClient&&) -> SyncClient& = delete;
};
}  // namespace opentxs::blockchain::node::base

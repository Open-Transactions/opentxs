// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/reply/storesecret/StoreSecretPrivate.hpp"  // IWYU pragma: associated

#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep

namespace opentxs::contract::peer::reply
{
StoreSecretPrivate::StoreSecretPrivate(allocator_type alloc) noexcept
    : ReplyPrivate(alloc)
{
}

StoreSecretPrivate::StoreSecretPrivate(
    const StoreSecretPrivate& rhs,
    allocator_type alloc) noexcept
    : ReplyPrivate(rhs, alloc)
{
}

auto StoreSecretPrivate::Type() const noexcept -> RequestType
{
    return RequestType::StoreSecret;
}

auto StoreSecretPrivate::Value() const noexcept -> bool { return {}; }

StoreSecretPrivate::~StoreSecretPrivate() = default;
}  // namespace opentxs::contract::peer::reply

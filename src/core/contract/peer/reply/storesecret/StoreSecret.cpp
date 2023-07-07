// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/contract/peer/reply/StoreSecret.hpp"  // IWYU pragma: associated

#include <utility>

#include "core/contract/peer/reply/base/ReplyPrivate.hpp"
#include "core/contract/peer/reply/storesecret/StoreSecretPrivate.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::reply
{
StoreSecret::StoreSecret(ReplyPrivate* imp) noexcept
    : Reply(imp)
{
}

StoreSecret::StoreSecret(allocator_type alloc) noexcept
    : StoreSecret(StoreSecretPrivate::Blank(alloc))
{
}

StoreSecret::StoreSecret(const StoreSecret& rhs, allocator_type alloc) noexcept
    : Reply(rhs, alloc)
{
}

StoreSecret::StoreSecret(StoreSecret&& rhs) noexcept
    : Reply(std::move(rhs))
{
}

StoreSecret::StoreSecret(StoreSecret&& rhs, allocator_type alloc) noexcept
    : Reply(std::move(rhs), alloc)
{
}

auto StoreSecret::Blank() noexcept -> StoreSecret&
{
    static auto blank = StoreSecret{allocator_type{alloc::Default()}};

    return blank;
}

auto StoreSecret::IsValid() const noexcept -> bool
{
    using enum RequestType;

    return imp_->IsValid() && (Type() == StoreSecret);
}

// NOLINTBEGIN(modernize-use-equals-default)
auto StoreSecret::operator=(const StoreSecret& rhs) noexcept -> StoreSecret&
{
    Reply::operator=(rhs);

    return *this;
}

auto StoreSecret::operator=(StoreSecret&& rhs) noexcept -> StoreSecret&
{
    Reply::operator=(std::move(rhs));

    return *this;
}
// NOLINTEND(modernize-use-equals-default)

auto StoreSecret::Value() const noexcept -> bool
{
    return imp_->asStoreSecretPrivate()->Value();
}

StoreSecret::~StoreSecret() = default;
}  // namespace opentxs::contract::peer::reply

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/contract/peer/reply/StoreSecret.hpp"  // IWYU pragma: associated

#include <span>
#include <string_view>
#include <utility>

#include "core/contract/peer/request/base/RequestPrivate.hpp"
#include "core/contract/peer/request/storesecret/StoreSecretPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/contract/peer/request/StoreSecret.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::request
{
StoreSecret::StoreSecret(RequestPrivate* imp) noexcept
    : Request(imp)
{
}

StoreSecret::StoreSecret(allocator_type alloc) noexcept
    : StoreSecret(StoreSecretPrivate::Blank(alloc))
{
}

StoreSecret::StoreSecret(const StoreSecret& rhs, allocator_type alloc) noexcept
    : Request(rhs, alloc)
{
}

StoreSecret::StoreSecret(StoreSecret&& rhs) noexcept
    : Request(std::move(rhs))
{
}

StoreSecret::StoreSecret(StoreSecret&& rhs, allocator_type alloc) noexcept
    : Request(std::move(rhs), alloc)
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

auto StoreSecret::Kind() const noexcept -> SecretType
{
    return imp_->asStoreSecretPrivate()->Kind();
}

auto StoreSecret::operator=(const StoreSecret& rhs) noexcept -> StoreSecret&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_child<Request>(*this, rhs);
}

auto StoreSecret::operator=(StoreSecret&& rhs) noexcept -> StoreSecret&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_child<Request>(*this, std::move(rhs));
}

auto StoreSecret::Values() const noexcept -> std::span<const std::string_view>
{
    return imp_->asStoreSecretPrivate()->Values();
}

StoreSecret::~StoreSecret() = default;
}  // namespace opentxs::contract::peer::request

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/contract/peer/reply/Faucet.hpp"  // IWYU pragma: associated

#include <string_view>
#include <utility>

#include "core/contract/peer/request/base/RequestPrivate.hpp"
#include "core/contract/peer/request/faucet/FaucetPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/contract/peer/request/Faucet.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::request
{
Faucet::Faucet(RequestPrivate* imp) noexcept
    : Request(imp)
{
}

Faucet::Faucet(allocator_type alloc) noexcept
    : Faucet(FaucetPrivate::Blank(alloc))
{
}

Faucet::Faucet(const Faucet& rhs, allocator_type alloc) noexcept
    : Request(rhs, alloc)
{
}

Faucet::Faucet(Faucet&& rhs) noexcept
    : Request(std::move(rhs))
{
}

Faucet::Faucet(Faucet&& rhs, allocator_type alloc) noexcept
    : Request(std::move(rhs), alloc)
{
}

auto Faucet::Blank() noexcept -> Faucet&
{
    static auto blank = Faucet{allocator_type{alloc::Default()}};

    return blank;
}

auto Faucet::Currency() const noexcept -> opentxs::UnitType
{
    return imp_->asFaucetPrivate()->Currency();
}

auto Faucet::Instructions() const noexcept -> std::string_view
{
    return imp_->asFaucetPrivate()->Instructions();
}

auto Faucet::IsValid() const noexcept -> bool
{
    using enum RequestType;

    return imp_->IsValid() && (Type() == Faucet);
}

auto Faucet::operator=(const Faucet& rhs) noexcept -> Faucet&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_child<Request>(*this, rhs);
}

auto Faucet::operator=(Faucet&& rhs) noexcept -> Faucet&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_child<Request>(*this, std::move(rhs));
}

Faucet::~Faucet() = default;
}  // namespace opentxs::contract::peer::request

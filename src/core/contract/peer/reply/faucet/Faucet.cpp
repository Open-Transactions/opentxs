// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/contract/peer/reply/Faucet.hpp"  // IWYU pragma: associated

#include <utility>

#include "core/contract/peer/reply/base/ReplyPrivate.hpp"
#include "core/contract/peer/reply/faucet/FaucetPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::reply
{
Faucet::Faucet(ReplyPrivate* imp) noexcept
    : Reply(imp)
{
}

Faucet::Faucet(allocator_type alloc) noexcept
    : Faucet(FaucetPrivate::Blank(alloc))
{
}

Faucet::Faucet(const Faucet& rhs, allocator_type alloc) noexcept
    : Reply(rhs, alloc)
{
}

Faucet::Faucet(Faucet&& rhs) noexcept
    : Reply(std::move(rhs))
{
}

Faucet::Faucet(Faucet&& rhs, allocator_type alloc) noexcept
    : Reply(std::move(rhs), alloc)
{
}

auto Faucet::Accepted() const noexcept -> bool
{
    return imp_->asFaucetPrivate()->Accepted();
}

auto Faucet::Blank() noexcept -> Faucet&
{
    static auto blank = Faucet{allocator_type{alloc::Default()}};

    return blank;
}

auto Faucet::IsValid() const noexcept -> bool
{
    using enum RequestType;

    return imp_->IsValid() && (Type() == Faucet);
}

auto Faucet::operator=(const Faucet& rhs) noexcept -> Faucet&
{
    return pmr::copy_assign_child<Reply>(*this, rhs);
}

auto Faucet::operator=(Faucet&& rhs) noexcept -> Faucet&
{
    return pmr::move_assign_child<Reply>(*this, std::move(rhs));
}

auto Faucet::Transaction() const noexcept
    -> const blockchain::block::Transaction&
{
    return imp_->asFaucetPrivate()->Transaction();
}

Faucet::~Faucet() = default;
}  // namespace opentxs::contract::peer::reply

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "blockchain/p2p/peer/Address.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"

namespace opentxs::blockchain::p2p::peer
{
Address::Address(std::unique_ptr<internal::Address> address) noexcept
    : lock_()
    , address_(std::move(address))
{
    OT_ASSERT(address_);
}

auto Address::Bytes() const noexcept -> OTData
{
    Lock lock(lock_);

    return address_->Bytes();
}

auto Address::Chain() const noexcept -> blockchain::Type
{
    Lock lock(lock_);

    return address_->Chain();
}

auto Address::Display() const noexcept -> UnallocatedCString
{
    Lock lock(lock_);

    return address_->Display();
}

auto Address::ID() const noexcept -> OTIdentifier
{
    Lock lock(lock_);

    return address_->ID();
}

auto Address::Incoming() const noexcept -> bool
{
    Lock lock(lock_);

    return address_->Incoming();
}

auto Address::Port() const noexcept -> std::uint16_t
{
    Lock lock(lock_);

    return address_->Port();
}

auto Address::Services() const noexcept -> UnallocatedSet<Service>
{
    Lock lock(lock_);

    return address_->Services();
}

auto Address::Type() const noexcept -> Network
{
    Lock lock(lock_);

    return address_->Type();
}

auto Address::UpdateServices(
    const UnallocatedSet<p2p::Service>& services) noexcept -> pointer
{
    Lock lock(lock_);
    address_->SetServices(services);

    return address_->clone_internal();
}

auto Address::UpdateTime(const Time& time) noexcept -> pointer
{
    Lock lock(lock_);
    address_->SetLastConnected(time);

    return address_->clone_internal();
}
}  // namespace opentxs::blockchain::p2p::peer

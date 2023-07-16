// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Getdata.hpp"  // IWYU pragma: associated

#include <utility>

#include "blockchain/bitcoin/Inventory.hpp"  // IWYU pragma: keep
#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/getdata/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Getdata::Getdata(allocator_type alloc) noexcept
    : Getdata(MessagePrivate::Blank(alloc))
{
}

Getdata::Getdata(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Getdata::Getdata(const Getdata& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Getdata::Getdata(Getdata&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Getdata::Getdata(Getdata&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Getdata::Blank() noexcept -> Getdata&
{
    static auto blank = Getdata{};

    return blank;
}

auto Getdata::get() const noexcept -> std::span<const value_type>
{
    return imp_->asGetdataPrivate()->get();
}

auto Getdata::get() noexcept -> std::span<value_type>
{
    return imp_->asGetdataPrivate()->get();
}

auto Getdata::operator=(const Getdata& rhs) noexcept -> Getdata&
{
    return pmr::copy_assign_child<Message>(*this, rhs);
}

auto Getdata::operator=(Getdata&& rhs) noexcept -> Getdata&
{
    return pmr::move_assign_child<Message>(*this, std::move(rhs));
}

Getdata::~Getdata() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/network/BlockchainHandle.hpp"  // IWYU pragma: associated

#include <utility>

#include "api/network/blockchain/BlockchainHandle.hpp"
#include "internal/util/LogMacros.hpp"

namespace opentxs::api::network
{
auto swap(BlockchainHandle& lhs, BlockchainHandle& rhs) noexcept -> void
{
    lhs.swap(rhs);
}
}  // namespace opentxs::api::network

namespace opentxs::api::network
{
BlockchainHandle::BlockchainHandle(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(imp_);
}

BlockchainHandle::BlockchainHandle(const BlockchainHandle& rhs) noexcept
    : BlockchainHandle(rhs.imp_->clone().release())
{
}

BlockchainHandle::BlockchainHandle(BlockchainHandle&& rhs) noexcept
    : BlockchainHandle(std::make_unique<Imp>().release())
{
    swap(rhs);
}

BlockchainHandle::operator bool() const noexcept { return IsValid(); }

BlockchainHandle::operator const blockchain::node::Manager&() const noexcept
{
    return get();
}

auto BlockchainHandle::get() const noexcept -> const blockchain::node::Manager&
{
    return *(imp_->chain_);
}

auto BlockchainHandle::IsValid() const noexcept -> bool
{
    // TODO once it is possible to construct an empty blockchain::node::Manager
    // this function will distinguish between a pointer to a real object and a
    // pointer to a fake object and api::network::Blockchain::GetChain can be
    // made noexcept

    return true;
}

auto BlockchainHandle::operator=(const BlockchainHandle& rhs) noexcept
    -> BlockchainHandle&
{
    auto temp = std::unique_ptr<Imp>(imp_);
    imp_ = rhs.imp_->clone().release();

    return *this;
}

auto BlockchainHandle::operator=(BlockchainHandle&& rhs) noexcept
    -> BlockchainHandle&
{
    swap(rhs);

    return *this;
}

auto BlockchainHandle::swap(BlockchainHandle& rhs) noexcept -> void
{
    using std::swap;
    swap(imp_, rhs.imp_);
}

BlockchainHandle::~BlockchainHandle()
{
    if (nullptr != imp_) { delete imp_; }
}
}  // namespace opentxs::api::network

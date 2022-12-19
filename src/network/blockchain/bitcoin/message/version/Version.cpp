// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Version.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/version/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Version::Version(allocator_type alloc) noexcept
    : Version(MessagePrivate::Blank(alloc))
{
}

Version::Version(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Version::Version(const Version& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Version::Version(Version&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Version::Version(Version&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Version::Bip37() const noexcept -> bool
{
    return imp_->asVersionPrivate()->Bip37();
}

auto Version::Blank() noexcept -> Version&
{
    static auto blank = Version{};

    return blank;
}

auto Version::Height() const noexcept -> opentxs::blockchain::block::Height
{
    return imp_->asVersionPrivate()->Height();
}

auto Version::LocalAddress() const noexcept -> tcp::endpoint
{
    return imp_->asVersionPrivate()->LocalAddress();
}

auto Version::LocalServices(allocator_type alloc) const noexcept
    -> Set<bitcoin::Service>
{
    return imp_->asVersionPrivate()->LocalServices(alloc);
}

auto Version::Nonce() const noexcept -> message::Nonce
{
    return imp_->asVersionPrivate()->Nonce();
}

auto Version::operator=(const Version& rhs) noexcept -> Version&
{
    return copy_assign_child<Message>(*this, rhs);
}

auto Version::operator=(Version&& rhs) noexcept -> Version&
{
    return move_assign_child<Message>(*this, std::move(rhs));
}

auto Version::ProtocolVersion() const noexcept -> message::ProtocolVersion
{
    return imp_->asVersionPrivate()->ProtocolVersion();
}

auto Version::RemoteAddress() const noexcept -> tcp::endpoint
{
    return imp_->asVersionPrivate()->RemoteAddress();
}

auto Version::RemoteServices(allocator_type alloc) const noexcept
    -> Set<bitcoin::Service>
{
    return imp_->asVersionPrivate()->RemoteServices(alloc);
}

auto Version::UserAgent() const noexcept -> ReadView
{
    return imp_->asVersionPrivate()->UserAgent();
}

Version::~Version() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal

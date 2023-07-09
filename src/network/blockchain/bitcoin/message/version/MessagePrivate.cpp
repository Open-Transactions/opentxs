// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/version/MessagePrivate.hpp"

#include <utility>

namespace opentxs::network::blockchain::bitcoin::message::version
{
MessagePrivate::MessagePrivate(allocator_type alloc) noexcept
    : internal::MessagePrivate(std::move(alloc))
    , self_(this)
{
}

MessagePrivate::MessagePrivate(
    const MessagePrivate& rhs,
    allocator_type alloc) noexcept
    : internal::MessagePrivate(rhs, std::move(alloc))
    , self_(this)
{
}

auto MessagePrivate::Height() const noexcept
    -> opentxs::blockchain::block::Height
{
    return {};
}

auto MessagePrivate::LocalAddress() const noexcept -> tcp::endpoint
{
    return {};
}

auto MessagePrivate::LocalServices(alloc::Strategy alloc) const noexcept
    -> Set<bitcoin::Service>
{
    return Set<bitcoin::Service>{alloc.result_};
}

auto MessagePrivate::Nonce() const noexcept -> message::Nonce { return {}; }

auto MessagePrivate::ProtocolVersion() const noexcept
    -> message::ProtocolVersion
{
    return {};
}

auto MessagePrivate::Bip37() const noexcept -> bool { return {}; }

auto MessagePrivate::RemoteAddress() const noexcept -> tcp::endpoint
{
    return {};
}

auto MessagePrivate::RemoteServices(alloc::Strategy alloc) const noexcept
    -> Set<bitcoin::Service>
{
    return Set<bitcoin::Service>{alloc.result_};
}

auto MessagePrivate::UserAgent() const noexcept -> ReadView { return {}; }

MessagePrivate::~MessagePrivate() { Reset(self_); }
}  // namespace opentxs::network::blockchain::bitcoin::message::version

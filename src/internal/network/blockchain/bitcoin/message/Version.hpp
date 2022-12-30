// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/blockchain/bitcoin/message/Message.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace blockchain
{
namespace bitcoin
{
namespace message
{
namespace internal
{
class MessagePrivate;
}  // namespace internal
}  // namespace message
}  // namespace bitcoin
}  // namespace blockchain
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin::message::internal
{
class Version final : virtual public Message
{
public:
    static auto Blank() noexcept -> Version&;

    auto Bip37() const noexcept -> bool;
    auto Height() const noexcept -> opentxs::blockchain::block::Height;
    auto LocalAddress() const noexcept -> tcp::endpoint;
    auto LocalServices(allocator_type alloc) const noexcept
        -> Set<bitcoin::Service>;
    auto Nonce() const noexcept -> message::Nonce;
    auto ProtocolVersion() const noexcept -> message::ProtocolVersion;
    auto RemoteAddress() const noexcept -> tcp::endpoint;
    auto RemoteServices(allocator_type alloc) const noexcept
        -> Set<bitcoin::Service>;
    auto UserAgent() const noexcept -> ReadView;

    Version(MessagePrivate* imp) noexcept;
    Version(allocator_type alloc = {}) noexcept;
    Version(const Version& rhs, allocator_type alloc = {}) noexcept;
    Version(Version&& rhs) noexcept;
    Version(Version&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Version& rhs) noexcept -> Version&;
    auto operator=(Version&& rhs) noexcept -> Version&;

    ~Version() final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal

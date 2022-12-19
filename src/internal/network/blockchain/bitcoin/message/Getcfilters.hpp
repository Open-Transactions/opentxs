// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/blockchain/bitcoin/message/Message.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Hash;
}  // namespace block
}  // namespace blockchain

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
class Getcfilters final : virtual public Message
{
public:
    static auto Blank() noexcept -> Getcfilters&;

    auto Start() const noexcept -> opentxs::blockchain::block::Height;
    auto Stop() const noexcept -> const opentxs::blockchain::block::Hash&;
    auto Type() const noexcept -> opentxs::blockchain::cfilter::Type;

    Getcfilters(MessagePrivate* imp) noexcept;
    Getcfilters(allocator_type alloc = {}) noexcept;
    Getcfilters(const Getcfilters& rhs, allocator_type alloc = {}) noexcept;
    Getcfilters(Getcfilters&& rhs) noexcept;
    Getcfilters(Getcfilters&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Getcfilters& rhs) noexcept -> Getcfilters&;
    auto operator=(Getcfilters&& rhs) noexcept -> Getcfilters&;

    ~Getcfilters() final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/blockchain/bitcoin/message/Message.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"

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
class Getcfheaders final : virtual public Message
{
public:
    static auto Blank() noexcept -> Getcfheaders&;

    auto Start() const noexcept -> opentxs::blockchain::block::Height;
    auto Stop() const noexcept -> const opentxs::blockchain::block::Hash&;
    auto Type() const noexcept -> opentxs::blockchain::cfilter::Type;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Getcfheaders(MessagePrivate* imp) noexcept;
    Getcfheaders(allocator_type alloc = {}) noexcept;
    Getcfheaders(const Getcfheaders& rhs, allocator_type alloc = {}) noexcept;
    Getcfheaders(Getcfheaders&& rhs) noexcept;
    Getcfheaders(Getcfheaders&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Getcfheaders& rhs) noexcept -> Getcfheaders&;
    auto operator=(Getcfheaders&& rhs) noexcept -> Getcfheaders&;

    ~Getcfheaders() final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal

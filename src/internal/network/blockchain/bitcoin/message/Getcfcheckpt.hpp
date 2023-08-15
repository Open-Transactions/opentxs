// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/blockchain/bitcoin/message/Message.hpp"
#include "internal/util/PMR.hpp"
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
class Getcfcheckpt final : virtual public Message
{
public:
    static auto Blank() noexcept -> Getcfcheckpt&;

    auto Stop() const noexcept -> const opentxs::blockchain::block::Hash&;
    auto Type() const noexcept -> opentxs::blockchain::cfilter::Type;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Getcfcheckpt(MessagePrivate* imp) noexcept;
    Getcfcheckpt(allocator_type alloc = {}) noexcept;
    Getcfcheckpt(const Getcfcheckpt& rhs, allocator_type alloc = {}) noexcept;
    Getcfcheckpt(Getcfcheckpt&& rhs) noexcept;
    Getcfcheckpt(Getcfcheckpt&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Getcfcheckpt& rhs) noexcept -> Getcfcheckpt&;
    auto operator=(Getcfcheckpt&& rhs) noexcept -> Getcfcheckpt&;

    ~Getcfcheckpt() final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal

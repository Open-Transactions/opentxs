// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/blockchain/bitcoin/message/Message.hpp"
#include "internal/util/PMR.hpp"

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
class Verack final : virtual public Message
{
public:
    static auto Blank() noexcept -> Verack&;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Verack(MessagePrivate* imp) noexcept;
    Verack(allocator_type alloc = {}) noexcept;
    Verack(const Verack& rhs, allocator_type alloc = {}) noexcept;
    Verack(Verack&& rhs) noexcept;
    Verack(Verack&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Verack& rhs) noexcept -> Verack&;
    auto operator=(Verack&& rhs) noexcept -> Verack&;

    ~Verack() final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal

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
class Getaddr final : virtual public Message
{
public:
    static auto Blank() noexcept -> Getaddr&;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Getaddr(MessagePrivate* imp) noexcept;
    Getaddr(allocator_type alloc = {}) noexcept;
    Getaddr(const Getaddr& rhs, allocator_type alloc = {}) noexcept;
    Getaddr(Getaddr&& rhs) noexcept;
    Getaddr(Getaddr&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Getaddr& rhs) noexcept -> Getaddr&;
    auto operator=(Getaddr&& rhs) noexcept -> Getaddr&;

    ~Getaddr() final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal

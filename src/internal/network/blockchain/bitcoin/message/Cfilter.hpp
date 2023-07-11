// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "internal/network/blockchain/bitcoin/message/Message.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/util/Types.hpp"

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
class Cfilter final : virtual public Message
{
public:
    static auto Blank() noexcept -> Cfilter&;

    auto Bits() const noexcept -> std::uint8_t;
    auto ElementCount() const noexcept -> std::uint32_t;
    auto FPRate() const noexcept -> std::uint32_t;
    auto Filter() const noexcept -> ReadView;
    auto Hash() const noexcept -> const opentxs::blockchain::block::Hash&;
    auto Type() const noexcept -> opentxs::blockchain::cfilter::Type;

    auto get_deleter() noexcept -> delete_function final
    {
        return make_deleter(this);
    }

    Cfilter(MessagePrivate* imp) noexcept;
    Cfilter(allocator_type alloc = {}) noexcept;
    Cfilter(const Cfilter& rhs, allocator_type alloc = {}) noexcept;
    Cfilter(Cfilter&& rhs) noexcept;
    Cfilter(Cfilter&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Cfilter& rhs) noexcept -> Cfilter&;
    auto operator=(Cfilter&& rhs) noexcept -> Cfilter&;

    ~Cfilter() final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal

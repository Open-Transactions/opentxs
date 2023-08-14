// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <span>

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

namespace cfilter
{
class Header;
}  // namespace cfilter
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
class Cfcheckpt final : virtual public Message
{
public:
    using value_type = opentxs::blockchain::cfilter::Header;

    static auto Blank() noexcept -> Cfcheckpt&;

    auto get() const noexcept -> std::span<const value_type>;
    auto Stop() const noexcept -> const opentxs::blockchain::block::Hash&;
    auto Type() const noexcept -> opentxs::blockchain::cfilter::Type;

    auto get() noexcept -> std::span<value_type>;
    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Cfcheckpt(MessagePrivate* imp) noexcept;
    Cfcheckpt(allocator_type alloc = {}) noexcept;
    Cfcheckpt(const Cfcheckpt& rhs, allocator_type alloc = {}) noexcept;
    Cfcheckpt(Cfcheckpt&& rhs) noexcept;
    Cfcheckpt(Cfcheckpt&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Cfcheckpt& rhs) noexcept -> Cfcheckpt&;
    auto operator=(Cfcheckpt&& rhs) noexcept -> Cfcheckpt&;

    ~Cfcheckpt() final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal

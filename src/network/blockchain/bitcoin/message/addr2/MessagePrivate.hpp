// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"

#include <span>

#include "internal/network/blockchain/bitcoin/message/Addr2.hpp"
#include "internal/util/PMR.hpp"

namespace opentxs::network::blockchain::bitcoin::message::addr2
{
class MessagePrivate : virtual public internal::MessagePrivate
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> MessagePrivate*
    {
        return default_construct<MessagePrivate>({alloc});
    }

    auto asAddr2Private() const noexcept -> const addr2::MessagePrivate* final
    {
        return this;
    }
    auto asAddr2Public() const noexcept -> const internal::Addr2& final
    {
        return self_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> internal::MessagePrivate* override
    {
        return pmr::clone_as<internal::MessagePrivate>(this, {alloc});
    }
    virtual auto get() const noexcept
        -> std::span<const internal::Addr2::value_type>;

    auto asAddr2Private() noexcept -> addr2::MessagePrivate* final
    {
        return this;
    }
    auto asAddr2Public() noexcept -> internal::Addr2& final { return self_; }
    virtual auto get() noexcept -> std::span<internal::Addr2::value_type>;
    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return make_deleter(this);
    }

    MessagePrivate(allocator_type alloc) noexcept;
    MessagePrivate() = delete;
    MessagePrivate(const MessagePrivate& rhs, allocator_type alloc) noexcept;
    MessagePrivate(const MessagePrivate&) = delete;
    MessagePrivate(MessagePrivate&&) = delete;
    auto operator=(const MessagePrivate&) -> MessagePrivate& = delete;
    auto operator=(MessagePrivate&&) -> MessagePrivate& = delete;

    ~MessagePrivate() override;

protected:
    internal::Addr2 self_;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::addr2

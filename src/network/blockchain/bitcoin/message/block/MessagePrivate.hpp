// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"

#include "internal/network/blockchain/bitcoin/message/Block.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::network::blockchain::bitcoin::message::block
{
class MessagePrivate : virtual public internal::MessagePrivate
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> MessagePrivate*;

    auto asBlockPrivate() const noexcept -> const block::MessagePrivate* final
    {
        return this;
    }
    auto asBlockPublic() const noexcept -> const internal::Block& final
    {
        return self_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> internal::MessagePrivate* override;
    virtual auto get() const noexcept -> ReadView;

    auto asBlockPrivate() noexcept -> block::MessagePrivate* final
    {
        return this;
    }
    auto asBlockPublic() noexcept -> internal::Block& final { return self_; }
    [[nodiscard]] auto get_deleter() noexcept -> delete_function override;

    MessagePrivate(allocator_type alloc) noexcept;
    MessagePrivate() = delete;
    MessagePrivate(const MessagePrivate& rhs, allocator_type alloc) noexcept;
    MessagePrivate(const MessagePrivate&) = delete;
    MessagePrivate(MessagePrivate&&) = delete;
    auto operator=(const MessagePrivate&) -> MessagePrivate& = delete;
    auto operator=(MessagePrivate&&) -> MessagePrivate& = delete;

    ~MessagePrivate() override;

protected:
    internal::Block self_;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::block

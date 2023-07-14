// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"

#include "internal/network/blockchain/bitcoin/message/Reject.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::network::blockchain::bitcoin::message::reject
{
class MessagePrivate : virtual public internal::MessagePrivate
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> MessagePrivate*
    {
        return default_construct<MessagePrivate>({alloc});
    }

    auto asRejectPrivate() const noexcept -> const reject::MessagePrivate* final
    {
        return this;
    }
    auto asRejectPublic() const noexcept -> const internal::Reject& final
    {
        return self_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> internal::MessagePrivate* override
    {
        return pmr::clone_as<internal::MessagePrivate>(this, {alloc});
    }
    virtual auto Reason() const noexcept -> ReadView;
    virtual auto RejectedMessage() const noexcept -> ReadView;

    auto asRejectPrivate() noexcept -> reject::MessagePrivate* final
    {
        return this;
    }
    auto asRejectPublic() noexcept -> internal::Reject& final { return self_; }
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
    internal::Reject self_;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::reject

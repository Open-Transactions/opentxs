// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"

#include <functional>
#include <span>

#include "internal/network/blockchain/bitcoin/message/Getdata.hpp"
#include "internal/util/PMR.hpp"

namespace opentxs::network::blockchain::bitcoin::message::getdata
{
class MessagePrivate : virtual public internal::MessagePrivate
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> MessagePrivate*
    {
        return default_construct<MessagePrivate>({alloc});
    }

    auto asGetdataPrivate() const noexcept
        -> const getdata::MessagePrivate* final
    {
        return this;
    }
    auto asGetdataPublic() const noexcept -> const internal::Getdata& final
    {
        return self_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> internal::MessagePrivate* override
    {
        return pmr::clone_as<internal::MessagePrivate>(this, {alloc});
    }
    virtual auto get() const noexcept
        -> std::span<const internal::Getdata::value_type>;

    auto asGetdataPrivate() noexcept -> getdata::MessagePrivate* final
    {
        return this;
    }
    auto asGetdataPublic() noexcept -> internal::Getdata& final
    {
        return self_;
    }
    virtual auto get() noexcept -> std::span<internal::Getdata::value_type>;
    [[nodiscard]] auto get_deleter() noexcept -> std::function<void()> override
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
    internal::Getdata self_;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::getdata
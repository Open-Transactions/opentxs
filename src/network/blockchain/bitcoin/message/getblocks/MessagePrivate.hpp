// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"

#include <functional>
#include <span>

#include "internal/network/blockchain/bitcoin/message/Getblocks.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/util/PMR.hpp"

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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin::message::getblocks
{
class MessagePrivate : virtual public internal::MessagePrivate
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> MessagePrivate*
    {
        return default_construct<MessagePrivate>({alloc});
    }

    auto asGetblocksPrivate() const noexcept
        -> const getblocks::MessagePrivate* final
    {
        return this;
    }
    auto asGetblocksPublic() const noexcept -> const internal::Getblocks& final
    {
        return self_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> internal::MessagePrivate* override
    {
        return pmr::clone_as<internal::MessagePrivate>(this, {alloc});
    }
    virtual auto get() const noexcept
        -> std::span<const opentxs::blockchain::block::Hash>;
    virtual auto Stop() const noexcept
        -> const opentxs::blockchain::block::Hash&;
    virtual auto Version() const noexcept -> ProtocolVersionUnsigned;

    auto asGetblocksPrivate() noexcept -> getblocks::MessagePrivate* final
    {
        return this;
    }
    auto asGetblocksPublic() noexcept -> internal::Getblocks& final
    {
        return self_;
    }
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
    internal::Getblocks self_;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::getblocks

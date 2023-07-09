// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"

#include <functional>

#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/network/blockchain/bitcoin/message/Version.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::network::blockchain::bitcoin::message::version
{
class MessagePrivate : virtual public internal::MessagePrivate
{
public:
    [[nodiscard]] static auto Blank(alloc::Strategy alloc) noexcept
        -> MessagePrivate*
    {
        return default_construct<MessagePrivate>({alloc.result_});
    }

    auto asVersionPrivate() const noexcept
        -> const version::MessagePrivate* final
    {
        return this;
    }
    auto asVersionPublic() const noexcept -> const internal::Version& final
    {
        return self_;
    }
    virtual auto Bip37() const noexcept -> bool;
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> internal::MessagePrivate* override
    {
        return pmr::clone_as<internal::MessagePrivate>(this, {alloc});
    }
    virtual auto Height() const noexcept -> opentxs::blockchain::block::Height;
    virtual auto LocalAddress() const noexcept -> tcp::endpoint;
    virtual auto LocalServices(alloc::Strategy alloc) const noexcept
        -> Set<bitcoin::Service>;
    virtual auto Nonce() const noexcept -> message::Nonce;
    virtual auto ProtocolVersion() const noexcept -> message::ProtocolVersion;
    virtual auto RemoteAddress() const noexcept -> tcp::endpoint;
    virtual auto RemoteServices(alloc::Strategy alloc) const noexcept
        -> Set<bitcoin::Service>;
    virtual auto UserAgent() const noexcept -> ReadView;

    auto asVersionPrivate() noexcept -> version::MessagePrivate* final
    {
        return this;
    }
    auto asVersionPublic() noexcept -> internal::Version& final
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
    internal::Version self_;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::version

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"

#include <span>

#include "internal/network/blockchain/bitcoin/message/Cfheaders.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/util/Allocator.hpp"

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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin::message::cfheaders
{
class MessagePrivate : virtual public internal::MessagePrivate
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> MessagePrivate*
    {
        return pmr::default_construct<MessagePrivate>({alloc});
    }

    auto asCfheadersPrivate() const noexcept
        -> const cfheaders::MessagePrivate* final
    {
        return this;
    }
    auto asCfheadersPublic() const noexcept -> const internal::Cfheaders& final
    {
        return self_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> internal::MessagePrivate* override
    {
        return pmr::clone_as<internal::MessagePrivate>(this, {alloc});
    }
    virtual auto get() const noexcept
        -> std::span<const internal::Cfheaders::value_type>;
    virtual auto Previous() const noexcept
        -> const opentxs::blockchain::cfilter::Header&;
    virtual auto Stop() const noexcept
        -> const opentxs::blockchain::block::Hash&;
    virtual auto Type() const noexcept -> opentxs::blockchain::cfilter::Type;

    auto asCfheadersPrivate() noexcept -> cfheaders::MessagePrivate* final
    {
        return this;
    }
    auto asCfheadersPublic() noexcept -> internal::Cfheaders& final
    {
        return self_;
    }
    virtual auto get() noexcept -> std::span<internal::Cfheaders::value_type>;
    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
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
    internal::Cfheaders self_;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::cfheaders

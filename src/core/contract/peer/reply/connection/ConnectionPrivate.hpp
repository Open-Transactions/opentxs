// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "core/contract/peer/reply/base/ReplyPrivate.hpp"
#include "internal/core/contract/peer/reply/Connection.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::reply
{
class ConnectionPrivate : virtual public internal::Connection,
                          virtual public peer::ReplyPrivate
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> ConnectionPrivate*
    {
        return default_construct<ConnectionPrivate>(
            alloc::PMR<ConnectionPrivate>{alloc});
    }

    [[nodiscard]] virtual auto Accepted() const noexcept -> bool;
    [[nodiscard]] auto asConnectionPrivate() const& noexcept
        -> const reply::ConnectionPrivate* final
    {
        return this;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> peer::ReplyPrivate* override
    {
        return pmr::clone(this, alloc::PMR<ConnectionPrivate>{alloc});
    }
    [[nodiscard]] virtual auto Endpoint() const noexcept -> std::string_view;
    [[nodiscard]] virtual auto Key() const noexcept -> std::string_view;
    [[nodiscard]] virtual auto Login() const noexcept -> std::string_view;
    [[nodiscard]] virtual auto Password() const noexcept -> std::string_view;
    [[nodiscard]] auto Type() const noexcept -> RequestType final;

    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return make_deleter(this);
    }

    ConnectionPrivate(allocator_type alloc) noexcept;
    ConnectionPrivate() = delete;
    ConnectionPrivate(
        const ConnectionPrivate& rhs,
        allocator_type alloc) noexcept;
    ConnectionPrivate(const ConnectionPrivate&) = delete;
    ConnectionPrivate(ConnectionPrivate&&) = delete;
    auto operator=(const ConnectionPrivate&) -> ConnectionPrivate& = delete;
    auto operator=(ConnectionPrivate&&) -> ConnectionPrivate& = delete;

    ~ConnectionPrivate() override;
};
}  // namespace opentxs::contract::peer::reply

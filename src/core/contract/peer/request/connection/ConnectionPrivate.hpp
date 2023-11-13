// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/contract/peer/RequestType.hpp"

#pragma once

#include "core/contract/peer/request/base/RequestPrivate.hpp"
#include "internal/core/contract/peer/request/Connection.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/peer/Types.hpp"  // IWYU pragma: keep
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::request
{
class ConnectionPrivate : virtual public internal::Connection,
                          virtual public peer::RequestPrivate
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> ConnectionPrivate*
    {
        return pmr::default_construct<ConnectionPrivate>(
            alloc::PMR<ConnectionPrivate>{alloc});
    }

    [[nodiscard]] auto asConnectionPrivate() const& noexcept
        -> const request::ConnectionPrivate* final
    {
        return this;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> peer::RequestPrivate* override
    {
        return pmr::clone(this, alloc::PMR<ConnectionPrivate>{alloc});
    }
    [[nodiscard]] virtual auto Kind() const noexcept -> ConnectionInfoType;
    [[nodiscard]] auto Type() const noexcept -> RequestType final;

    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
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
}  // namespace opentxs::contract::peer::request

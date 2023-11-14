// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/contract/peer/RequestType.hpp"

#pragma once

#include <string_view>

#include "core/contract/peer/reply/base/ReplyPrivate.hpp"
#include "internal/core/contract/peer/reply/Outbailment.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/peer/Types.hpp"  // IWYU pragma: keep
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::reply
{
class OutbailmentPrivate : virtual public internal::Outbailment,
                           virtual public peer::ReplyPrivate
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> OutbailmentPrivate*
    {
        return pmr::default_construct<OutbailmentPrivate>(
            alloc::PMR<OutbailmentPrivate>{alloc});
    }

    [[nodiscard]] auto asOutbailmentPrivate() const& noexcept
        -> const reply::OutbailmentPrivate* final
    {
        return this;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> peer::ReplyPrivate* override
    {
        return pmr::clone(this, alloc::PMR<OutbailmentPrivate>{alloc});
    }
    [[nodiscard]] virtual auto Description() const noexcept -> std::string_view;
    [[nodiscard]] auto Type() const noexcept -> RequestType final;

    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
    }

    OutbailmentPrivate(allocator_type alloc) noexcept;
    OutbailmentPrivate() = delete;
    OutbailmentPrivate(
        const OutbailmentPrivate& rhs,
        allocator_type alloc) noexcept;
    OutbailmentPrivate(const OutbailmentPrivate&) = delete;
    OutbailmentPrivate(OutbailmentPrivate&&) = delete;
    auto operator=(const OutbailmentPrivate&) -> OutbailmentPrivate& = delete;
    auto operator=(OutbailmentPrivate&&) -> OutbailmentPrivate& = delete;

    ~OutbailmentPrivate() override;
};
}  // namespace opentxs::contract::peer::reply

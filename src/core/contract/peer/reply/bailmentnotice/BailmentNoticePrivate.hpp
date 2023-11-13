// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/contract/peer/RequestType.hpp"

#pragma once

#include "core/contract/peer/reply/base/ReplyPrivate.hpp"
#include "internal/core/contract/peer/reply/BailmentNotice.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/contract/peer/Types.hpp"  // IWYU pragma: keep
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::reply
{
class BailmentNoticePrivate : virtual public internal::BailmentNotice,
                              virtual public peer::ReplyPrivate
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> BailmentNoticePrivate*
    {
        return pmr::default_construct<BailmentNoticePrivate>(
            alloc::PMR<BailmentNoticePrivate>{alloc});
    }

    [[nodiscard]] virtual auto Amount() const noexcept -> opentxs::Amount;
    [[nodiscard]] auto asBailmentNoticePrivate() const& noexcept
        -> const reply::BailmentNoticePrivate* final
    {
        return this;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> peer::ReplyPrivate* override
    {
        return pmr::clone(this, alloc::PMR<BailmentNoticePrivate>{alloc});
    }
    [[nodiscard]] auto Type() const noexcept -> RequestType final;
    [[nodiscard]] virtual auto Value() const noexcept -> bool;

    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
    }

    BailmentNoticePrivate(allocator_type alloc) noexcept;
    BailmentNoticePrivate() = delete;
    BailmentNoticePrivate(
        const BailmentNoticePrivate& rhs,
        allocator_type alloc) noexcept;
    BailmentNoticePrivate(const BailmentNoticePrivate&) = delete;
    BailmentNoticePrivate(BailmentNoticePrivate&&) = delete;
    auto operator=(const BailmentNoticePrivate&)
        -> BailmentNoticePrivate& = delete;
    auto operator=(BailmentNoticePrivate&&) -> BailmentNoticePrivate& = delete;

    ~BailmentNoticePrivate() override;
};
}  // namespace opentxs::contract::peer::reply

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <string_view>

#include "core/contract/peer/reply/base/ReplyPrivate.hpp"
#include "internal/core/contract/peer/reply/Bailment.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::reply
{
class BailmentPrivate : virtual public internal::Bailment,
                        virtual public peer::ReplyPrivate
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> BailmentPrivate*
    {
        return default_construct<BailmentPrivate>(
            alloc::PMR<BailmentPrivate>{alloc});
    }

    [[nodiscard]] auto asBailmentPrivate() const& noexcept
        -> const reply::BailmentPrivate* final
    {
        return this;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> peer::ReplyPrivate* override
    {
        return pmr::clone(this, alloc::PMR<BailmentPrivate>{alloc});
    }
    [[nodiscard]] virtual auto Instructions() const noexcept
        -> std::string_view;
    [[nodiscard]] auto Type() const noexcept -> RequestType final;

    [[nodiscard]] auto get_deleter() noexcept -> std::function<void()> override
    {
        return make_deleter(this);
    }

    BailmentPrivate(allocator_type alloc) noexcept;
    BailmentPrivate() = delete;
    BailmentPrivate(const BailmentPrivate& rhs, allocator_type alloc) noexcept;
    BailmentPrivate(const BailmentPrivate&) = delete;
    BailmentPrivate(BailmentPrivate&&) = delete;
    auto operator=(const BailmentPrivate&) -> BailmentPrivate& = delete;
    auto operator=(BailmentPrivate&&) -> BailmentPrivate& = delete;

    ~BailmentPrivate() override;
};
}  // namespace opentxs::contract::peer::reply

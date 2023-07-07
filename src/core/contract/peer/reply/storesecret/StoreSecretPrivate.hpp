// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>

#include "core/contract/peer/reply/base/ReplyPrivate.hpp"
#include "internal/core/contract/peer/reply/StoreSecret.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::reply
{
class StoreSecretPrivate : virtual public internal::StoreSecret,
                           virtual public peer::ReplyPrivate
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> StoreSecretPrivate*
    {
        return default_construct<StoreSecretPrivate>(
            alloc::PMR<StoreSecretPrivate>{alloc});
    }

    [[nodiscard]] auto asStoreSecretPrivate() const& noexcept
        -> const reply::StoreSecretPrivate* final
    {
        return this;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> peer::ReplyPrivate* override
    {
        return pmr::clone(this, alloc::PMR<StoreSecretPrivate>{alloc});
    }
    [[nodiscard]] auto Type() const noexcept -> RequestType final;
    [[nodiscard]] virtual auto Value() const noexcept -> bool;

    [[nodiscard]] auto get_deleter() noexcept -> std::function<void()> override
    {
        return make_deleter(this);
    }

    StoreSecretPrivate(allocator_type alloc) noexcept;
    StoreSecretPrivate() = delete;
    StoreSecretPrivate(
        const StoreSecretPrivate& rhs,
        allocator_type alloc) noexcept;
    StoreSecretPrivate(const StoreSecretPrivate&) = delete;
    StoreSecretPrivate(StoreSecretPrivate&&) = delete;
    auto operator=(const StoreSecretPrivate&) -> StoreSecretPrivate& = delete;
    auto operator=(StoreSecretPrivate&&) -> StoreSecretPrivate& = delete;

    ~StoreSecretPrivate() override;
};
}  // namespace opentxs::contract::peer::reply

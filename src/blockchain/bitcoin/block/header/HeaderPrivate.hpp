// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "blockchain/block/header/HeaderPrivate.hpp"

#include <functional>

#include "internal/blockchain/bitcoin/block/Header.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::blockchain::bitcoin::block
{
class HeaderPrivate : virtual public blockchain::block::HeaderPrivate,
                      virtual public internal::Header
{
public:
    [[nodiscard]] static auto Blank(alloc::Strategy alloc) noexcept
        -> HeaderPrivate*
    {
        return default_construct<HeaderPrivate>({alloc.result_});
    }

    auto asBitcoinPrivate() const noexcept
        -> const bitcoin::block::HeaderPrivate* final
    {
        return this;
    }
    auto asBitcoinPublic() const noexcept -> const bitcoin::block::Header& final
    {
        return self_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> blockchain::block::HeaderPrivate* override
    {
        return pmr::clone_as<blockchain::block::HeaderPrivate>(this, {alloc});
    }

    auto asBitcoinPrivate() noexcept -> bitcoin::block::HeaderPrivate* final
    {
        return this;
    }
    auto asBitcoinPublic() noexcept -> bitcoin::block::Header& final
    {
        return self_;
    }
    [[nodiscard]] auto get_deleter() noexcept -> std::function<void()> override
    {
        return make_deleter(this);
    }

    HeaderPrivate(allocator_type alloc) noexcept;
    HeaderPrivate() = delete;
    HeaderPrivate(const HeaderPrivate& rhs, allocator_type alloc) noexcept;
    HeaderPrivate(const HeaderPrivate&) = delete;
    HeaderPrivate(HeaderPrivate&&) = delete;
    auto operator=(const HeaderPrivate&) -> HeaderPrivate& = delete;
    auto operator=(HeaderPrivate&&) -> HeaderPrivate& = delete;

    ~HeaderPrivate() override;

private:
    block::Header self_;
};
}  // namespace opentxs::blockchain::bitcoin::block

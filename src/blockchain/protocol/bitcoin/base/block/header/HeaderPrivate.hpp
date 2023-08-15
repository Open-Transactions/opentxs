// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "blockchain/block/header/HeaderPrivate.hpp"

#include "internal/blockchain/protocol/bitcoin/base/block/Header.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Header.hpp"

namespace opentxs::blockchain::protocol::bitcoin::base::block
{
class HeaderPrivate : virtual public blockchain::block::HeaderPrivate,
                      virtual public internal::Header
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> HeaderPrivate*
    {
        return pmr::default_construct<HeaderPrivate>({alloc});
    }

    auto asBitcoinPrivate() const noexcept
        -> const protocol::bitcoin::base::block::HeaderPrivate* final
    {
        return this;
    }
    auto asBitcoinPublic() const noexcept
        -> const protocol::bitcoin::base::block::Header& final
    {
        return self_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> blockchain::block::HeaderPrivate* override
    {
        return pmr::clone_as<blockchain::block::HeaderPrivate>(this, {alloc});
    }

    auto asBitcoinPrivate() noexcept
        -> protocol::bitcoin::base::block::HeaderPrivate* final
    {
        return this;
    }
    auto asBitcoinPublic() noexcept
        -> protocol::bitcoin::base::block::Header& final
    {
        return self_;
    }
    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
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
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block

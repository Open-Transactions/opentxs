// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/block/Header.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/alloc/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Header;
}  // namespace block

namespace protocol
{
namespace bitcoin
{
namespace base
{
namespace block
{
class Header;
class HeaderPrivate;
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block
{
class HeaderPrivate : virtual public internal::Header,
                      public opentxs::pmr::Allocated
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> HeaderPrivate*
    {
        return pmr::default_construct<HeaderPrivate>({alloc});
    }
    static auto Reset(block::Header& header) noexcept -> void;

    virtual auto asBitcoinPrivate() const noexcept
        -> const protocol::bitcoin::base::block::HeaderPrivate*;
    virtual auto asBitcoinPublic() const noexcept
        -> const protocol::bitcoin::base::block::Header&;
    [[nodiscard]] virtual auto clone(allocator_type alloc) const noexcept
        -> HeaderPrivate*
    {
        return pmr::clone(this, {alloc});
    }

    virtual auto asBitcoinPrivate() noexcept
        -> protocol::bitcoin::base::block::HeaderPrivate*;
    virtual auto asBitcoinPublic() noexcept
        -> protocol::bitcoin::base::block::Header&;
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
};
}  // namespace opentxs::blockchain::block

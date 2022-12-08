// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>

#include "internal/blockchain/block/Header.hpp"
#include "util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Header;
class HeaderPrivate;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Header;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block
{
class HeaderPrivate : virtual public internal::Header,
                      public opentxs::implementation::Allocated
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> HeaderPrivate*;
    static auto Reset(block::Header& header) noexcept -> void;

    virtual auto asBitcoinPrivate() const noexcept
        -> const bitcoin::block::HeaderPrivate*;
    virtual auto asBitcoinPublic() const noexcept
        -> const bitcoin::block::Header&;
    [[nodiscard]] virtual auto clone(allocator_type alloc) const noexcept
        -> HeaderPrivate*;

    virtual auto asBitcoinPrivate() noexcept -> bitcoin::block::HeaderPrivate*;
    virtual auto asBitcoinPublic() noexcept -> bitcoin::block::Header&;
    [[nodiscard]] virtual auto get_deleter() noexcept -> std::function<void()>;

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

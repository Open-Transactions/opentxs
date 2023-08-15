// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/protocol/bitcoin/base/block/Input.hpp"
#include "internal/util/PMR.hpp"
#include "util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace protocol
{
namespace bitcoin
{
namespace base
{
namespace block
{
class Input;
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::protocol::bitcoin::base::block
{
class InputPrivate : virtual public internal::Input,
                     public opentxs::implementation::Allocated
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> InputPrivate*
    {
        return pmr::default_construct<InputPrivate>({alloc});
    }
    static auto Reset(block::Input& tx) noexcept -> void;

    [[nodiscard]] virtual auto clone(allocator_type alloc) const noexcept
        -> InputPrivate*
    {
        return pmr::clone(this, {alloc});
    }

    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
    }

    InputPrivate(allocator_type alloc) noexcept;
    InputPrivate() = delete;
    InputPrivate(const InputPrivate& rhs, allocator_type alloc) noexcept;
    InputPrivate(const InputPrivate&) = delete;
    InputPrivate(InputPrivate&&) = delete;
    auto operator=(const InputPrivate&) -> InputPrivate& = delete;
    auto operator=(InputPrivate&&) -> InputPrivate& = delete;

    ~InputPrivate() override;
};
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block

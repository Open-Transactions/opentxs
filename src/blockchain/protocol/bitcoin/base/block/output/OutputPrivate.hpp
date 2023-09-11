// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/alloc/Allocated.hpp"

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
class Output;
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::protocol::bitcoin::base::block
{
class OutputPrivate : virtual public internal::Output,
                      public opentxs::pmr::Allocated
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> OutputPrivate*
    {
        return pmr::default_construct<OutputPrivate>({alloc});
    }
    static auto Reset(block::Output& tx) noexcept -> void;

    [[nodiscard]] virtual auto clone(allocator_type alloc) const noexcept
        -> OutputPrivate*
    {
        return pmr::clone(this, {alloc});
    }

    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
    }

    OutputPrivate(allocator_type alloc) noexcept;
    OutputPrivate() = delete;
    OutputPrivate(const OutputPrivate& rhs, allocator_type alloc) noexcept;
    OutputPrivate(const OutputPrivate&) = delete;
    OutputPrivate(OutputPrivate&&) = delete;
    auto operator=(const OutputPrivate&) -> OutputPrivate& = delete;
    auto operator=(OutputPrivate&&) -> OutputPrivate& = delete;

    ~OutputPrivate() override;
};
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block

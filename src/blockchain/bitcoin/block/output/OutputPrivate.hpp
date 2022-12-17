// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>

#include "internal/blockchain/bitcoin/block/Output.hpp"
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
class Output;
class OutputPrivate;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Output;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block
{
class OutputPrivate : virtual public internal::Output,
                      public opentxs::implementation::Allocated
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> OutputPrivate*;
    static auto Reset(block::Output& tx) noexcept -> void;

    [[nodiscard]] virtual auto clone(allocator_type alloc) const noexcept
        -> OutputPrivate*;

    [[nodiscard]] virtual auto get_deleter() noexcept -> std::function<void()>;

    OutputPrivate(allocator_type alloc) noexcept;
    OutputPrivate() = delete;
    OutputPrivate(const OutputPrivate& rhs, allocator_type alloc) noexcept;
    OutputPrivate(const OutputPrivate&) = delete;
    OutputPrivate(OutputPrivate&&) = delete;
    auto operator=(const OutputPrivate&) -> OutputPrivate& = delete;
    auto operator=(OutputPrivate&&) -> OutputPrivate& = delete;

    ~OutputPrivate() override;
};
}  // namespace opentxs::blockchain::bitcoin::block
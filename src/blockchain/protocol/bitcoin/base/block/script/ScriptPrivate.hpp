// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/protocol/bitcoin/base/block/Script.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/alloc/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"

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
class Script;
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::protocol::bitcoin::base::block
{
class ScriptPrivate : virtual public internal::Script,
                      public opentxs::pmr::Allocated
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> ScriptPrivate*
    {
        return pmr::default_construct<ScriptPrivate>({alloc});
    }
    static auto Reset(block::Script& tx) noexcept -> void;

    [[nodiscard]] virtual auto clone(allocator_type alloc) const noexcept
        -> ScriptPrivate*
    {
        return pmr::clone(this, {alloc});
    }

    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
    }

    ScriptPrivate(allocator_type alloc) noexcept;
    ScriptPrivate() = delete;
    ScriptPrivate(const ScriptPrivate& rhs, allocator_type alloc) noexcept;
    ScriptPrivate(const ScriptPrivate&) = delete;
    ScriptPrivate(ScriptPrivate&&) = delete;
    auto operator=(const ScriptPrivate&) -> ScriptPrivate& = delete;
    auto operator=(ScriptPrivate&&) -> ScriptPrivate& = delete;

    ~ScriptPrivate() override;
};
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block

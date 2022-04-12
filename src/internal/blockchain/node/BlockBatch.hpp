// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstddef>
#include <memory>
#include <string_view>

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace block
{
class Hash;
}  // namespace block
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class BlockBatch
{
public:
    class Imp;

    auto Get() const noexcept -> const Vector<block::Hash>&;
    auto ID() const noexcept -> std::size_t;
    auto LastActivity() const noexcept -> std::chrono::seconds;
    auto Remaining() const noexcept -> std::size_t;

    auto Submit(const std::string_view block) noexcept -> void;
    auto swap(BlockBatch& rhs) noexcept -> void;

    BlockBatch() noexcept;
    BlockBatch(Imp* imp) noexcept;
    BlockBatch(const BlockBatch&) = delete;
    BlockBatch(BlockBatch&& rhs) noexcept;
    auto operator=(const BlockBatch&) -> BlockBatch& = delete;
    auto operator=(BlockBatch&&) -> BlockBatch& = delete;

    ~BlockBatch();

private:
    Imp* imp_;
};
}  // namespace opentxs::blockchain::node::internal

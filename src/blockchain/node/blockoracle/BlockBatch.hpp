// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string_view>

#include "internal/blockchain/node/BlockBatch.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "util/ScopeGuard.hpp"

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

class ScopeGuard;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class BlockBatch::Imp final : public Allocated
{
public:
    using DownloadCallback = std::function<void(const std::string_view)>;

    const std::size_t id_;
    const Vector<block::Hash> hashes_;
    const Time start_;
    const std::shared_ptr<const ScopeGuard> finish_;

    auto get_allocator() const noexcept -> allocator_type final
    {
        return hashes_.get_allocator();
    }
    auto LastActivity() const noexcept -> std::chrono::seconds;
    auto Remaining() const noexcept -> std::size_t;

    auto Submit(const std::string_view block) noexcept -> void;

    Imp(std::size_t id,
        Vector<block::Hash>&& hashes,
        DownloadCallback download,
        std::shared_ptr<const ScopeGuard>&& finish,
        allocator_type alloc) noexcept;
    Imp(allocator_type alloc = {}) noexcept;

    ~Imp() final;

private:
    const DownloadCallback callback_;
    Time last_;
    std::size_t submitted_;
};
}  // namespace opentxs::blockchain::node::internal

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <string_view>

#include "internal/blockchain/node/Job.hpp"
#include "internal/blockchain/node/blockoracle/BlockBatch.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class BlockBatch::Imp final : public Allocated
{
public:
    using DownloadCallback = std::function<void(const std::string_view)>;

    const download::JobID id_;
    const Vector<block::Hash> hashes_;
    const sTime start_;

    auto get_allocator() const noexcept -> allocator_type final
    {
        return hashes_.get_allocator();
    }
    auto LastActivity() const noexcept -> std::chrono::seconds;
    auto Remaining() const noexcept -> std::size_t;

    auto clone(allocator_type alloc) noexcept -> Imp*
    {
        return pmr::clone_mutable(this, {alloc});
    }
    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }
    auto Submit(const std::string_view block) noexcept -> bool;

    Imp(download::JobID id,
        Vector<block::Hash>&& hashes,
        DownloadCallback download,
        SimpleCallback&& finish,
        allocator_type alloc) noexcept;
    Imp(allocator_type alloc = {}) noexcept;
    Imp(Imp& rhs, allocator_type alloc = {}) noexcept;

    ~Imp() final;

private:
    const Log& log_;
    DownloadCallback callback_;
    SimpleCallback finish_;
    sTime last_;
    std::size_t submitted_;
};
}  // namespace opentxs::blockchain::node::internal

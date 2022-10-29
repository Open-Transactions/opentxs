// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <cstddef>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace ottest
{
struct Counter;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ot = opentxs;

namespace ottest
{
struct OPENTXS_EXPORT SeedTreeNym {
    std::size_t index_{};
    ot::UnallocatedCString id_{};
    ot::UnallocatedCString name_{};
};

struct OPENTXS_EXPORT SeedTreeItem {
    ot::UnallocatedCString id_{};
    ot::UnallocatedCString name_{};
    ot::crypto::SeedStyle type_{};
    ot::UnallocatedVector<SeedTreeNym> rows_;
};

struct OPENTXS_EXPORT SeedTreeData {
    ot::UnallocatedVector<SeedTreeItem> rows_;
};

OPENTXS_EXPORT auto check_seed_tree(
    const ot::api::session::Client& api,
    const SeedTreeData& expected) noexcept -> bool;
OPENTXS_EXPORT auto check_seed_tree_qt(
    const ot::api::session::Client& api,
    const SeedTreeData& expected) noexcept -> bool;
OPENTXS_EXPORT auto init_seed_tree(
    const ot::api::session::Client& api,
    Counter& counter) noexcept -> void;
OPENTXS_EXPORT auto print_seed_tree(
    const ot::api::session::Client& api) noexcept -> ot::UnallocatedCString;
}  // namespace ottest

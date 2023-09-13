// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstddef>
#include <optional>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace ottest
{
class User;
struct Counter;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT SeedTree : public ::testing::Test
{
public:
    static constexpr auto alex_name_{"Alex"};
    static constexpr auto bob_name_{"Bob"};
    static constexpr auto chris_name_{"Chris"};
    static constexpr auto daniel_name_{"Daniel"};
    static constexpr auto pkt_words_{
        "forum school old approve bubble warfare robust figure pact glance "
        "farm leg taxi sing ankle"};
    static constexpr auto pkt_passphrase_{"Password123#"};

    static Counter counter_;
    static std::optional<User> alex_;
    static std::optional<User> bob_;
    static std::optional<User> chris_;

    const ot::api::session::Client& api_;
    const ot::crypto::SeedID seed_1_id_;
    const ot::crypto::SeedID seed_2_id_;
    const ot::crypto::SeedID seed_3_id_;
    ot::PasswordPrompt reason_;

    SeedTree();
};

struct OPENTXS_EXPORT SeedTreeNym {
    std::size_t index_{};
    ot::UnallocatedCString id_{};
    ot::UnallocatedCString name_{};
};

struct OPENTXS_EXPORT SeedTreeItem {
    ot::crypto::SeedID id_{};
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

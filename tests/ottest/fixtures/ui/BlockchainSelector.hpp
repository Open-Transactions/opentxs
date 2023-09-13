// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class BlockchainSelection;
}  // namespace ui
}  // namespace opentxs

namespace ottest
{
struct Counter;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT BlockchainSelector : public ::testing::Test
{
public:
    using Type = ot::blockchain::Type;

    static Counter counter_full_;
    static Counter counter_main_;
    static Counter counter_test_;

    const ot::api::session::Client& client_;
    const ot::ui::BlockchainSelection& full_;
    const ot::ui::BlockchainSelection& main_;
    const ot::ui::BlockchainSelection& test_;

    BlockchainSelector();
};
}  // namespace ottest

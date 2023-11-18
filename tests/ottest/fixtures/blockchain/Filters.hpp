// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "internal/blockchain/Blockchain.hpp"

namespace ottest
{
namespace ot = opentxs;
using namespace opentxs::literals;

class OPENTXS_EXPORT Filters : public ::testing::Test
{
public:
    struct TestData {
        ot::UnallocatedCString block_hash_;
        ot::UnallocatedCString block_;
        ot::UnallocatedVector<ot::UnallocatedCString> previous_;
        ot::UnallocatedVector<ot::UnallocatedCString> outputs_;
        ot::UnallocatedCString previous_header_;
        ot::UnallocatedCString filter_;
        ot::UnallocatedCString header_;
    };
    using TestMap = ot::UnallocatedMap<ot::blockchain::block::Height, TestData>;

    static const ot::blockchain::internal::FilterParams params_;
    static const TestMap gcs_;

    const ot::api::session::Client& api_;

    auto TestGCSBlock(const ot::blockchain::block::Height height) const -> bool;

    Filters();
};
}  // namespace ottest

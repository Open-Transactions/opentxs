// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstddef>

namespace ottest
{
struct Bip158Vector;

namespace ot = opentxs;

struct OPENTXS_EXPORT BIP158 : public ::testing::Test {
    const ot::api::session::Client& api_;

    auto CompareElements(
        const ot::Vector<ot::ByteArray>& input,
        ot::UnallocatedVector<ot::UnallocatedCString> expected) const -> bool;

    auto CompareToOracle(
        const ot::blockchain::Type chain,
        const ot::blockchain::cfilter::Type filterType,
        const ot::Data& filter,
        const ot::blockchain::cfilter::Header& header) const -> bool;

    auto ExtractElements(
        const Bip158Vector& vector,
        const ot::blockchain::block::Block& block,
        const std::size_t encodedElements) const noexcept
        -> ot::Vector<ot::ByteArray>;

    auto GenerateGenesisFilter(
        const ot::blockchain::Type chain,
        const ot::blockchain::cfilter::Type filterType) const noexcept -> bool;

    BIP158();
};
}  // namespace ottest

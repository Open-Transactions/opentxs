// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

namespace ottest
{
namespace ot = opentxs;

struct OPENTXS_EXPORT BitcoinTransaction : public ::testing::Test {
    const ot::api::session::Client& api_;
    const ot::ByteArray tx_id_;
    const ot::ByteArray tx_bytes_;
    const ot::ByteArray mutated_bytes_;
    const ot::ByteArray outpoint_1_;
    const ot::ByteArray outpoint_2_;
    const ot::ByteArray outpoint_3_;
    const ot::ByteArray in_script_1_;
    const ot::ByteArray in_script_2_;
    const ot::ByteArray in_script_3_;
    const ot::ByteArray vbyte_test_transaction_;

    using Pattern =
        ot::blockchain::protocol::bitcoin::base::block::script::Pattern;
    using Position =
        ot::blockchain::protocol::bitcoin::base::block::script::Position;

    static auto IDNormalized(
        const ot::api::Session& api,
        const ot::blockchain::block::Transaction& tx) noexcept
        -> const ot::identifier::Generic&;
    static auto Serialize(const ot::blockchain::block::Transaction& tx) noexcept
        -> ot::ByteArray;

    BitcoinTransaction();
};
}  // namespace ottest

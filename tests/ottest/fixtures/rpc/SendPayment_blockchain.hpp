// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ottest/fixtures/rpc/Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <optional>
#include <tuple>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "ottest/fixtures/blockchain/Common.hpp"
#include "ottest/fixtures/blockchain/ScanListener.hpp"
#include "ottest/fixtures/blockchain/regtest/Normal.hpp"

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT RPC_BC : public RPC_fixture, public Regtest_fixture_normal
{
protected:
    using Subchain = ot::blockchain::crypto::Subchain;

    static ot::Nym_p alex_p_;
    static ot::UnallocatedDeque<ot::blockchain::block::TransactionHash>
        transactions_;
    static std::unique_ptr<ScanListener> listener_p_;

    const ot::identity::Nym& alex_;
    const ot::blockchain::crypto::HD account_;
    const Generator mine_to_alex_;
    ScanListener& listener_;

    auto Cleanup() noexcept -> void final;

    RPC_BC();
};
}  // namespace ottest

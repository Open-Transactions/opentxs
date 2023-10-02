// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <ratio>
#include <string_view>
#include <tuple>

#include "internal/api/session/UI.hpp"
#include "internal/interface/ui/AccountActivity.hpp"
#include "internal/interface/ui/BalanceItem.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/fixtures/blockchain/Common.hpp"
#include "ottest/fixtures/blockchain/ScanListener.hpp"
#include "ottest/fixtures/blockchain/regtest/Normal.hpp"
#include "ottest/fixtures/common/Counter.hpp"

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

constexpr auto blocks_ = std::uint64_t{200u};
constexpr auto tx_per_block_ = std::uint64_t{500u};
constexpr auto transaction_count_ = blocks_ * tx_per_block_;
constexpr auto amount_ = std::uint64_t{100000000u};

class OPENTXS_EXPORT Regtest_stress : public Regtest_fixture_normal
{
protected:
    using Subchain = ot::blockchain::crypto::Subchain;
    using Transactions =
        ot::UnallocatedDeque<ot::blockchain::block::TransactionHash>;

    static ot::Nym_p alex_p_;
    static ot::Nym_p bob_p_;
    static Transactions transactions_;
    static std::unique_ptr<ScanListener> listener_alex_p_;
    static std::unique_ptr<ScanListener> listener_bob_p_;

    const ot::identity::Nym& alex_;
    const ot::identity::Nym& bob_;
    const ot::blockchain::crypto::HD& alex_account_;
    const ot::blockchain::crypto::HD& bob_account_;
    const ot::identifier::Account& expected_account_alex_;
    const ot::identifier::Account& expected_account_bob_;
    const ot::identifier::Notary& expected_notary_;
    const ot::identifier::UnitDefinition& expected_unit_;
    const ot::UnallocatedCString expected_display_unit_;
    const ot::UnallocatedCString expected_account_name_;
    const ot::UnallocatedCString expected_notary_name_;
    const ot::UnallocatedCString memo_outgoing_;
    const ot::AccountType expected_account_type_;
    const ot::UnitType expected_unit_type_;
    const Generator mine_to_alex_;
    ScanListener& listener_alex_;
    ScanListener& listener_bob_;

    auto GetAddresses() noexcept
        -> ot::UnallocatedVector<ot::UnallocatedCString>;
    auto Shutdown() noexcept -> void final;

    Regtest_stress();
};
}  // namespace ottest

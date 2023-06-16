// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/regtest/SyncServer.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>

#include "internal/util/LogMacros.hpp"
#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/fixtures/blockchain/SyncRequestor.hpp"
#include "ottest/fixtures/blockchain/SyncSubscriber.hpp"
#include "ottest/fixtures/blockchain/regtest/Base.hpp"
#include "ottest/fixtures/common/User.hpp"

namespace ottest
{
bool Regtest_fixture_sync_server::init_sync_server_{false};
const User Regtest_fixture_sync_server::alex_{
    GetPaymentCodeVector3().alice_.words_,
    "Alex"};
std::optional<ot::OTServerContract> Regtest_fixture_sync_server::notary_{
    std::nullopt};
std::optional<ot::OTUnitDefinition> Regtest_fixture_sync_server::unit_{
    std::nullopt};
std::unique_ptr<SyncSubscriber> Regtest_fixture_sync_server::sync_subscriber_{};
std::unique_ptr<SyncRequestor> Regtest_fixture_sync_server::sync_requestor_{};
}  // namespace ottest

namespace ottest
{
Regtest_fixture_sync_server::Regtest_fixture_sync_server()
    : Regtest_fixture_normal(
          ot_,
          0,
          ot::Options{}.SetBlockchainWalletEnabled(false))
{
    if (false == init_sync_server_) {
        auto& alex = const_cast<User&>(alex_);
        alex.init(miner_);

        OT_ASSERT(
            alex.payment_code_ == GetPaymentCodeVector3().alice_.payment_code_);

        init_sync_server_ = true;
    }
}

auto Regtest_fixture_sync_server::Requestor() noexcept -> SyncRequestor&
{
    if (false == sync_requestor_.operator bool()) {
        sync_requestor_ =
            std::make_unique<SyncRequestor>(miner_, mined_blocks_);
    }

    return *sync_requestor_;
}

auto Regtest_fixture_sync_server::Shutdown() noexcept -> void
{
    sync_requestor_.reset();
    sync_subscriber_.reset();
    Regtest_fixture_base::Shutdown();
}

auto Regtest_fixture_sync_server::Subscriber() noexcept -> SyncSubscriber&
{
    if (false == sync_subscriber_.operator bool()) {
        sync_subscriber_ =
            std::make_unique<SyncSubscriber>(miner_, mined_blocks_);
    }

    return *sync_subscriber_;
}
}  // namespace ottest

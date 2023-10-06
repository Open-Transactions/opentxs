// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/regtest/MultiplePaymentCode.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <string_view>

#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/fixtures/blockchain/Common.hpp"
#include "ottest/fixtures/blockchain/ScanListener.hpp"
#include "ottest/fixtures/blockchain/TXOs.hpp"
#include "ottest/fixtures/common/User.hpp"

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;
bool Regtest_multiple_payment_code::init_multiple_{false};
const User Regtest_multiple_payment_code::chris_{
    GetPaymentCodeVector3().bob_.words_,
    "Chris"};
const User Regtest_multiple_payment_code::daniel_{
    GetPaymentCodeVector3().bob_.words_,
    "Daniel"};
TXOs Regtest_multiple_payment_code::txos_chris_{chris_};
TXOs Regtest_multiple_payment_code::txos_daniel_{daniel_};
std::unique_ptr<ScanListener>
    Regtest_multiple_payment_code::listener_chris_p_{};
std::unique_ptr<ScanListener>
    Regtest_multiple_payment_code::listener_daniel_p_{};
}  // namespace ottest

namespace ottest
{
Regtest_multiple_payment_code::Regtest_multiple_payment_code()
    : Regtest_payment_code()
    , listener_chris_([&]() -> ScanListener& {
        if (!listener_chris_p_) {
            listener_chris_p_ = std::make_unique<ScanListener>(client_2_);
        }

        opentxs::assert_false(nullptr == listener_chris_p_);

        return *listener_chris_p_;
    }())
    , listener_daniel_([&]() -> ScanListener& {
        if (!listener_daniel_p_) {
            listener_daniel_p_ = std::make_unique<ScanListener>(client_2_);
        }

        opentxs::assert_false(nullptr == listener_daniel_p_);

        return *listener_daniel_p_;
    }())
{
    if (false == init_multiple_) {
        auto cb = [](User& user) {
            const auto& api = *user.api_;
            const auto& nymID = user.nym_id_;
            const auto reason = api.Factory().PasswordPrompt(__func__);
            api.Crypto().Blockchain().NewHDSubaccount(
                nymID,
                ot::blockchain::crypto::HDProtocol::BIP_44,
                test_chain_,
                reason);
        };
        auto& chris = const_cast<User&>(chris_);
        auto& daniel = const_cast<User&>(daniel_);
        using enum ot::identity::Type;
        chris.init_custom(client_2_, server_1_, cb, individual, 1);
        daniel.init_custom(client_2_, server_1_, cb, individual, 2);

        opentxs::assert_true(
            alex_.payment_code_ ==
            GetPaymentCodeVector3().alice_.payment_code_);
        opentxs::assert_true(
            bob_.payment_code_ == GetPaymentCodeVector3().bob_.payment_code_);

        init_multiple_ = true;
    }
}

auto Regtest_multiple_payment_code::ChrisHD() const noexcept
    -> const ot::blockchain::crypto::HD&
{
    return client_2_.Crypto()
        .Blockchain()
        .Account(chris_.nym_id_, test_chain_)
        .GetHD()
        .at(0);
}

auto Regtest_multiple_payment_code::ChrisPC() const noexcept
    -> const ot::blockchain::crypto::PaymentCode&
{
    return client_2_.Crypto()
        .Blockchain()
        .Account(chris_.nym_id_, test_chain_)
        .GetPaymentCode()
        .at(0);
}

auto Regtest_multiple_payment_code::DanielHD() const noexcept
    -> const ot::blockchain::crypto::HD&
{
    return client_2_.Crypto()
        .Blockchain()
        .Account(daniel_.nym_id_, test_chain_)
        .GetHD()
        .at(0);
}

auto Regtest_multiple_payment_code::DanielPC() const noexcept
    -> const ot::blockchain::crypto::PaymentCode&
{
    return client_2_.Crypto()
        .Blockchain()
        .Account(daniel_.nym_id_, test_chain_)
        .GetPaymentCode()
        .at(0);
}

auto Regtest_multiple_payment_code::Shutdown() noexcept -> void
{
    listener_daniel_p_.reset();
    listener_chris_p_.reset();
    Regtest_payment_code::Shutdown();
}
}  // namespace ottest

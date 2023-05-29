// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstddef>
#include <iterator>
#include <memory>
#include <utility>

#include "internal/api/FactoryAPI.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/core/String.hpp"
#include "internal/crypto/Envelope.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ot = opentxs;

namespace ottest
{
using namespace opentxs::literals;

bool init_{false};

class Test_Envelope : public ::testing::Test
{
public:
    using Nyms = ot::UnallocatedVector<ot::Nym_p>;
    using Test = std::pair<bool, ot::UnallocatedVector<int>>;
    using Expected = ot::UnallocatedVector<Test>;

    static const bool have_rsa_;
    static const bool have_secp256k1_;
    static const bool have_ed25519_;
    static const Expected expected_;
    static Nyms nyms_;

    const ot::api::Session& sender_;
    const ot::api::Session& recipient_;
    const ot::PasswordPrompt reason_s_;
    const ot::PasswordPrompt reason_r_;
    const ot::OTString plaintext_;

    static auto can_seal(const std::size_t row) -> bool
    {
        return expected_.at(row).first;
    }
    static auto can_open(const std::size_t row, const std::size_t column)
        -> bool
    {
        return can_seal(row) && should_seal(row, column);
    }
    static auto is_active(const std::size_t row, const std::size_t column)
        -> bool
    {
        return 0 != (row & (1_uz << column));
    }
    static auto should_seal(const std::size_t row, const std::size_t column)
        -> bool
    {
        return expected_.at(row).second.at(column);
    }

    Test_Envelope()
        : sender_(OTTestEnvironment::GetOT().StartClientSession(0))
        , recipient_(OTTestEnvironment::GetOT().StartClientSession(1))
        , reason_s_(sender_.Factory().PasswordPrompt(__func__))
        , reason_r_(recipient_.Factory().PasswordPrompt(__func__))
        , plaintext_(ot::String::Factory(
              "The quick brown fox jumped over the lazy dog"))
    {
        if (false == init_) {
            init_ = true;
            {
                auto params = [this] {
                    using Type = ot::crypto::ParameterType;

                    if (have_ed25519_) {

                        return ot::crypto::Parameters{
                            recipient_.Factory(), Type::ed25519};
                    } else if (have_secp256k1_) {

                        return ot::crypto::Parameters{
                            recipient_.Factory(), Type::secp256k1};
                    } else if (have_rsa_) {

                        return ot::crypto::Parameters{
                            recipient_.Factory(), Type::rsa};
                    } else {

                        return ot::crypto::Parameters{recipient_.Factory()};
                    }
                }();
                auto rNym = recipient_.Wallet().Nym(params, reason_r_, "");

                OT_ASSERT(rNym);

                auto bytes = ot::Space{};
                OT_ASSERT(rNym->Serialize(ot::writer(bytes)));
                nyms_.emplace_back(sender_.Wallet().Nym(ot::reader(bytes)));

                OT_ASSERT(bool(*nyms_.crbegin()));
            }
            {
                auto params = [this] {
                    using Type = ot::crypto::ParameterType;

                    if (have_secp256k1_) {

                        return ot::crypto::Parameters{
                            recipient_.Factory(), Type::secp256k1};
                    } else if (have_rsa_) {

                        return ot::crypto::Parameters{
                            recipient_.Factory(), Type::rsa};
                    } else if (have_ed25519_) {

                        return ot::crypto::Parameters{
                            recipient_.Factory(), Type::ed25519};
                    } else {

                        return ot::crypto::Parameters{recipient_.Factory()};
                    }
                }();
                auto rNym = recipient_.Wallet().Nym(params, reason_r_, "");

                OT_ASSERT(rNym);

                auto bytes = ot::Space{};
                OT_ASSERT(rNym->Serialize(ot::writer(bytes)));
                nyms_.emplace_back(sender_.Wallet().Nym(ot::reader(bytes)));

                OT_ASSERT(bool(*nyms_.crbegin()));
            }
            {
                auto params = [this] {
                    using Type = ot::crypto::ParameterType;

                    if (have_rsa_) {

                        return ot::crypto::Parameters{
                            recipient_.Factory(), Type::rsa};
                    } else if (have_ed25519_) {

                        return ot::crypto::Parameters{
                            recipient_.Factory(), Type::ed25519};
                    } else if (have_secp256k1_) {

                        return ot::crypto::Parameters{
                            recipient_.Factory(), Type::secp256k1};
                    } else {

                        return ot::crypto::Parameters{recipient_.Factory()};
                    }
                }();
                auto rNym = recipient_.Wallet().Nym(params, reason_r_, "");

                OT_ASSERT(rNym);

                auto bytes = ot::Space{};
                OT_ASSERT(rNym->Serialize(ot::writer(bytes)));
                nyms_.emplace_back(sender_.Wallet().Nym(ot::reader(bytes)));

                OT_ASSERT(bool(*nyms_.crbegin()));
            }
        }
    }
};

const bool Test_Envelope::have_rsa_{
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::Legacy)};
const bool Test_Envelope::have_secp256k1_{
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::Secp256k1)};
const bool Test_Envelope::have_ed25519_{
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::ED25519)};
Test_Envelope::Nyms Test_Envelope::nyms_{};
const Test_Envelope::Expected Test_Envelope::expected_{
    {false, {false, false, false}},
    {true, {true, false, false}},
    {true, {false, true, false}},
    {true, {true, true, false}},
    {true, {false, false, true}},
    {true, {true, false, true}},
    {true, {false, true, true}},
    {true, {true, true, true}},
};

TEST_F(Test_Envelope, init_ot) {}

TEST_F(Test_Envelope, one_recipient)
{
    for (const auto& pNym : nyms_) {
        const auto& nym = *pNym;
        auto plaintext = ot::String::Factory();
        auto armored = sender_.Factory().Internal().Armored();
        auto sender = sender_.Factory().InternalSession().Envelope();
        const auto sealed = sender->Seal(nym, plaintext_->Bytes(), reason_s_);

        EXPECT_TRUE(sealed);

        if (false == sealed) { continue; }

        EXPECT_TRUE(sender->Armored(armored));

        try {
            auto recipient =
                recipient_.Factory().InternalSession().Envelope(armored);
            auto opened =
                recipient->Open(nym, plaintext->WriteInto(), reason_r_);

            EXPECT_FALSE(opened);

            auto rNym = recipient_.Wallet().Nym(nym.ID());

            OT_ASSERT(rNym);

            opened = recipient->Open(*rNym, plaintext->WriteInto(), reason_r_);

            EXPECT_TRUE(opened);

            if (opened) { EXPECT_STREQ(plaintext->Get(), plaintext_->Get()); }
        } catch (...) {
            EXPECT_TRUE(false);
        }
    }
}

TEST_F(Test_Envelope, multiple_recipients)
{
    constexpr auto one = 1_uz;

    for (auto row = 0_uz; row < (one << nyms_.size()); ++row) {
        auto recipients = ot::crypto::Envelope::Recipients{};
        auto sender = sender_.Factory().InternalSession().Envelope();

        for (auto nym = nyms_.cbegin(); nym != nyms_.cend(); ++nym) {
            const auto column =
                static_cast<std::size_t>(std::distance(nyms_.cbegin(), nym));

            if (is_active(row, column)) { recipients.insert(*nym); }
        }

        const auto sealed =
            sender->Seal(recipients, plaintext_->Bytes(), reason_s_);

        EXPECT_EQ(sealed, can_seal(row));

        if (false == sealed) { continue; }

        auto bytes = ot::Space{};
        ASSERT_TRUE(sender->Serialize(ot::writer(bytes)));

        for (auto nym = nyms_.cbegin(); nym != nyms_.cend(); ++nym) {
            const auto column =
                static_cast<std::size_t>(std::distance(nyms_.cbegin(), nym));
            auto plaintext = ot::String::Factory();

            try {
                auto recipient = sender_.Factory().InternalSession().Envelope(
                    ot::reader(bytes));
                auto rNym = recipient_.Wallet().Nym((*nym)->ID());

                OT_ASSERT(rNym);

                const auto opened =
                    recipient->Open(*rNym, plaintext->WriteInto(), reason_s_);

                EXPECT_EQ(opened, can_open(row, column));

                if (opened) {
                    EXPECT_STREQ(plaintext->Get(), plaintext_->Get());
                }
            } catch (...) {
                EXPECT_TRUE(false);
            }
        }
    }
}
}  // namespace ottest

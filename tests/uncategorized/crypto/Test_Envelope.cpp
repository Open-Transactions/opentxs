// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstddef>
#include <iterator>
#include <memory>
#include <string_view>

#include "internal/api/FactoryAPI.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/core/String.hpp"
#include "internal/crypto/Envelope.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "ottest/fixtures/crypto/Envelope.hpp"

namespace ot = opentxs;

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

TEST_F(Envelope, init_ot) {}

TEST_F(Envelope, one_recipient)
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

TEST_F(Envelope, multiple_recipients)
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

// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <optional>
#include <string_view>

namespace ot = opentxs;

namespace ottest
{
using namespace std::literals;

struct OPENTXS_EXPORT Test_Symmetric : public ::testing::Test {
    static constexpr auto master_password_ = "test password"sv;
    static constexpr auto plaintext_ =
        "The quick brown fox jumped over the lazy dog."sv;

    static const ot::crypto::symmetric::Algorithm mode_;
    static bool init_;
    static ot::identifier::Nym alice_nym_id_;
    static ot::identifier::Nym bob_nym_id_;
    static ot::crypto::symmetric::Key key_;
    static ot::crypto::symmetric::Key second_key_;
    static std::optional<ot::Secret> key_password_;
    static ot::Space ciphertext_;
    static ot::Space second_ciphertext_;

    const ot::api::session::Client& api_;
    ot::PasswordPrompt reason_;
    ot::Nym_p alice_;
    ot::Nym_p bob_;

    auto SetPassword(ot::PasswordPrompt& out) const noexcept -> bool;
    auto SetPassword(const ot::Secret& password, ot::PasswordPrompt& out)
        const noexcept -> bool;

    Test_Symmetric();

    auto init() noexcept -> void;
};
}  // namespace ottest

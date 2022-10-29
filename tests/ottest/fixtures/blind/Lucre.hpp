// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <optional>

namespace ot = opentxs;

namespace ottest
{
class OPENTXS_EXPORT Lucre : public ::testing::Test
{
public:
    static constexpr auto MINT_EXPIRE_MONTHS{6};
    static constexpr auto MINT_VALID_MONTHS{12};
    static constexpr auto REQUEST_PURSE_VALUE{20000};

    static bool init_;
    static ot::identifier::Nym alice_nym_id_;
    static ot::identifier::Nym bob_nym_id_;
    static const ot::identifier::Notary server_id_;
    static const ot::identifier::UnitDefinition unit_id_;
    static std::optional<ot::otx::blind::Mint> mint_;
    static std::optional<ot::otx::blind::Purse> request_purse_;
    static std::optional<ot::otx::blind::Purse> issue_purse_;
    static ot::Space serialized_bytes_;
    static ot::Time valid_from_;
    static ot::Time valid_to_;

    const ot::api::session::Client& api_;
    ot::PasswordPrompt reason_;
    ot::Nym_p alice_;
    ot::Nym_p bob_;

    auto DeserializePurse() noexcept -> ot::otx::blind::Purse;
    auto DeserializePurse(ot::ReadView bytes) noexcept -> ot::otx::blind::Purse;
    auto GenerateMint() noexcept -> bool;
    auto IssuePurse(ot::otx::blind::Purse& request) noexcept -> bool;
    auto MakePurse() noexcept -> void;
    auto NewPurse() noexcept -> bool;
    auto Process(ot::otx::blind::Purse& purse) noexcept -> bool;
    auto ReceivePurse() noexcept -> bool;
    auto Sign(ot::otx::blind::Token& token) noexcept -> bool;
    auto Verify(const ot::otx::blind::Token& token) noexcept -> bool;

    Lucre();

private:
    auto init() noexcept -> void;
};
}  // namespace ottest

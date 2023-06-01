// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blind/Lucre.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <chrono>
#include <memory>
#include <shared_mutex>
#include <utility>

#include "internal/api/session/Client.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/otx/blind/Factory.hpp"
#include "internal/otx/blind/Mint.hpp"
#include "internal/otx/blind/Purse.hpp"
#include "internal/otx/blind/Token.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "internal/util/Editor.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
bool Lucre::init_{false};
ot::identifier::Nym Lucre::alice_nym_id_{};
ot::identifier::Nym Lucre::bob_nym_id_{};
const ot::identifier::Notary Lucre::server_id_{};
const ot::identifier::UnitDefinition Lucre::unit_id_{};
std::optional<ot::otx::blind::Mint> Lucre::mint_{std::nullopt};
std::optional<ot::otx::blind::Purse> Lucre::request_purse_{std::nullopt};
std::optional<ot::otx::blind::Purse> Lucre::issue_purse_{std::nullopt};
ot::Space Lucre::serialized_bytes_{};
ot::Time Lucre::valid_from_;
ot::Time Lucre::valid_to_;
}  // namespace ottest

namespace ottest
{
Lucre::Lucre()
    : api_(dynamic_cast<const ot::api::session::Client&>(
          OTTestEnvironment::GetOT().StartClientSession(0)))
    , reason_(api_.Factory().PasswordPrompt(__func__))
    , alice_()
    , bob_()
{
    if (false == init_) { init(); }

    alice_ = api_.Wallet().Nym(alice_nym_id_);
    bob_ = api_.Wallet().Nym(bob_nym_id_);
}

auto Lucre::DeserializePurse() noexcept -> ot::otx::blind::Purse
{
    return DeserializePurse(ot::reader(serialized_bytes_));
}

auto Lucre::DeserializePurse(ot::ReadView bytes) noexcept
    -> ot::otx::blind::Purse
{
    return ot::factory::Purse(api_, bytes);
}

auto Lucre::GenerateMint() noexcept -> bool
{
    auto out{true};
    mint_.emplace(api_.Factory().Mint(server_id_, unit_id_));
    out &= mint_.operator bool();

    EXPECT_TRUE(mint_);

    if (false == mint_) { return false; }

    const auto& nym = *bob_;
    const auto now = ot::Clock::now();
    const std::chrono::seconds expireInterval(
        std::chrono::hours(MINT_EXPIRE_MONTHS * 30 * 24));
    const std::chrono::seconds validInterval(
        std::chrono::hours(MINT_VALID_MONTHS * 30 * 24));
    const auto expires = now + expireInterval;
    const auto validTo = now + validInterval;
    valid_from_ = now;
    valid_to_ = validTo;
    mint_->Internal().GenerateNewMint(
        api_.Wallet(),
        0,
        now,
        validTo,
        expires,
        unit_id_,
        server_id_,
        nym,
        1,
        10,
        100,
        1000,
        10000,
        100000,
        1000000,
        10000000,
        100000000,
        1000000000,
        288,
        reason_);

    return out;
}

auto Lucre::init() noexcept -> void
{
    const auto seedA = api_.InternalClient().Exec().Wallet_ImportSeed(
        "spike nominee miss inquiry fee nothing belt list other "
        "daughter leave valley twelve gossip paper",
        "");
    const auto seedB = api_.InternalClient().Exec().Wallet_ImportSeed(
        "trim thunder unveil reduce crop cradle zone inquiry "
        "anchor skate property fringe obey butter text tank drama "
        "palm guilt pudding laundry stay axis prosper",
        "");
    alice_nym_id_ =
        api_.Wallet().Nym({api_.Factory(), seedA, 0}, reason_, "Alice")->ID();
    bob_nym_id_ =
        api_.Wallet().Nym({api_.Factory(), seedB, 0}, reason_, "Bob")->ID();
    const_cast<ot::identifier::UnitDefinition&>(unit_id_) =
        api_.Factory().UnitIDFromRandom();
    const_cast<ot::identifier::Notary&>(server_id_) =
        api_.Factory().NotaryIDFromRandom();
    init_ = true;
}

auto Lucre::IssuePurse(ot::otx::blind::Purse& request) noexcept -> bool
{
    issue_purse_.emplace(ot::factory::Purse(api_, request, *alice_, reason_));

    return issue_purse_.has_value();
}

auto Lucre::MakePurse() noexcept -> void
{
    api_.Wallet().Internal().mutable_Purse(
        alice_nym_id_,
        server_id_,
        unit_id_,
        reason_,
        ot::otx::blind::CashType::Lucre);
}

auto Lucre::NewPurse() noexcept -> bool
{
    request_purse_.emplace(ot::factory::Purse(
        api_,
        *alice_,
        server_id_,
        *bob_,
        ot::otx::blind::CashType::Lucre,
        *mint_,
        REQUEST_PURSE_VALUE,
        reason_));

    return request_purse_.has_value();
}

auto Lucre::Process(ot::otx::blind::Purse& purse) noexcept -> bool
{
    return purse.Internal().Process(*alice_, *mint_, reason_);
}

auto Lucre::ReceivePurse() noexcept -> bool
{
    EXPECT_TRUE(issue_purse_.has_value());

    if (false == issue_purse_.has_value()) { return false; }

    auto purseEditor = api_.Wallet().Internal().mutable_Purse(
        alice_nym_id_,
        server_id_,
        unit_id_,
        reason_,
        ot::otx::blind::CashType::Lucre);
    auto& purse = purseEditor.get();
    auto& issuePurse = *issue_purse_;
    const auto& alice = *alice_;
    auto out{true};
    const auto unlocked = issuePurse.Unlock(alice, reason_);
    const auto isUnlocked = issuePurse.IsUnlocked();
    out &= unlocked;
    out &= isUnlocked;

    EXPECT_TRUE(unlocked);
    EXPECT_TRUE(isUnlocked);

    auto token = issuePurse.Pop();

    while (token) {
        const auto markedSpent = token.Internal().MarkSpent(reason_);
        const auto isSpent = token.IsSpent(reason_);
        const auto pushed = purse.Push(std::move(token), reason_);

        out &= markedSpent;
        out &= isSpent;
        out &= pushed;

        EXPECT_TRUE(markedSpent);
        EXPECT_TRUE(isSpent);
        EXPECT_TRUE(pushed);

        token = issuePurse.Pop();
    }

    out &= (purse.size() == 2);
    out &= (purse.Value() == 0);
    out &= (issuePurse.Value() == 0);

    EXPECT_EQ(purse.size(), 2);
    EXPECT_EQ(purse.Value(), 0);
    EXPECT_EQ(issuePurse.Value(), 0);

    return out;
}

auto Lucre::Sign(ot::otx::blind::Token& token) noexcept -> bool
{
    return mint_->Internal().SignToken(*bob_, token, reason_);
}

auto Lucre::Verify(const ot::otx::blind::Token& token) noexcept -> bool
{
    return mint_->Internal().VerifyToken(*bob_, token, reason_);
}
}  // namespace ottest

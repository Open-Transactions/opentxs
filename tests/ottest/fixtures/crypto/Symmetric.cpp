// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/crypto/Symmetric.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <memory>

#include "internal/api/session/Client.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "internal/util/PasswordPrompt.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
const ot::crypto::symmetric::Algorithm Test_Symmetric::mode_{
    ot::crypto::symmetric::Algorithm::ChaCha20Poly1305};
bool Test_Symmetric::init_{false};
ot::identifier::Nym Test_Symmetric::alice_nym_id_{};
ot::identifier::Nym Test_Symmetric::bob_nym_id_{};
ot::crypto::symmetric::Key Test_Symmetric::key_{};
ot::crypto::symmetric::Key Test_Symmetric::second_key_{};
std::optional<ot::Secret> Test_Symmetric::key_password_{};
ot::Space Test_Symmetric::ciphertext_{};
ot::Space Test_Symmetric::second_ciphertext_{};
}  // namespace ottest

namespace ottest
{
Test_Symmetric::Test_Symmetric()
    : api_(OTTestEnvironment::GetOT().StartClientSession(0))
    , reason_(api_.Factory().PasswordPrompt(__func__))
    , alice_()
    , bob_()
{
    if (false == init_) { init(); }

    alice_ = api_.Wallet().Nym(alice_nym_id_);
    bob_ = api_.Wallet().Nym(bob_nym_id_);
}

auto Test_Symmetric::init() noexcept -> void
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
    key_password_ = api_.Factory().SecretFromText(master_password_);
    init_ = true;
}

auto Test_Symmetric::SetPassword(ot::PasswordPrompt& out) const noexcept -> bool
{
    return SetPassword(key_password_.value(), out);
}

auto Test_Symmetric::SetPassword(
    const ot::Secret& password,
    ot::PasswordPrompt& out) const noexcept -> bool
{
    return out.Internal().SetPassword(password);
}
}  // namespace ottest

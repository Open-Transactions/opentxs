// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"             // IWYU pragma: associated
#include "1_Internal.hpp"           // IWYU pragma: associated
#include "otx/blind/token/Imp.hpp"  // IWYU pragma: associated

#include "internal/core/Factory.hpp"
#include "internal/otx/blind/Purse.hpp"
#include "internal/otx/blind/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"
#include "opentxs/otx/blind/CashType.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/Token.pb.h"

namespace opentxs::otx::blind::token
{
const opentxs::crypto::key::symmetric::Algorithm Token::mode_{
    opentxs::crypto::key::symmetric::Algorithm::ChaCha20Poly1305};

Token::Token(
    const api::Session& api,
    blind::internal::Purse& purse,
    const blind::TokenState state,
    const blind::CashType type,
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit,
    const std::uint64_t series,
    const Denomination denomination,
    const Time validFrom,
    const Time validTo,
    const VersionNumber version)
    : api_(api)
    , purse_(purse)
    , state_(state)
    , notary_(notary)
    , unit_(unit)
    , series_(series)
    , denomination_(denomination)
    , valid_from_(validFrom)
    , valid_to_(validTo)
    , type_(type)
    , version_(version)
{
}

Token::Token(const Token& rhs)
    : Token(
          rhs.api_,
          rhs.purse_,
          rhs.state_,
          rhs.type_,
          rhs.notary_,
          rhs.unit_,
          rhs.series_,
          rhs.denomination_,
          rhs.valid_from_,
          rhs.valid_to_,
          rhs.version_)
{
}

Token::Token(
    const api::Session& api,
    blind::internal::Purse& purse,
    const proto::Token& in)
    : Token(
          api,
          purse,
          translate(in.state()),
          translate(in.type()),
          identifier::Notary::Factory(in.notary()),
          identifier::UnitDefinition::Factory(in.mint()),
          in.series(),
          factory::Amount(in.denomination()),
          Clock::from_time_t(in.validfrom()),
          Clock::from_time_t(in.validto()),
          in.version())
{
}

Token::Token(
    const api::Session& api,
    blind::internal::Purse& purse,
    const VersionNumber version,
    const blind::TokenState state,
    const std::uint64_t series,
    const Denomination denomination,
    const Time validFrom,
    const Time validTo)
    : Token(
          api,
          purse,
          state,
          purse.Type(),
          purse.Notary(),
          purse.Unit(),
          series,
          denomination,
          validFrom,
          validTo,
          version)
{
}

auto Token::reencrypt(
    const crypto::key::Symmetric& oldKey,
    const PasswordPrompt& oldPassword,
    const crypto::key::Symmetric& newKey,
    const PasswordPrompt& newPassword,
    proto::Ciphertext& ciphertext) -> bool
{
    auto plaintext = Data::Factory();
    auto output =
        oldKey.Decrypt(ciphertext, oldPassword, plaintext->WriteInto());

    if (false == output) {
        LogError()(OT_PRETTY_CLASS())("Failed to decrypt ciphertext.").Flush();

        return false;
    }

    output = newKey.Encrypt(
        plaintext->Bytes(),
        newPassword,
        ciphertext,
        false,
        opentxs::crypto::key::symmetric::Algorithm::ChaCha20Poly1305);

    if (false == output) {
        LogError()(OT_PRETTY_CLASS())("Failed to encrypt ciphertext.").Flush();

        return false;
    }

    return output;
}

auto Token::Serialize(proto::Token& output) const noexcept -> bool
{
    output.set_version(version_);
    output.set_type(translate(type_));
    output.set_state(translate(state_));
    output.set_notary(notary_->str());
    output.set_mint(unit_->str());
    output.set_series(series_);
    denomination_.Serialize(writer(output.mutable_denomination()));
    output.set_validfrom(Clock::to_time_t(valid_from_));
    output.set_validto(Clock::to_time_t(valid_to_));

    return true;
}
}  // namespace opentxs::otx::blind::token

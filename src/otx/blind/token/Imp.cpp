// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/blind/token/Imp.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Token.pb.h>
#include <optional>
#include <string>

#include "internal/core/Factory.hpp"
#include "internal/crypto/symmetric/Key.hpp"
#include "internal/otx/blind/Purse.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/crypto/symmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/symmetric/Key.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/otx/blind/Types.internal.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::otx::blind::token
{
const opentxs::crypto::symmetric::Algorithm Token::mode_{
    opentxs::crypto::symmetric::Algorithm::ChaCha20Poly1305};

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
    const protobuf::Token& in)
    : Token(
          api,
          purse,
          opentxs::translate(in.state()),
          opentxs::translate(in.type()),
          api.Factory().NotaryIDFromBase58(in.notary()),
          api.Factory().UnitIDFromBase58(in.mint()),
          in.series(),
          factory::Amount(in.denomination()),
          seconds_since_epoch_unsigned(in.validfrom()).value(),
          seconds_since_epoch_unsigned(in.validto()).value(),
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
    const crypto::symmetric::Key& oldKey,
    const PasswordPrompt& oldPassword,
    const crypto::symmetric::Key& newKey,
    const PasswordPrompt& newPassword,
    protobuf::Ciphertext& ciphertext) -> bool
{
    auto plaintext = ByteArray{};
    auto output = oldKey.Internal().Decrypt(
        ciphertext, plaintext.WriteInto(), oldPassword);

    if (false == output) {
        LogError()()("Failed to decrypt ciphertext.").Flush();

        return false;
    }

    output = newKey.Internal().Encrypt(
        plaintext.Bytes(),
        opentxs::crypto::symmetric::Algorithm::ChaCha20Poly1305,
        ciphertext,
        newPassword,
        false);

    if (false == output) {
        LogError()()("Failed to encrypt ciphertext.").Flush();

        return false;
    }

    return output;
}

auto Token::Serialize(protobuf::Token& output) const noexcept -> bool
{
    output.set_version(version_);
    output.set_type(opentxs::translate(type_));
    output.set_state(opentxs::translate(state_));
    output.set_notary(notary_.asBase58(api_.Crypto()));
    output.set_mint(unit_.asBase58(api_.Crypto()));
    output.set_series(series_);
    denomination_.Serialize(writer(output.mutable_denomination()));
    output.set_validfrom(seconds_since_epoch(valid_from_).value());
    output.set_validto(seconds_since_epoch(valid_to_).value());

    return true;
}
}  // namespace opentxs::otx::blind::token

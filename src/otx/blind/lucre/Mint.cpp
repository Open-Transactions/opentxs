// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "otx/blind/lucre/Mint.hpp"  // IWYU pragma: associated

extern "C" {
#include <openssl/bio.h>
#include <openssl/bn.h>
#if __has_include(<openssl/types.h>)
// TODO openssl-3
#include <openssl/types.h>  // IWYU pragma: keep
#elif __has_include(<openssl/ossl_typ.h>)
#include <openssl/ossl_typ.h>  // IWYU pragma: keep
#endif
}

#include <array>
#include <limits>
#include <memory>
#include <utility>

#include "crypto/library/openssl/BIO.hpp"
#include "internal/otx/blind/Factory.hpp"
#include "internal/otx/blind/Token.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Envelope.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/blind/CashType.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/otx/blind/Token.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "otx/blind/lucre/Lucre.hpp"
#include "otx/blind/lucre/Token.hpp"
#include "otx/blind/mint/Imp.hpp"

namespace opentxs::factory
{
auto MintLucre(const api::Session& api) noexcept -> otx::blind::Mint
{
    using ReturnType = otx::blind::mint::Lucre;

    return std::make_unique<ReturnType>(api).release();
}

auto MintLucre(
    const api::Session& api,
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit) noexcept -> otx::blind::Mint
{
    using ReturnType = otx::blind::mint::Lucre;

    return std::make_unique<ReturnType>(api, notary, unit).release();
}

auto MintLucre(
    const api::Session& api,
    const identifier::Notary& notary,
    const identifier::Nym& serverNym,
    const identifier::UnitDefinition& unit) noexcept -> otx::blind::Mint
{
    using ReturnType = otx::blind::mint::Lucre;

    return std::make_unique<ReturnType>(api, notary, serverNym, unit).release();
}
}  // namespace opentxs::factory

namespace opentxs::otx::blind::mint
{
Lucre::Lucre(const api::Session& api)
    : Mint(api)
{
}

Lucre::Lucre(
    const api::Session& api,
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit)
    : Mint(api, notary, unit)
{
}

Lucre::Lucre(
    const api::Session& api,
    const identifier::Notary& notary,
    const identifier::Nym& serverNym,
    const identifier::UnitDefinition& unit)
    : Mint(api, notary, serverNym, unit)
{
}

// The mint has a different key pair for each denomination.
// Pass the actual denomination such as 5, 10, 20, 50, 100...
auto Lucre::AddDenomination(
    const identity::Nym& theNotary,
    const Amount& denomination,
    const std::size_t keySize,
    const PasswordPrompt& reason) -> bool
{
    if (std::numeric_limits<int>::max() < keySize) {
        LogError()(OT_PRETTY_CLASS())("Invalid key size").Flush();
        return false;
    }

    bool bReturnValue = false;
    const auto size = static_cast<int>(keySize);

    // Let's make sure it doesn't already exist
    auto theArmor = Armored::Factory();
    if (GetPublic(theArmor, denomination)) {
        LogError()(OT_PRETTY_CLASS())(
            "Error: Denomination public already exists in AddDenomination.")
            .Flush();
        return false;
    }
    if (GetPrivate(theArmor, denomination)) {
        LogError()(OT_PRETTY_CLASS())(
            "Error: Denomination private already exists in AddDenomination.")
            .Flush();
        return false;
    }

    if ((size / 8) < (MIN_COIN_LENGTH + DIGEST_LENGTH)) {
        LogError()(OT_PRETTY_CLASS())("Prime must be at least ")(
            (MIN_COIN_LENGTH + DIGEST_LENGTH) * 8)(" bits.")
            .Flush();
        return false;
    }

    if (size % 8) {
        LogError()(OT_PRETTY_CLASS())("Prime length must be a multiple of 8.")
            .Flush();
        return false;
    }

    auto setDumper = LucreDumper{};
    crypto::openssl::BIO bio = ::BIO_new(::BIO_s_mem());
    crypto::openssl::BIO bioPublic = ::BIO_new(::BIO_s_mem());

    // Generate the mint private key information
    Bank bank(size / 8);
    bank.WriteBIO(bio);

    // Generate the mint public key information
    PublicBank pbank(bank);
    pbank.WriteBIO(bioPublic);
    const auto strPrivateBank = bio.ToString();
    const auto strPublicBank = bioPublic.ToString();

    if (strPrivateBank->empty()) {
        LogError()(OT_PRETTY_CLASS())("Failed to generate private mint")
            .Flush();

        return false;
    }

    if (strPublicBank->empty()) {
        LogError()(OT_PRETTY_CLASS())("Failed to generate public mint").Flush();

        return false;
    }

    auto pPublic = Armored::Factory();
    auto pPrivate = Armored::Factory();

    // Set the public bank info onto pPublic
    pPublic->SetString(strPublicBank, true);  // linebreaks = true

    // Seal the private bank info up into an encrypted Envelope
    // and set it onto pPrivate
    auto envelope = api_.Factory().Envelope();
    envelope->Seal(theNotary, strPrivateBank->Bytes(), reason);
    // TODO check the return values on these twofunctions
    envelope->Armored(pPrivate);

    // Add the new key pair to the maps, using denomination as the key
    m_mapPublic.emplace(denomination, std::move(pPublic));
    m_mapPrivate.emplace(denomination, std::move(pPrivate));

    // Grab the Server Nym ID and save it with this Mint
    theNotary.GetIdentifier(m_ServerNymID);
    m_nDenominationCount++;
    bReturnValue = true;
    LogDetail()(OT_PRETTY_CLASS())("Successfully added denomination: ")(
        denomination)
        .Flush();

    return bReturnValue;
}

// Lucre step 3: the mint signs the token
//
auto Lucre::SignToken(
    const identity::Nym& notary,
    opentxs::otx::blind::Token& token,
    const PasswordPrompt& reason) -> bool
{
    auto setDumper = LucreDumper{};

    if (opentxs::otx::blind::CashType::Lucre != token.Type()) {
        LogError()(OT_PRETTY_CLASS())("Incorrect token type").Flush();

        return false;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Signing a lucre token").Flush();
    }

    auto* lucre = dynamic_cast<otx::blind::token::Lucre*>(&(token.Internal()));

    if (nullptr == lucre) {
        LogError()(OT_PRETTY_CLASS())("provided token is not a lucre token")
            .Flush();

        return false;
    }

    auto& lToken = *lucre;
    crypto::openssl::BIO bioBank = ::BIO_new(::BIO_s_mem());
    crypto::openssl::BIO bioRequest = ::BIO_new(::BIO_s_mem());
    crypto::openssl::BIO bioSignature = ::BIO_new(::BIO_s_mem());

    auto armoredPrivate = Armored::Factory();

    if (false == GetPrivate(armoredPrivate, lToken.Value())) {
        LogError()(OT_PRETTY_CLASS())("Failed to load private key").Flush();

        return false;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Loaded private mint key").Flush();
    }

    auto privateKey = String::Factory();

    try {
        auto envelope = api_.Factory().Envelope(armoredPrivate);

        if (false == envelope->Open(notary, privateKey->WriteInto(), reason)) {
            LogError()(OT_PRETTY_CLASS())("Failed to decrypt private key")
                .Flush();

            return false;
        } else {
            LogInsane()(OT_PRETTY_CLASS())("Decrypted private mint key")
                .Flush();
        }
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode ciphertext").Flush();

        return false;
    }

    ::BIO_puts(bioBank, privateKey->Get());
    Bank bank(bioBank);
    auto prototoken = String::Factory();

    if (false == lToken.GetPublicPrototoken(prototoken, reason)) {
        LogError()(OT_PRETTY_CLASS())("Failed to extract prototoken").Flush();

        return false;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Extracted prototoken").Flush();
    }

    ::BIO_puts(bioRequest, prototoken->Get());
    PublicCoinRequest req(bioRequest);
    BIGNUM* bnSignature = bank.SignRequest(req);

    if (nullptr == bnSignature) {
        LogError()(OT_PRETTY_CLASS())("Failed to sign prototoken").Flush();

        return false;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Signed prototoken").Flush();
    }

    req.WriteBIO(bioSignature);
    DumpNumber(bioSignature, "signature=", bnSignature);
    BN_free(bnSignature);
    auto sig_buf = std::array<char, 1024>{};
    auto sig_len = BIO_read(bioSignature, sig_buf.data(), 1023);
    sig_buf[sig_len] = '\0';

    if (0 == sig_len) {
        LogError()(OT_PRETTY_CLASS())("Failed to copy signature").Flush();

        return false;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Signature copied").Flush();
    }

    auto signature = String::Factory(sig_buf.data());

    if (false == lToken.AddSignature(signature)) {
        LogError()(OT_PRETTY_CLASS())("Failed to set signature").Flush();

        return false;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Signature serialized").Flush();
    }

    return true;
}

auto Lucre::VerifyToken(
    const identity::Nym& notary,
    const opentxs::otx::blind::Token& token,
    const PasswordPrompt& reason) -> bool
{

    if (opentxs::otx::blind::CashType::Lucre != token.Type()) {
        LogError()(OT_PRETTY_CLASS())("Incorrect token type").Flush();

        return false;
    }

    const auto* lucre =
        dynamic_cast<const otx::blind::token::Lucre*>(&(token.Internal()));

    if (nullptr == lucre) {
        LogError()(OT_PRETTY_CLASS())("provided token is not a lucre token")
            .Flush();

        return false;
    }

    const auto& lucreToken = *lucre;
    auto setDumper = LucreDumper{};
    crypto::openssl::BIO bioBank = ::BIO_new(::BIO_s_mem());
    crypto::openssl::BIO bioCoin = ::BIO_new(::BIO_s_mem());
    auto spendable = String::Factory();

    if (false == lucreToken.GetSpendable(spendable, reason)) {
        LogError()(OT_PRETTY_CLASS())("Failed to extract").Flush();

        return false;
    }

    ::BIO_puts(bioCoin, spendable->Get());
    auto armoredPrivate = Armored::Factory();
    GetPrivate(armoredPrivate, token.Value());
    auto privateKey = String::Factory();

    try {
        auto envelope = api_.Factory().Envelope(armoredPrivate);

        if (false == envelope->Open(notary, privateKey->WriteInto(), reason)) {
            LogError()(OT_PRETTY_CLASS())(
                ": Failed to decrypt private mint key")
                .Flush();

            return false;
        }
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode private mint key")
            .Flush();

        return false;
    }

    ::BIO_puts(bioBank, privateKey->Get());
    Bank bank(bioBank);
    Coin coin(bioCoin);

    return bank.Verify(coin);
}

Lucre::~Lucre() { Release_Mint(); }
}  // namespace opentxs::otx::blind::mint

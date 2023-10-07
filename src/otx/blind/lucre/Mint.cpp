// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/crypto/Envelope.hpp"
#include "internal/otx/blind/Factory.hpp"
#include "internal/otx/blind/Token.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/blind/CashType.hpp"  // IWYU pragma: keep
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/otx/blind/Token.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"
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
        LogError()()("Invalid key size").Flush();
        return false;
    }

    bool bReturnValue = false;
    const auto size = static_cast<int>(keySize);

    // Let's make sure it doesn't already exist
    auto theArmor = Armored::Factory(api_.Crypto());
    if (GetPublic(theArmor, denomination)) {
        LogError()()(
            "Error: Denomination public already exists in AddDenomination.")
            .Flush();
        return false;
    }
    if (GetPrivate(theArmor, denomination)) {
        LogError()()(
            "Error: Denomination private already exists in AddDenomination.")
            .Flush();
        return false;
    }

    if ((size / 8) < (MIN_COIN_LENGTH + DIGEST_LENGTH)) {
        LogError()()("Prime must be at least ")(
            (MIN_COIN_LENGTH + DIGEST_LENGTH) * 8)(" bits.")
            .Flush();
        return false;
    }

    if (size % 8) {
        LogError()()("Prime length must be a multiple of 8.").Flush();
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
        LogError()()("Failed to generate private mint").Flush();

        return false;
    }

    if (strPublicBank->empty()) {
        LogError()()("Failed to generate public mint").Flush();

        return false;
    }

    auto pPublic = Armored::Factory(api_.Crypto());
    auto pPrivate = Armored::Factory(api_.Crypto());

    // Set the public bank info onto pPublic
    pPublic->SetString(strPublicBank, true);  // linebreaks = true

    // Seal the private bank info up into an encrypted Envelope
    // and set it onto pPrivate
    auto envelope = api_.Factory().InternalSession().Envelope();
    envelope->Seal(theNotary, strPrivateBank->Bytes(), reason);
    // TODO check the return values on these twofunctions
    envelope->Armored(pPrivate);

    // Add the new key pair to the maps, using denomination as the key
    public_.emplace(denomination, std::move(pPublic));
    private_.emplace(denomination, std::move(pPrivate));

    // Grab the Server Nym ID and save it with this Mint
    theNotary.GetIdentifier(server_nym_id_);
    denomination_count_++;
    bReturnValue = true;
    LogDetail()()("Successfully added denomination: ")(denomination).Flush();

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
        LogError()()("Incorrect token type").Flush();

        return false;
    } else {
        LogInsane()()("Signing a lucre token").Flush();
    }

    auto* lucre = dynamic_cast<otx::blind::token::Lucre*>(&(token.Internal()));

    if (nullptr == lucre) {
        LogError()()("provided token is not a lucre token").Flush();

        return false;
    }

    auto& lToken = *lucre;
    crypto::openssl::BIO bioBank = ::BIO_new(::BIO_s_mem());
    crypto::openssl::BIO bioRequest = ::BIO_new(::BIO_s_mem());
    crypto::openssl::BIO bioSignature = ::BIO_new(::BIO_s_mem());

    auto armoredPrivate = Armored::Factory(api_.Crypto());

    if (false == GetPrivate(armoredPrivate, lToken.Value())) {
        LogError()()("Failed to load private key").Flush();

        return false;
    } else {
        LogInsane()()("Loaded private mint key").Flush();
    }

    auto privateKey = String::Factory();

    try {
        auto envelope =
            api_.Factory().InternalSession().Envelope(armoredPrivate);

        if (false == envelope->Open(notary, privateKey->WriteInto(), reason)) {
            LogError()()("Failed to decrypt private key").Flush();

            return false;
        } else {
            LogInsane()()("Decrypted private mint key").Flush();
        }
    } catch (...) {
        LogError()()("Failed to decode ciphertext").Flush();

        return false;
    }

    ::BIO_puts(bioBank, privateKey->Get());
    Bank bank(bioBank);
    auto prototoken = String::Factory();

    if (false == lToken.GetPublicPrototoken(prototoken, reason)) {
        LogError()()("Failed to extract prototoken").Flush();

        return false;
    } else {
        LogInsane()()("Extracted prototoken").Flush();
    }

    ::BIO_puts(bioRequest, prototoken->Get());
    PublicCoinRequest req(bioRequest);
    BIGNUM* bnSignature = bank.SignRequest(req);

    if (nullptr == bnSignature) {
        LogError()()("Failed to sign prototoken").Flush();

        return false;
    } else {
        LogInsane()()("Signed prototoken").Flush();
    }

    req.WriteBIO(bioSignature);
    DumpNumber(bioSignature, "signature=", bnSignature);
    BN_free(bnSignature);
    auto sig_buf = std::array<char, 1024>{};
    auto sig_len = BIO_read(bioSignature, sig_buf.data(), 1023);
    sig_buf[sig_len] = '\0';

    if (0 == sig_len) {
        LogError()()("Failed to copy signature").Flush();

        return false;
    } else {
        LogInsane()()("Signature copied").Flush();
    }

    auto signature = String::Factory(sig_buf.data());

    if (false == lToken.AddSignature(signature)) {
        LogError()()("Failed to set signature").Flush();

        return false;
    } else {
        LogInsane()()("Signature serialized").Flush();
    }

    return true;
}

auto Lucre::VerifyToken(
    const identity::Nym& notary,
    const opentxs::otx::blind::Token& token,
    const PasswordPrompt& reason) -> bool
{

    if (opentxs::otx::blind::CashType::Lucre != token.Type()) {
        LogError()()("Incorrect token type").Flush();

        return false;
    }

    const auto* lucre =
        dynamic_cast<const otx::blind::token::Lucre*>(&(token.Internal()));

    if (nullptr == lucre) {
        LogError()()("provided token is not a lucre token").Flush();

        return false;
    }

    const auto& lucreToken = *lucre;
    auto setDumper = LucreDumper{};
    crypto::openssl::BIO bioBank = ::BIO_new(::BIO_s_mem());
    crypto::openssl::BIO bioCoin = ::BIO_new(::BIO_s_mem());
    auto spendable = String::Factory();

    if (false == lucreToken.GetSpendable(spendable, reason)) {
        LogError()()("Failed to extract").Flush();

        return false;
    }

    ::BIO_puts(bioCoin, spendable->Get());
    auto armoredPrivate = Armored::Factory(api_.Crypto());
    GetPrivate(armoredPrivate, token.Value());
    auto privateKey = String::Factory();

    try {
        auto envelope =
            api_.Factory().InternalSession().Envelope(armoredPrivate);

        if (false == envelope->Open(notary, privateKey->WriteInto(), reason)) {
            LogError()()(": Failed to decrypt private mint key").Flush();

            return false;
        }
    } catch (...) {
        LogError()()("Failed to decode private mint key").Flush();

        return false;
    }

    ::BIO_puts(bioBank, privateKey->Get());
    Bank bank(bioBank);
    Coin coin(bioCoin);

    return bank.Verify(coin);
}

Lucre::~Lucre() { Release_Mint(); }
}  // namespace opentxs::otx::blind::mint

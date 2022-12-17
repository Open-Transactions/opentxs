// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/blind/lucre/Token.hpp"  // IWYU pragma: associated

extern "C" {
#include <openssl/bio.h>
#include <openssl/bn.h>
}

#include <Ciphertext.pb.h>
#include <LucreTokenData.pb.h>
#include <Token.pb.h>
#include <algorithm>
#include <cctype>
#include <limits>
#include <regex>
#include <stdexcept>

#include "crypto/library/openssl/BIO.hpp"
#include "crypto/library/openssl/OpenSSL.hpp"
#include "internal/core/Armored.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/String.hpp"
#include "internal/crypto/symmetric/Key.hpp"
#include "internal/otx/blind/Factory.hpp"
#include "internal/otx/blind/Purse.hpp"
#include "internal/otx/blind/Token.hpp"
#include "internal/otx/blind/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/crypto/symmetric/Key.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/otx/blind/Token.hpp"
#include "opentxs/otx/blind/TokenState.hpp"  // IWYU pragma: keep
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Writer.hpp"
#include "otx/blind/lucre/Lucre.hpp"

#define LUCRE_TOKEN_VERSION 1

namespace opentxs::factory
{
auto TokenLucre(
    const otx::blind::Token& token,
    otx::blind::internal::Purse& purse) noexcept -> otx::blind::Token
{
    using ReturnType = otx::blind::token::Lucre;
    const auto* lucre = dynamic_cast<const ReturnType*>(&(token.Internal()));

    if (nullptr == lucre) {
        LogError()("opentxs::factory::")(__func__)(": wrong token type")
            .Flush();

        return {};
    }

    return std::make_unique<ReturnType>(*lucre, purse).release();
}

auto TokenLucre(
    const api::Session& api,
    otx::blind::internal::Purse& purse,
    const proto::Token& serialized) noexcept -> otx::blind::Token
{
    using ReturnType = otx::blind::token::Lucre;

    return std::make_unique<ReturnType>(api, purse, serialized).release();
}

auto TokenLucre(
    const api::Session& api,
    const identity::Nym& owner,
    const otx::blind::Mint& mint,
    const otx::blind::Denomination value,
    otx::blind::internal::Purse& purse,
    const opentxs::PasswordPrompt& reason) noexcept -> otx::blind::Token
{
    using ReturnType = otx::blind::token::Lucre;

    return std::make_unique<ReturnType>(api, owner, mint, value, purse, reason)
        .release();
}
}  // namespace opentxs::factory

namespace opentxs::otx::blind::token
{
Lucre::Lucre(
    const api::Session& api,
    blind::internal::Purse& purse,
    const VersionNumber version,
    const blind::TokenState state,
    const std::uint64_t series,
    const Denomination value,
    const Time validFrom,
    const Time validTo,
    const String& signature,
    std::shared_ptr<proto::Ciphertext> publicKey,
    std::shared_ptr<proto::Ciphertext> privateKey,
    std::shared_ptr<proto::Ciphertext> spendable)
    : Token(
          api,
          purse,
          OT_TOKEN_VERSION,
          state,
          series,
          value,
          validFrom,
          validTo)
    , lucre_version_(version)
    , signature_(signature)
    , private_(publicKey)
    , public_(privateKey)
    , spend_(spendable)
{
}

Lucre::Lucre(const Lucre& rhs)
    : Lucre(
          rhs.api_,
          rhs.purse_,
          rhs.lucre_version_,
          rhs.state_,
          rhs.series_,
          rhs.denomination_,
          rhs.valid_from_,
          rhs.valid_to_,
          rhs.signature_,
          rhs.private_,
          rhs.public_,
          rhs.spend_)
{
}

Lucre::Lucre(const Lucre& rhs, blind::internal::Purse& newOwner)
    : Lucre(
          rhs.api_,
          newOwner,
          rhs.lucre_version_,
          rhs.state_,
          rhs.series_,
          rhs.denomination_,
          rhs.valid_from_,
          rhs.valid_to_,
          rhs.signature_,
          rhs.private_,
          rhs.public_,
          rhs.spend_)
{
}

Lucre::Lucre(
    const api::Session& api,
    blind::internal::Purse& purse,
    const proto::Token& in)
    : Lucre(
          api,
          purse,
          in.lucre().version(),
          translate(in.state()),
          in.series(),
          factory::Amount(in.denomination()),
          convert_stime(in.validfrom()),
          convert_stime(in.validto()),
          String::Factory(),
          nullptr,
          nullptr,
          nullptr)
{
    const auto& lucre = in.lucre();
    OT_ASSERT(
        std::numeric_limits<std::uint32_t>::max() >= lucre.signature().size());

    if (lucre.has_signature()) {
        LogInsane()(OT_PRETTY_CLASS())("This token has a signature").Flush();
        signature_->Set(
            lucre.signature().data(),
            static_cast<std::uint32_t>(lucre.signature().size()));
    } else {
        LogInsane()(OT_PRETTY_CLASS())("This token does not have a signature")
            .Flush();
    }

    if (lucre.has_privateprototoken()) {
        LogInsane()(OT_PRETTY_CLASS())("This token has a private prototoken")
            .Flush();
        private_ =
            std::make_shared<proto::Ciphertext>(lucre.privateprototoken());
    } else {
        LogInsane()(OT_PRETTY_CLASS())(
            "This token does not have a private prototoken")
            .Flush();
    }

    if (lucre.has_publicprototoken()) {
        LogInsane()(OT_PRETTY_CLASS())("This token has a public prototoken")
            .Flush();
        public_ = std::make_shared<proto::Ciphertext>(lucre.publicprototoken());
    } else {
        LogInsane()(OT_PRETTY_CLASS())(
            "This token does not have a public prototoken")
            .Flush();
    }

    if (lucre.has_spendable()) {
        LogInsane()(OT_PRETTY_CLASS())("This token has a spendable string")
            .Flush();
        spend_ = std::make_shared<proto::Ciphertext>(lucre.spendable());
    } else {
        LogInsane()(OT_PRETTY_CLASS())(
            "This token does not have a spendable string")
            .Flush();
    }
}

Lucre::Lucre(
    const api::Session& api,
    const identity::Nym& owner,
    const Mint& mint,
    const Denomination value,
    blind::internal::Purse& purse,
    const PasswordPrompt& reason)
    : Lucre(
          api,
          purse,
          LUCRE_TOKEN_VERSION,
          blind::TokenState::Blinded,
          mint.GetSeries(),
          value,
          mint.GetValidFrom(),
          mint.GetValidTo(),
          String::Factory(),
          nullptr,
          nullptr,
          nullptr)
{
    const bool generated = GenerateTokenRequest(owner, mint, reason);

    if (false == generated) {
        throw std::runtime_error("Failed to generate prototoken");
    }
}

auto Lucre::AddSignature(const String& signature) -> bool
{
    if (signature.empty()) {
        LogError()(OT_PRETTY_CLASS())("Missing signature").Flush();

        return false;
    }

    signature_ = signature;
    state_ = blind::TokenState::Signed;

    return true;
}

auto Lucre::ChangeOwner(
    blind::internal::Purse& oldOwner,
    blind::internal::Purse& newOwner,
    const PasswordPrompt& reason) -> bool
{
    // NOTE: private_ is never re-encrypted

    auto oldPass = api_.Factory().PasswordPrompt(reason);
    auto newPass = api_.Factory().PasswordPrompt(reason);
    auto& oldKey = oldOwner.PrimaryKey(oldPass);
    auto& newKey = newOwner.PrimaryKey(newPass);

    if (public_) {
        if (false == reencrypt(oldKey, oldPass, newKey, newPass, *public_)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed to re-encrypt public prototoken")
                .Flush();

            return false;
        }
    }

    if (spend_) {
        if (false == reencrypt(oldKey, oldPass, newKey, newPass, *spend_)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed to re-encrypt spendable token")
                .Flush();

            return false;
        }
    }

    return true;
}

auto Lucre::GenerateTokenRequest(
    const identity::Nym& owner,
    const Mint& mint,
    const PasswordPrompt& reason) -> bool
{
    auto setDumper = LucreDumper{};
    crypto::openssl::BIO bioBank = ::BIO_new(::BIO_s_mem());
    auto armoredMint = Armored::Factory();
    mint.GetPublic(armoredMint, denomination_);
    auto serializedMint = String::Factory(armoredMint);

    if (serializedMint->empty()) {
        LogError()(OT_PRETTY_CLASS())("Failed to get public mint for series ")(
            denomination_)
            .Flush();

        return false;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Begin mint series ")(denomination_)
            .Flush();
        LogInsane()(serializedMint.get()).Flush();
        LogInsane()(OT_PRETTY_CLASS())("End mint").Flush();
    }

    ::BIO_puts(bioBank, serializedMint->Get());
    PublicBank bank;
    bank.ReadBIO(bioBank);
    crypto::openssl::BIO bioCoin = ::BIO_new(::BIO_s_mem());
    crypto::openssl::BIO bioPublicCoin = ::BIO_new(::BIO_s_mem());
    CoinRequest req(bank);
    req.WriteBIO(bioCoin);
    static_cast<PublicCoinRequest*>(&req)->WriteBIO(bioPublicCoin);
    const auto strPrivateCoin = bioCoin.ToString();
    const auto strPublicCoin = bioPublicCoin.ToString();

    if (strPrivateCoin->empty()) {
        LogError()(OT_PRETTY_CLASS())("Failed to generate private prototoken")
            .Flush();

        return false;
    }

    if (strPublicCoin->empty()) {
        LogError()(OT_PRETTY_CLASS())("Failed to generate public prototoken")
            .Flush();

        return false;
    }

    private_ = std::make_shared<proto::Ciphertext>();
    public_ = std::make_shared<proto::Ciphertext>();

    if (false == bool(private_)) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed to instantiate private prototoken")
            .Flush();

        return false;
    }

    if (false == bool(public_)) {
        LogError()(OT_PRETTY_CLASS())("Failed to instantiate public prototoken")
            .Flush();

        return false;
    }

    {
        auto password = api_.Factory().PasswordPrompt(reason);
        const auto encryptedPrivate =
            purse_.SecondaryKey(owner, password)
                .Internal()
                .Encrypt(
                    strPrivateCoin->Bytes(), mode_, *private_, password, false);

        if (false == bool(encryptedPrivate)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed to encrypt private prototoken")
                .Flush();

            return false;
        }
    }

    {
        auto password = api_.Factory().PasswordPrompt(reason);
        const auto encryptedPublic =
            purse_.PrimaryKey(password).Internal().Encrypt(
                strPublicCoin->Bytes(), mode_, *public_, password, false);

        if (false == bool(encryptedPublic)) {
            LogError()(OT_PRETTY_CLASS())("Failed to encrypt public prototoken")
                .Flush();

            return false;
        }
    }

    return true;
}

auto Lucre::GetPublicPrototoken(String& output, const PasswordPrompt& reason)
    -> bool
{
    if (false == bool(public_)) {
        LogError()(OT_PRETTY_CLASS())("Missing public prototoken").Flush();

        return false;
    }

    auto& ciphertext = *public_;
    bool decrypted{false};

    try {
        auto password = api_.Factory().PasswordPrompt(reason);
        decrypted = purse_.PrimaryKey(password).Internal().Decrypt(
            ciphertext, output.WriteInto(), password);
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Missing primary key").Flush();

        return false;
    }

    if (false == decrypted) {
        LogError()(OT_PRETTY_CLASS())("Failed to decrypt prototoken").Flush();
    }

    return decrypted;
}

auto Lucre::GetSpendable(String& output, const PasswordPrompt& reason) const
    -> bool
{
    if (false == bool(spend_)) {
        LogError()(OT_PRETTY_CLASS())("Missing spendable token").Flush();

        return false;
    }

    auto& ciphertext = *spend_;
    bool decrypted{false};

    try {
        auto password = api_.Factory().PasswordPrompt(reason);
        decrypted = purse_.PrimaryKey(password).Internal().Decrypt(
            ciphertext, output.WriteInto(), password);
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Missing primary key").Flush();

        return false;
    }

    if (false == decrypted) {
        LogError()(OT_PRETTY_CLASS())("Failed to decrypt spendable token")
            .Flush();
    }

    return decrypted;
}

auto Lucre::ID(const PasswordPrompt& reason) const -> UnallocatedCString
{
    auto spendable = String::Factory();

    if (false == GetSpendable(spendable, reason)) {
        LogError()(OT_PRETTY_CLASS())("Missing spendable string").Flush();

        return {};
    }

    UnallocatedCString output;
    std::regex reg("id=([A-Z0-9]*)");
    std::cmatch match{};

    if (std::regex_search(spendable->Get(), match, reg)) { output = match[1]; }

    std::transform(output.begin(), output.end(), output.begin(), [](char c) {
        return (std::toupper(c));
    });

    return output;
}

auto Lucre::IsSpent(const PasswordPrompt& reason) const -> bool
{
    switch (state_) {
        case blind::TokenState::Spent: {
            return true;
        }
        case blind::TokenState::Blinded:
        case blind::TokenState::Signed:
        case blind::TokenState::Expired: {
            return false;
        }
        case blind::TokenState::Ready: {
            break;
        }
        case blind::TokenState::Error:
        default: {
            throw std::runtime_error("invalid token state");
        }
    }

    const auto id = ID(reason);

    if (id.empty()) {
        throw std::runtime_error("failed to calculate token ID");
    }

    return api_.Storage().CheckTokenSpent(notary_, unit_, series_, id);
}

auto Lucre::MarkSpent(const PasswordPrompt& reason) -> bool
{
    if (blind::TokenState::Ready != state_) {
        throw std::runtime_error("invalid token state");
    }

    bool output{false};
    const auto id = ID(reason);

    if (id.empty()) {
        throw std::runtime_error("failed to calculate token ID");
    }

    try {
        output = api_.Storage().MarkTokenSpent(notary_, unit_, series_, id);
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Failed to load spendable token").Flush();
    }

    if (output) { state_ = blind::TokenState::Spent; }

    return output;
}

auto Lucre::Process(
    const identity::Nym& owner,
    const Mint& mint,
    const PasswordPrompt& reason) -> bool
{
    if (blind::TokenState::Signed != state_) {
        LogError()(OT_PRETTY_CLASS())("Incorrect token state.").Flush();

        return false;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Processing signed token").Flush();
    }

    if (signature_->empty()) {
        LogError()(OT_PRETTY_CLASS())("Missing signature").Flush();

        return false;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Loaded signature").Flush();
    }

    if (false == bool(private_)) {
        LogError()(OT_PRETTY_CLASS())("Missing encrypted prototoken").Flush();

        return false;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Loaded encrypted prototoken").Flush();
    }

    auto setDumper = LucreDumper{};
    auto bioBank = crypto::OpenSSL_BIO{::BIO_new(::BIO_s_mem()), ::BIO_free};
    auto bioSignature =
        crypto::OpenSSL_BIO{::BIO_new(::BIO_s_mem()), ::BIO_free};
    auto bioPrivateRequest =
        crypto::OpenSSL_BIO{::BIO_new(::BIO_s_mem()), ::BIO_free};
    auto bioCoin = crypto::openssl::BIO{::BIO_new(::BIO_s_mem())};
    auto armoredMint = Armored::Factory();
    mint.GetPublic(armoredMint, denomination_);
    auto serializedMint = String::Factory(armoredMint);

    if (serializedMint->empty()) {
        LogError()(OT_PRETTY_CLASS())("Failed to get public mint for series ")(
            denomination_)
            .Flush();

        return false;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Begin mint series ")(denomination_)
            .Flush();
        LogInsane()(serializedMint.get()).Flush();
        LogInsane()(OT_PRETTY_CLASS())("End mint").Flush();
    }

    ::BIO_puts(bioBank.get(), serializedMint->Get());
    ::BIO_puts(bioSignature.get(), signature_->Get());
    auto prototoken = String::Factory();

    try {
        auto password = api_.Factory().PasswordPrompt(reason);
        const auto& key = purse_.SecondaryKey(owner, password);
        const auto decrypted = key.Internal().Decrypt(
            *private_, prototoken->WriteInto(), password);

        if (false == decrypted) {
            LogError()(OT_PRETTY_CLASS())("Failed to decrypt prototoken")
                .Flush();

            return false;
        } else {
            LogInsane()(OT_PRETTY_CLASS())("Prototoken decrypted").Flush();
        }
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Failed to get secondary key.").Flush();

        return false;
    }

    if (prototoken->empty()) {
        LogError()(OT_PRETTY_CLASS())("Missing prototoken").Flush();

        return false;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Prototoken ready:").Flush();
    }

    ::BIO_puts(bioPrivateRequest.get(), prototoken->Get());
    PublicBank bank(bioBank.get());
    CoinRequest req(bioPrivateRequest.get());
    using BN = crypto::OpenSSL_BN;
    auto bnRequest =
        BN{::ReadNumber(bioSignature.get(), "request="), ::BN_free};
    auto bnSignature =
        BN{::ReadNumber(bioSignature.get(), "signature="), ::BN_free};
    DumpNumber("signature=", bnSignature.get());
    Coin coin;
    req.ProcessResponse(&coin, bank, bnSignature.get());
    coin.WriteBIO(bioCoin);
    const auto spend = bioCoin.ToString();

    if (spend->empty()) {
        LogError()(OT_PRETTY_CLASS())("Failed to read token").Flush();

        return false;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Obtained spendable token").Flush();
    }

    spend_ = std::make_shared<proto::Ciphertext>();

    if (false == bool(spend_)) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed to instantiate spendable ciphertext")
            .Flush();

        return false;
    }

    try {
        auto password = api_.Factory().PasswordPrompt(reason);
        auto& key = purse_.PrimaryKey(password);
        const auto encrypted = key.Internal().Encrypt(
            spend->Bytes(), mode_, *spend_, password, false);

        if (false == encrypted) {
            LogError()(OT_PRETTY_CLASS())("Failed to encrypt spendable token")
                .Flush();

            return false;
        }
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Failed to get primary key.").Flush();

        return false;
    }

    state_ = blind::TokenState::Ready;
    private_.reset();
    public_.reset();

    return true;
}

auto Lucre::Serialize(proto::Token& output) const noexcept -> bool
{
    if (false == Token::Serialize(output)) { return false; }

    try {
        auto& lucre = *output.mutable_lucre();
        lucre.set_version(lucre_version_);

        switch (state_) {
            case blind::TokenState::Blinded: {
                serialize_private(lucre);
                serialize_public(lucre);
            } break;
            case blind::TokenState::Signed: {
                serialize_private(lucre);
                serialize_public(lucre);
                serialize_signature(lucre);
            } break;
            case blind::TokenState::Ready:
            case blind::TokenState::Spent: {
                serialize_spendable(lucre);
            } break;
            case blind::TokenState::Expired: {
                if (false == signature_->empty()) {
                    serialize_signature(lucre);
                }

                if (private_) { serialize_private(lucre); }

                if (public_) { serialize_public(lucre); }

                if (spend_) { serialize_spendable(lucre); }
            } break;
            case blind::TokenState::Error:
            default: {
                throw std::runtime_error("invalid token state");
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }

    return true;
}

void Lucre::serialize_private(proto::LucreTokenData& lucre) const
{
    if (false == bool(private_)) {
        throw std::runtime_error("missing private prototoken");
    }

    *lucre.mutable_privateprototoken() = *private_;
}

void Lucre::serialize_public(proto::LucreTokenData& lucre) const
{
    if (false == bool(public_)) {
        throw std::runtime_error("missing public prototoken");
    }

    *lucre.mutable_publicprototoken() = *public_;
}

void Lucre::serialize_signature(proto::LucreTokenData& lucre) const
{
    if (signature_->empty()) { throw std::runtime_error("missing signature"); }

    lucre.set_signature(signature_->Get(), signature_->GetLength());
}

void Lucre::serialize_spendable(proto::LucreTokenData& lucre) const
{
    if (false == bool(spend_)) {
        throw std::runtime_error("missing spendable token");
    }

    *lucre.mutable_spendable() = *spend_;
}
}  // namespace opentxs::otx::blind::token

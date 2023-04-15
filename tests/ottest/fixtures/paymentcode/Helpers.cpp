// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <exception>
#include <stdexcept>
#include <utility>

#include "internal/util/LogMacros.hpp"

namespace ottest
{
PaymentCodeFixture PC_Fixture_Base::user_1_{};
PaymentCodeFixture PC_Fixture_Base::user_2_{};

auto PaymentCodeFixture::bip44_path(
    const ot::api::session::Client& api,
    const ot::blockchain::Type chain,
    ot::Writer&& destination) const -> bool
{
    try {
        if (false == seed_.has_value()) {
            throw std::runtime_error("missing seed");
        }

        if (false == api.Crypto().Blockchain().Bip44Path(
                         chain, seed_.value(), std::move(destination))) {
            throw std::runtime_error("missing path");
        }

        return true;
    } catch (const std::exception& e) {
        ot::LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        std::rethrow_exception(std::current_exception());
    }
}

auto PaymentCodeFixture::blinding_key_public()
    -> const ot::crypto::asymmetric::key::EllipticCurve&
{
    try {
        if (false == blind_key_secret_.has_value()) {
            throw std::runtime_error("missing private key");
        }

        auto& var = [&]() -> auto& {
            if (false == blind_key_public_.has_value()) {

                return blind_key_public_.emplace();
            } else {

                return *blind_key_public_;
            }
        }();

        if (false == var.IsValid()) {
            var = blind_key_secret_->asPublic().asEllipticCurve();
        }

        if (false == var.IsValid()) {
            throw std::runtime_error("failed to calculate public blinding key");
        }

        return var;
    } catch (const std::exception& e) {
        ot::LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        std::rethrow_exception(std::current_exception());
    }
}

auto PaymentCodeFixture::blinding_key_secret(
    const ot::api::session::Client& api,
    const ot::blockchain::Type chain,
    const ot::PasswordPrompt& reason)
    -> const ot::crypto::asymmetric::key::EllipticCurve&
{
    try {
        auto& var = [&]() -> auto& {
            if (false == blind_key_secret_.has_value()) {

                return blind_key_secret_.emplace();
            } else {

                return *blind_key_secret_;
            }
        }();

        if (false == var.IsValid()) {
            auto bytes = ot::Space{};
            if (false == bip44_path(api, chain, ot::writer(bytes))) {
                throw std::runtime_error("Failed");
            }
            var = api.Crypto().Seed().AccountChildKey(
                ot::reader(bytes), ot::INTERNAL_CHAIN, index_, reason);
        }

        if (false == var.IsValid()) {
            throw std::runtime_error("failed to derive secret blinding key");
        }

        return var;
    } catch (const std::exception& e) {
        ot::LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        std::rethrow_exception(std::current_exception());
    }
}

auto PaymentCodeFixture::blinding_key_secret(
    const ot::api::session::Client& api,
    const ot::UnallocatedCString& privateKey,
    const ot::PasswordPrompt& reason)
    -> const ot::crypto::asymmetric::key::EllipticCurve&
{
    try {
        auto& var = [&]() -> auto& {
            if (false == blind_key_secret_.has_value()) {

                return blind_key_secret_.emplace();
            } else {

                return *blind_key_secret_;
            }
        }();

        if (false == var.IsValid()) {
            const auto decoded = api.Factory().DataFromHex(privateKey);
            const auto key = api.Factory().SecretFromBytes(decoded.Bytes());
            var =
                api.Crypto().Asymmetric().InstantiateSecp256k1Key(key, reason);
        }

        if (false == var.IsValid()) {
            throw std::runtime_error(
                "failed to instantiate secret blinding key");
        }

        return var;
    } catch (const std::exception& e) {
        ot::LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        std::rethrow_exception(std::current_exception());
    }
}

auto PaymentCodeFixture::cleanup() -> void
{
    blind_key_public_ = std::nullopt;
    blind_key_secret_ = std::nullopt;
    pc_public_ = std::nullopt;
    pc_secret_ = std::nullopt;
    seed_ = std::nullopt;
}

auto PaymentCodeFixture::payment_code_public(
    const ot::api::Session& api,
    const ot::UnallocatedCString& base58) -> const ot::PaymentCode&
{
    try {
        auto& var = pc_public_;

        if (false == var.has_value()) {
            var.emplace(api.Factory().PaymentCode(base58));
        }

        if (false == var.has_value()) {
            throw std::runtime_error("failed to deserialize payment code");
        }

        return var.value();
    } catch (const std::exception& e) {
        ot::LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        std::rethrow_exception(std::current_exception());
    }
}

auto PaymentCodeFixture::payment_code_secret(
    const ot::api::Session& api,
    const std::uint8_t version,
    const ot::PasswordPrompt& reason) -> const ot::PaymentCode&
{
    try {
        if (false == seed_.has_value()) {
            throw std::runtime_error("missing seed");
        }

        auto& var = pc_secret_;

        if (false == var.has_value()) {
            var.emplace(api.Factory().PaymentCode(
                seed_.value(), account_, version, reason));
        }

        if (false == var.has_value()) {
            throw std::runtime_error("failed to derive payment code");
        }

        return var.value();
    } catch (const std::exception& e) {
        ot::LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        std::rethrow_exception(std::current_exception());
    }
}

auto PaymentCodeFixture::seed(
    const ot::api::Session& api,
    const std::string_view wordList,
    const ot::PasswordPrompt& reason) -> const ot::UnallocatedCString&
{
    try {
        auto& var = seed_;

        if (false == var.has_value()) {
            const auto words = api.Factory().SecretFromText(wordList);
            const auto phrase = api.Factory().Secret(0);
            var.emplace(api.Crypto().Seed().ImportSeed(
                words,
                phrase,
                ot::crypto::SeedStyle::BIP39,
                ot::crypto::Language::en,
                reason));
        }

        if (false == var.has_value()) {
            throw std::runtime_error("failed to import seed");
        }

        return var.value();
    } catch (const std::exception& e) {
        ot::LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        std::rethrow_exception(std::current_exception());
    }
}

PC_Fixture_Base::PC_Fixture_Base(
    const std::uint8_t aliceVersion,
    const std::uint8_t bobVersion,
    const ot::UnallocatedCString& aliceBip39,
    const ot::UnallocatedCString& bobBip39,
    const ot::UnallocatedCString& aliceExpectedPC,
    const ot::UnallocatedCString& bobExpectedPC) noexcept
    : api_(ot::Context().StartClientSession(0))
    , reason_(api_.Factory().PasswordPrompt(__func__))
    , alice_seed_(user_1_.seed(api_, aliceBip39, reason_))
    , bob_seed_(user_2_.seed(api_, bobBip39, reason_))
    , alice_pc_secret_(user_1_.payment_code_secret(api_, aliceVersion, reason_))
    , alice_pc_public_(user_1_.payment_code_public(api_, aliceExpectedPC))
    , bob_pc_secret_(user_2_.payment_code_secret(api_, bobVersion, reason_))
    , bob_pc_public_(user_2_.payment_code_public(api_, bobExpectedPC))
{
}

auto PC_Fixture_Base::Shutdown() noexcept -> void
{
    user_1_.cleanup();
    user_2_.cleanup();
}
}  // namespace ottest

// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/crypto/Envelope.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <memory>
#include <string_view>
#include <utility>

#include "internal/core/String.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/opentxs.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
namespace ot = opentxs;
using namespace opentxs::literals;
using namespace std::literals;

bool Envelope::init_{false};
const bool Envelope::have_rsa_{
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::Legacy)};
const bool Envelope::have_secp256k1_{
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::Secp256k1)};
const bool Envelope::have_ed25519_{
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::ED25519)};
Envelope::Nyms Envelope::nyms_{};
const Envelope::Expected Envelope::expected_{
    {false, {false, false, false}},
    {true, {true, false, false}},
    {true, {false, true, false}},
    {true, {true, true, false}},
    {true, {false, false, true}},
    {true, {true, false, true}},
    {true, {false, true, true}},
    {true, {true, true, true}},
};

auto Envelope::can_seal(const std::size_t row) -> bool
{
    return expected_.at(row).first;
}
auto Envelope::can_open(const std::size_t row, const std::size_t column) -> bool
{
    return can_seal(row) && should_seal(row, column);
}
auto Envelope::is_active(const std::size_t row, const std::size_t column)
    -> bool
{
    return 0 != (row & (1_uz << column));
}
auto Envelope::should_seal(const std::size_t row, const std::size_t column)
    -> bool
{
    return expected_.at(row).second.at(column);
}

Envelope::Envelope()
    : sender_(OTTestEnvironment::GetOT().StartClientSession(0))
    , recipient_(OTTestEnvironment::GetOT().StartClientSession(1))
    , reason_s_(sender_.Factory().PasswordPrompt(__func__))
    , reason_r_(recipient_.Factory().PasswordPrompt(__func__))
    , plaintext_(
          ot::String::Factory("The quick brown fox jumped over the lazy dog"))
{
    if (false == init_) {
        init_ = true;
        {
            auto params = [this] {
                using Type = ot::crypto::ParameterType;

                if (have_ed25519_) {

                    return ot::crypto::Parameters{
                        recipient_.Factory(), Type::ed25519};
                } else if (have_secp256k1_) {

                    return ot::crypto::Parameters{
                        recipient_.Factory(), Type::secp256k1};
                } else if (have_rsa_) {

                    return ot::crypto::Parameters{
                        recipient_.Factory(), Type::rsa};
                } else {

                    return ot::crypto::Parameters{recipient_.Factory()};
                }
            }();
            auto rNym = recipient_.Wallet().Nym(params, reason_r_, "");

            OT_ASSERT(rNym);

            auto bytes = ot::Space{};
            OT_ASSERT(rNym->Serialize(ot::writer(bytes)));
            nyms_.emplace_back(sender_.Wallet().Nym(ot::reader(bytes)));

            OT_ASSERT(bool(*nyms_.crbegin()));
        }
        {
            auto params = [this] {
                using Type = ot::crypto::ParameterType;

                if (have_secp256k1_) {

                    return ot::crypto::Parameters{
                        recipient_.Factory(), Type::secp256k1};
                } else if (have_rsa_) {

                    return ot::crypto::Parameters{
                        recipient_.Factory(), Type::rsa};
                } else if (have_ed25519_) {

                    return ot::crypto::Parameters{
                        recipient_.Factory(), Type::ed25519};
                } else {

                    return ot::crypto::Parameters{recipient_.Factory()};
                }
            }();
            auto rNym = recipient_.Wallet().Nym(params, reason_r_, "");

            OT_ASSERT(rNym);

            auto bytes = ot::Space{};
            OT_ASSERT(rNym->Serialize(ot::writer(bytes)));
            nyms_.emplace_back(sender_.Wallet().Nym(ot::reader(bytes)));

            OT_ASSERT(bool(*nyms_.crbegin()));
        }
        {
            auto params = [this] {
                using Type = ot::crypto::ParameterType;

                if (have_rsa_) {

                    return ot::crypto::Parameters{
                        recipient_.Factory(), Type::rsa};
                } else if (have_ed25519_) {

                    return ot::crypto::Parameters{
                        recipient_.Factory(), Type::ed25519};
                } else if (have_secp256k1_) {

                    return ot::crypto::Parameters{
                        recipient_.Factory(), Type::secp256k1};
                } else {

                    return ot::crypto::Parameters{recipient_.Factory()};
                }
            }();
            auto rNym = recipient_.Wallet().Nym(params, reason_r_, "");

            OT_ASSERT(rNym);

            auto bytes = ot::Space{};
            OT_ASSERT(rNym->Serialize(ot::writer(bytes)));
            nyms_.emplace_back(sender_.Wallet().Nym(ot::reader(bytes)));

            OT_ASSERT(bool(*nyms_.crbegin()));
        }
    }
}
}  // namespace ottest

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "internal/core/Factory.hpp"  // IWYU pragma: associated

#include <PaymentCode.pb.h>
#include <array>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <memory>
#include <utility>

#include "core/paymentcode/Imp.hpp"
#include "core/paymentcode/Preimage.hpp"
#include "internal/crypto/key/Factory.hpp"
#include "internal/crypto/key/Null.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs::factory
{
auto PaymentCode(
    const api::Session& api,
    const UnallocatedCString& base58) noexcept -> opentxs::PaymentCode
{
    const auto serialized = [&] {
        auto out = opentxs::paymentcode::Base58Preimage{};
        const auto bytes = api.Crypto().Encode().IdentifierDecode(base58);
        const auto* data = reinterpret_cast<const std::byte*>(bytes.data());

        switch (bytes.size()) {
            case 81: {
                static_assert(
                    81 == sizeof(opentxs::paymentcode::Base58Preimage));

                if (*data ==
                    opentxs::paymentcode::Base58Preimage::expected_prefix_) {
                    const auto version =
                        std::to_integer<std::uint8_t>(*std::next(data));

                    if ((0u < version) && (3u > version)) {
                        std::memcpy(
                            static_cast<void*>(&out), data, bytes.size());
                    }
                }
            } break;
            case 35: {
                static_assert(
                    35 == sizeof(opentxs::paymentcode::Base58Preimage_3));

                auto compact = opentxs::paymentcode::Base58Preimage_3{};

                if (*data ==
                    opentxs::paymentcode::Base58Preimage_3::expected_prefix_) {
                    const auto version =
                        std::to_integer<std::uint8_t>(*std::next(data));

                    if (2u < version) {
                        std::memcpy(
                            static_cast<void*>(&compact), data, bytes.size());
                        const auto& payload = compact.payload_;
                        const auto key = ReadView{
                            reinterpret_cast<const char*>(payload.key_.data()),
                            payload.key_.size()};
                        auto code = api.Factory().Data();
                        api.Crypto().Hash().Digest(
                            opentxs::crypto::HashType::Sha256D,
                            key,
                            code.WriteInto());
                        out = opentxs::paymentcode::Base58Preimage{
                            payload.version_, false, key, code.Bytes(), 0, 0};
                    }
                }
            } break;
            default: {
            }
        }

        return out;
    }();
    const auto& raw = serialized.payload_;

    return std::make_unique<opentxs::implementation::PaymentCode>(
               api,
               raw.version_,
               raw.haveBitmessage(),
               raw.xpub_.Key(),
               raw.xpub_.Chaincode(),
               raw.bm_version_,
               raw.bm_stream_,
               factory::Secp256k1Key(
                   api, raw.xpub_.Key(), raw.xpub_.Chaincode()))
        .release();
}

auto PaymentCode(
    const api::Session& api,
    const proto::PaymentCode& serialized) noexcept -> opentxs::PaymentCode
{
    return std::make_unique<opentxs::implementation::PaymentCode>(
               api,
               static_cast<std::uint8_t>(serialized.version()),
               serialized.bitmessage(),
               serialized.key(),
               serialized.chaincode(),
               static_cast<std::uint8_t>(serialized.bitmessageversion()),
               static_cast<std::uint8_t>(serialized.bitmessagestream()),
               factory::Secp256k1Key(
                   api, serialized.key(), serialized.chaincode()))
        .release();
}

auto PaymentCode(
    const api::Session& api,
    const UnallocatedCString& seed,
    const Bip32Index nym,
    const std::uint8_t version,
    const bool bitmessage,
    const std::uint8_t bitmessageVersion,
    const std::uint8_t bitmessageStream,
    const opentxs::PasswordPrompt& reason) noexcept -> opentxs::PaymentCode
{
    auto fingerprint{seed};
    auto pKey =
        api.Crypto().Seed().GetPaymentCode(fingerprint, nym, version, reason);

    if (false == bool(pKey)) {
        pKey = std::make_unique<opentxs::crypto::key::blank::Secp256k1>();
    }

    OT_ASSERT(pKey);

    const auto& key = *pKey;

    return std::make_unique<opentxs::implementation::PaymentCode>(
               api,
               version,
               bitmessage,
               key.PublicKey(),
               key.Chaincode(reason),
               bitmessageVersion,
               bitmessageStream,
               std::move(pKey))
        .release();
}
}  // namespace opentxs::factory

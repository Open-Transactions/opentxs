// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/core/Factory.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/PaymentCode.pb.h>
#include <array>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <memory>
#include <utility>

#include "core/paymentcode/Imp.hpp"
#include "core/paymentcode/Preimage.hpp"
#include "internal/crypto/asymmetric/Factory.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/crypto/HashType.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/key/Secp256k1.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto PaymentCode(
    const api::Session& api,
    const UnallocatedCString& base58) noexcept -> opentxs::PaymentCode
{
    const auto serialized = [&] {
        auto output = opentxs::paymentcode::Base58Preimage{};
        const auto bytes = [&] {
            auto out = ByteArray{};
            // TODO handle errors
            [[maybe_unused]] const auto rc =
                api.Crypto().Encode().Base58CheckDecode(
                    base58, out.WriteInto());

            return out;
        }();
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
                            static_cast<void*>(&output), data, bytes.size());
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
                        output = opentxs::paymentcode::Base58Preimage{
                            payload.version_, false, key, code.Bytes(), 0, 0};
                    }
                }
            } break;
            default: {
            }
        }

        return output;
    }();
    const auto& raw = serialized.payload_;
    auto key = factory::Secp256k1Key(
        api, raw.xpub_.Key(), raw.xpub_.Chaincode(), {}  // TODO allocator
    );

    return std::make_unique<opentxs::implementation::PaymentCode>(
               api,
               raw.version_,
               raw.haveBitmessage(),
               raw.xpub_.Key(),
               raw.xpub_.Chaincode(),
               raw.bm_version_,
               raw.bm_stream_,
               std::move(key))
        .release();
}

auto PaymentCode(
    const api::Session& api,
    const protobuf::PaymentCode& serialized) noexcept -> opentxs::PaymentCode
{
    auto key = factory::Secp256k1Key(
        api, serialized.key(), serialized.chaincode(), {}  // TODO allocator
    );

    return std::make_unique<opentxs::implementation::PaymentCode>(
               api,
               static_cast<std::uint8_t>(serialized.version()),
               serialized.bitmessage(),
               serialized.key(),
               serialized.chaincode(),
               static_cast<std::uint8_t>(serialized.bitmessageversion()),
               static_cast<std::uint8_t>(serialized.bitmessagestream()),
               std::move(key))
        .release();
}

auto PaymentCode(
    const api::Session& api,
    const crypto::SeedID& seed,
    const crypto::Bip32Index nym,
    const std::uint8_t version,
    const bool bitmessage,
    const std::uint8_t bitmessageVersion,
    const std::uint8_t bitmessageStream,
    const opentxs::PasswordPrompt& reason) noexcept -> opentxs::PaymentCode
{
    auto key = api.Crypto().Seed().GetPaymentCode(seed, nym, version, reason);
    const auto pub = key.PublicKey();
    const auto chain = key.Chaincode(reason);

    return std::make_unique<opentxs::implementation::PaymentCode>(
               api,
               version,
               bitmessage,
               pub,
               chain,
               bitmessageVersion,
               bitmessageStream,
               std::move(key))
        .release();
}
}  // namespace opentxs::factory

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::crypto::Encode

#include "api/crypto/Encode.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <iterator>
#include <limits>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "base58/base58.h"
#include "base64/base64.h"
#include "internal/api/crypto/Factory.hpp"
#include "internal/core/Core.hpp"
#include "internal/core/String.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/HashType.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto Encode(const api::Crypto& crypto) noexcept
    -> std::unique_ptr<api::crypto::Encode>
{
    using ReturnType = api::crypto::imp::Encode;

    return std::make_unique<ReturnType>(crypto);
}
}  // namespace opentxs::factory

namespace opentxs::api::crypto::imp
{
Encode::Encode(const api::Crypto& crypto) noexcept
    : crypto_(crypto)
    , factory_()
{
}

auto Encode::Base58CheckEncode(ReadView input, Writer&& output) const noexcept
    -> bool
{
    try {
        if (false == valid(input)) {

            return 0_uz == output.Reserve(0_uz).size();
        }

        static constexpr auto checksumBytes = 4_uz;
        const auto checksum = [&] {
            auto out = ByteArray{};
            const auto rc = crypto_.Hash().Digest(
                opentxs::crypto::HashType::Sha256DC, input, out.WriteInto());

            if (false == rc) {
                throw std::runtime_error{"failed to calculate checksum"};
            }

            return out;
        }();

        OT_ASSERT(checksumBytes == checksum.size());

        const auto size = input.size();
        const auto total = checksumBytes + size;
        const auto preimage = [&] {
            auto out = factory::Secret(total);

            OT_ASSERT(total == out.size());

            auto* i = static_cast<std::byte*>(out.data());
            auto rc = copy(input, preallocated(size, i));
            std::advance(i, size);

            OT_ASSERT(rc);

            rc = copy(checksum.Bytes(), preallocated(checksumBytes, i));

            OT_ASSERT(rc);

            return out;
        }();
        // TODO modify bitcoin_base58::EncodeBase58 to accept Writer to avoid
        // unnecessary copy
        const auto temp = bitcoin_base58::EncodeBase58(
            static_cast<const unsigned char*>(preimage.data()),
            static_cast<const unsigned char*>(preimage.data()) +
                preimage.size());

        return copy(temp, std::move(output));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Encode::Base58CheckDecode(std::string_view input, Writer&& output)
    const noexcept -> bool
{
    try {
        if (false == valid(input)) {

            return 0_uz == output.Reserve(0_uz).size();
        }

        const auto sanitized{SanatizeBase58(input)};
        auto vector = UnallocatedVector<unsigned char>{};
        const auto decoded =
            bitcoin_base58::DecodeBase58(sanitized.c_str(), vector);
        constexpr auto checkBytes = 4_uz;

        if (false == decoded) { throw std::runtime_error{"decode failure"}; }

        if (checkBytes > vector.size()) {
            throw std::runtime_error{"checksum missing"};
        }

        const auto bytes = reader(vector);
        const auto payload = bytes.substr(0_uz, vector.size() - checkBytes);
        const auto checksum = bytes.substr(payload.size());
        const auto calculated = [&] {
            auto out = ByteArray{};
            auto rc = crypto_.Hash().Digest(
                opentxs::crypto::HashType::Sha256DC, payload, out.WriteInto());

            if (false == rc) {
                throw std::runtime_error{"failed to calculate checksum"};
            }

            return out;
        }();

        if (calculated.Bytes() != checksum) {
            throw std::runtime_error{"checksum mismatch"};
        }

        return copy(payload, std::move(output));
    } catch (const std::exception& e) {
        LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Encode::Base64Encode(ReadView input, Writer&& output) const noexcept
    -> bool
{
    const auto temp = base64_encode(
        reinterpret_cast<const std::uint8_t*>(input.data()), input.size());

    return copy(temp, std::move(output));
}

auto Encode::Base64Decode(std::string_view input, Writer&& output)
    const noexcept -> bool
{
    RawData decoded;

    if (base64_decode(SanatizeBase64(input), decoded)) {

        return copy(reader(decoded), std::move(output));
    }

    return false;
}

auto Encode::base64_decode(const UnallocatedCString&& input, RawData& output)
    const -> bool
{
    output.resize(::Base64decode_len(input.data()), 0x0);

    const std::size_t decoded =
        ::Base64decode(reinterpret_cast<char*>(output.data()), input.data());

    if (0 == decoded) { return false; }

    OT_ASSERT(decoded <= output.size());

    output.resize(decoded);

    return true;
}

auto Encode::base64_encode(const std::uint8_t* inputStart, std::size_t size)
    const -> UnallocatedCString
{
    auto output = UnallocatedCString{};

    if (std::numeric_limits<int>::max() < size) { return {}; }

    const auto bytes = static_cast<int>(size);
    output.resize(::Base64encode_len(bytes));
    ::Base64encode(
        const_cast<char*>(output.data()),
        reinterpret_cast<const char*>(inputStart),
        bytes);

    return BreakLines(output);
}

auto Encode::BreakLines(const UnallocatedCString& input) const
    -> UnallocatedCString
{
    UnallocatedCString output;

    if (0 == input.size()) { return output; }

    std::size_t width = 0;

    for (const auto& character : input) {
        output.push_back(character);

        if (++width >= LineWidth) {
            output.push_back('\n');
            width = 0;
        }
    }

    if ('\n' != output.back()) { output.push_back('\n'); }

    return output;
}

auto Encode::Init(const std::shared_ptr<const api::Factory>& factory) noexcept
    -> void
{
    factory_ = factory;
}

auto Encode::IsBase64(std::string_view str) const noexcept -> bool
{
    return str.find_first_not_of("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHI"
                                 "JKLMNOPQRSTUVWXYZ") ==
           UnallocatedCString::npos;
}

auto Encode::Nonce(const std::uint32_t size) const -> OTString
{
    auto unusedOutput = ByteArray{};

    return Nonce(size, unusedOutput);
}

auto Encode::Nonce(const std::uint32_t size, Data& rawOutput) const -> OTString
{
    rawOutput.zeroMemory();
    rawOutput.SetSize(size);
    auto factory = factory_.lock();

    OT_ASSERT(factory);

    auto source = factory->Secret(0);
    source.Randomize(size);
    auto nonce = String::Factory();
    // TODO error handling
    [[maybe_unused]] const auto rc =
        Base58CheckEncode(source.Bytes(), nonce->WriteInto());
    rawOutput.Assign(source.Bytes());

    return nonce;
}

auto Encode::RandomFilename() const -> UnallocatedCString
{
    return Nonce(16)->Get();
}

auto Encode::SanatizeBase58(std::string_view input) const -> UnallocatedCString
{
    // TODO replace std::regex with better alternative

    return std::regex_replace(
        UnallocatedCString{input}, std::regex("[^1-9A-HJ-NP-Za-km-z]"), "");
}

auto Encode::SanatizeBase64(std::string_view input) const -> UnallocatedCString
{
    return std::regex_replace(
        UnallocatedCString{input}, std::regex("[^0-9A-Za-z+/=]"), "");
}

auto Encode::Z85Encode(ReadView input, Writer&& output) const noexcept -> bool
{
    return opentxs::network::zeromq::RawToZ85(input, std::move(output));
}

auto Encode::Z85Decode(std::string_view input, Writer&& output) const noexcept
    -> bool
{
    return opentxs::network::zeromq::Z85ToRaw(input, std::move(output));
}
}  // namespace opentxs::api::crypto::imp

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string_view>

#include "internal/api/crypto/Encode.hpp"
#include "internal/core/String.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Encode;
}  // namespace crypto

class Crypto;
}  // namespace api

class ByteArray;
class Data;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto::imp
{
class Encode final : public internal::Encode
{
public:
    auto DataEncode(const UnallocatedCString& input) const
        -> UnallocatedCString final;
    auto DataEncode(const Data& input) const -> UnallocatedCString final;
    auto DataDecode(const UnallocatedCString& input) const
        -> UnallocatedCString final;
    auto IdentifierEncode(const ReadView input) const
        -> UnallocatedCString final;
    auto IdentifierDecode(const ReadView input) const
        -> UnallocatedCString final;
    auto IsBase62(const UnallocatedCString& str) const -> bool final;
    auto Nonce(const std::uint32_t size) const -> OTString final;
    auto Nonce(const std::uint32_t size, Data& rawOutput) const
        -> OTString final;
    auto RandomFilename() const -> UnallocatedCString final;
    auto SanatizeBase58(std::string_view input) const
        -> UnallocatedCString final;
    auto SanatizeBase64(const UnallocatedCString& input) const
        -> UnallocatedCString final;
    auto Z85Encode(const Data& input) const -> UnallocatedCString final;
    auto Z85Encode(const UnallocatedCString& input) const
        -> UnallocatedCString final;
    auto Z85Decode(const Data& input) const -> ByteArray final;
    auto Z85Decode(const UnallocatedCString& input) const
        -> UnallocatedCString final;

    Encode(const api::Crypto& crypto) noexcept;
    Encode() = delete;
    Encode(const Encode&) = delete;
    Encode(Encode&&) = delete;
    auto operator=(const Encode&) -> Encode& = delete;
    auto operator=(Encode&&) -> Encode& = delete;

    ~Encode() final = default;

private:
    static const std::uint8_t LineWidth{72};

    const api::Crypto& crypto_;

    auto Base64Encode(
        const std::uint8_t* inputStart,
        const std::size_t& inputSize) const -> UnallocatedCString;
    auto Base64Decode(const UnallocatedCString&& input, RawData& output) const
        -> bool;
    auto BreakLines(const UnallocatedCString& input) const
        -> UnallocatedCString;
    auto IdentifierEncode(const Secret& input) const -> UnallocatedCString;
};
}  // namespace opentxs::api::crypto::imp

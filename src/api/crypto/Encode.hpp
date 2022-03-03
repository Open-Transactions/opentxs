// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>

#include "internal/api/crypto/Encode.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace crypto
{
class Encode;
}  // namespace crypto

class Crypto;
}  // namespace api

class OTPassword;
class Secret;
// }  // namespace v1
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
    auto IdentifierEncode(const Data& input) const -> UnallocatedCString final;
    auto IdentifierDecode(const UnallocatedCString& input) const
        -> UnallocatedCString final;
    auto IsBase62(const UnallocatedCString& str) const -> bool final;
    auto Nonce(const std::uint32_t size) const -> OTString final;
    auto Nonce(const std::uint32_t size, Data& rawOutput) const
        -> OTString final;
    auto RandomFilename() const -> UnallocatedCString final;
    auto SanatizeBase58(const UnallocatedCString& input) const
        -> UnallocatedCString final;
    auto SanatizeBase64(const UnallocatedCString& input) const
        -> UnallocatedCString final;
    auto Z85Encode(const Data& input) const -> UnallocatedCString final;
    auto Z85Encode(const UnallocatedCString& input) const
        -> UnallocatedCString final;
    auto Z85Decode(const Data& input) const -> OTData final;
    auto Z85Decode(const UnallocatedCString& input) const
        -> UnallocatedCString final;

    Encode(const api::Crypto& crypto) noexcept;

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
    auto IdentifierEncode(const void* data, const std::size_t size) const
        -> UnallocatedCString;

    Encode() = delete;
    Encode(const Encode&) = delete;
    auto operator=(const Encode&) -> Encode& = delete;
};
}  // namespace opentxs::api::crypto::imp

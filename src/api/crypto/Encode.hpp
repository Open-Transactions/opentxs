// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>

#include "internal/api/crypto/Encode.hpp"
#include "internal/core/String.hpp"
#include "internal/util/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Factory;
}  // namespace internal

class Crypto;
}  // namespace api

class Data;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto::imp
{
class Encode final : public internal::Encode
{
public:
    [[nodiscard]] auto Armor(
        ReadView input,
        Writer&& output,
        std::string_view bookend) const noexcept -> bool final;
    [[nodiscard]] auto Base58CheckEncode(ReadView input, Writer&& output)
        const noexcept -> bool final;
    [[nodiscard]] auto Base58CheckDecode(
        std::string_view input,
        Writer&& output) const noexcept -> bool final;
    [[nodiscard]] auto Base64Encode(ReadView input, Writer&& output)
        const noexcept -> bool final;
    [[nodiscard]] auto Base64Decode(std::string_view input, Writer&& output)
        const noexcept -> bool final;
    [[nodiscard]] auto Dearmor(ReadView input, Writer&& output) const noexcept
        -> bool final;
    auto IsBase64(std::string_view str) const noexcept -> bool final;
    auto Nonce(const std::uint32_t size) const -> OTString final;
    auto Nonce(const std::uint32_t size, Data& rawOutput) const
        -> OTString final;
    auto RandomFilename() const -> UnallocatedCString final;
    auto SanatizeBase58(std::string_view input) const
        -> UnallocatedCString final;
    auto SanatizeBase64(std::string_view input) const
        -> UnallocatedCString final;
    [[nodiscard]] auto Z85Encode(ReadView input, Writer&& output) const noexcept
        -> bool final;
    [[nodiscard]] auto Z85Decode(std::string_view input, Writer&& output)
        const noexcept -> bool final;

    auto Init(
        const std::shared_ptr<const api::internal::Factory>& factory) noexcept
        -> void final;

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
    std::weak_ptr<const api::internal::Factory> factory_;

    auto base64_encode(const std::uint8_t* inputStart, std::size_t inputSize)
        const -> UnallocatedCString;
    auto base64_decode(const UnallocatedCString&& input, RawData& output) const
        -> bool;
    auto BreakLines(const UnallocatedCString& input) const
        -> UnallocatedCString;
};
}  // namespace opentxs::api::crypto::imp

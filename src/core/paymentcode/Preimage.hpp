// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <array>
#include <cstdint>
#include <cstring>

#include "opentxs/util/Bytes.hpp"

namespace opentxs::paymentcode
{
struct XpubPreimage {
    std::array<std::byte, 33> key_;
    std::array<std::byte, 32> code_;

    operator ReadView() const noexcept
    {
        return {reinterpret_cast<const char*>(this), sizeof(*this)};
    }

    auto Chaincode() const noexcept -> ReadView
    {
        return {reinterpret_cast<const char*>(code_.data()), code_.size()};
    }
    auto Key() const noexcept -> ReadView
    {
        return {reinterpret_cast<const char*>(key_.data()), key_.size()};
    }

    XpubPreimage(const ReadView key, const ReadView code) noexcept
        : key_()
        , code_()
    {
        static_assert(65 == sizeof(XpubPreimage));

        if (nullptr != key.data()) {
            std::memcpy(
                key_.data(), key.data(), std::min(key_.size(), key.size()));
        }

        if (nullptr != code.data()) {
            std::memcpy(
                code_.data(), code.data(), std::min(code_.size(), code.size()));
        }
    }
    XpubPreimage() noexcept
        : XpubPreimage({}, {})
    {
    }
};

struct BinaryPreimage {
    std::uint8_t version_;
    std::uint8_t features_;
    XpubPreimage xpub_;
    std::uint8_t bm_version_;
    std::uint8_t bm_stream_;
    std::array<std::byte, 11> blank_;

    operator ReadView() const noexcept
    {
        return {reinterpret_cast<const char*>(this), sizeof(*this)};
    }

    auto haveBitmessage() const noexcept -> bool
    {
        return 0 != (features_ & std::uint8_t{0x80});
    }

    BinaryPreimage(
        const VersionNumber version,
        const bool bitmessage,
        const ReadView key,
        const ReadView code,
        const std::uint8_t bmVersion,
        const std::uint8_t bmStream) noexcept
        : version_(static_cast<std::uint8_t>(version))
        , features_(bitmessage ? 0x80 : 0x00)
        , xpub_(key, code)
        , bm_version_(bmVersion)
        , bm_stream_(bmStream)
        , blank_()
    {
        static_assert(80 == sizeof(BinaryPreimage));
    }
    BinaryPreimage() noexcept
        : BinaryPreimage(0, false, {}, {}, 0, 0)
    {
    }
};

struct BinaryPreimage_3 {
    std::uint8_t version_;
    std::array<std::byte, 33> key_;

    operator ReadView() const noexcept
    {
        return {reinterpret_cast<const char*>(this), sizeof(*this)};
    }

    auto Key() const noexcept -> ReadView
    {
        return {reinterpret_cast<const char*>(key_.data()), key_.size()};
    }

    BinaryPreimage_3(const VersionNumber version, const ReadView key) noexcept
        : version_(version)
        , key_()
    {
        static_assert(34 == sizeof(BinaryPreimage_3));

        if (nullptr != key.data()) {
            std::memcpy(
                key_.data(), key.data(), std::min(key_.size(), key.size()));
        }
    }
    BinaryPreimage_3() noexcept
        : BinaryPreimage_3(0, {})
    {
    }
};

struct Base58Preimage {
    static constexpr auto expected_prefix_ = std::byte{0x47};

    std::uint8_t prefix_;
    BinaryPreimage payload_;

    operator ReadView() const noexcept
    {
        return {reinterpret_cast<const char*>(this), sizeof(*this)};
    }

    Base58Preimage(BinaryPreimage&& data) noexcept
        : prefix_(std::to_integer<std::uint8_t>(expected_prefix_))
        , payload_(std::move(data))
    {
        static_assert(81 == sizeof(Base58Preimage));
    }
    Base58Preimage(
        const VersionNumber version,
        const bool bitmessage,
        const ReadView key,
        const ReadView code,
        const std::uint8_t bmVersion,
        const std::uint8_t bmStream) noexcept
        : Base58Preimage(BinaryPreimage{
              version,
              bitmessage,
              key,
              code,
              bmVersion,
              bmStream})
    {
    }
    Base58Preimage() noexcept
        : Base58Preimage(BinaryPreimage{})
    {
    }
};

struct Base58Preimage_3 {
    static constexpr auto expected_prefix_ = std::byte{0x22};

    std::uint8_t prefix_;
    BinaryPreimage_3 payload_;

    operator ReadView() const noexcept
    {
        return {reinterpret_cast<const char*>(this), sizeof(*this)};
    }

    Base58Preimage_3(BinaryPreimage_3&& data) noexcept
        : prefix_(std::to_integer<std::uint8_t>(expected_prefix_))
        , payload_(std::move(data))
    {
        static_assert(35 == sizeof(Base58Preimage_3));
    }
    Base58Preimage_3(const VersionNumber version, const ReadView key) noexcept
        : Base58Preimage_3(BinaryPreimage_3{version, key})
    {
    }
    Base58Preimage_3() noexcept
        : Base58Preimage_3(BinaryPreimage_3{})
    {
    }
};
}  // namespace opentxs::paymentcode

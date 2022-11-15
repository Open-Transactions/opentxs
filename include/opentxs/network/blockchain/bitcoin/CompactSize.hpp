// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>

#include "opentxs/Export.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class ByteArray;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin
{
class OPENTXS_EXPORT CompactSize
{
public:
    // Returns the number of bytes SUBSEQUENT to the marker byte
    // Possible output values are: 0, 2, 4, 8
    static auto CalculateSize(const std::byte first) noexcept -> std::uint64_t;

    auto Encode() const noexcept -> ByteArray;
    auto Encode(Writer&& destination) const noexcept -> bool;
    // Number of bytes the CompactSize will occupy
    auto Size() const noexcept -> std::size_t;
    // Number of bytes the CompactSize and associated data will occupy
    auto Total() const noexcept -> std::size_t;
    // Number of bytes encoded by the CompactSize
    auto Value() const noexcept -> std::uint64_t;

    // Marker byte should be omitted
    // Valid inputs are 0, 2, 4, or 8 bytes
    auto Decode(ReadView bytes) noexcept -> bool;

    CompactSize() noexcept;
    explicit CompactSize(std::uint64_t value) noexcept;
    // Marker byte should be omitted
    // Valid inputs are 1, 2, 4, or 8 bytes
    // Throws std::invalid_argument for invalid input
    OPENTXS_NO_EXPORT CompactSize(const ReadView bytes) noexcept(false);
    CompactSize(const CompactSize&) noexcept;
    CompactSize(CompactSize&&) noexcept;
    auto operator=(const CompactSize&) noexcept -> CompactSize&;
    auto operator=(CompactSize&&) noexcept -> CompactSize&;
    auto operator=(const std::uint64_t rhs) noexcept -> CompactSize&;

    ~CompactSize();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};

/// Decodes a serialized CompactSize
///
/// If decoding is successful the input view will be modified to exclude the
/// bytes which have been read
OPENTXS_EXPORT auto DecodeCompactSize(ReadView& input) noexcept
    -> std::optional<std::size_t>;
/// Decodes a serialized CompactSize
///
/// If decoding is successful the input view will be modified to exclude the
/// bytes which have been read and parsed will be set to the excluded bytes.
///
/// If a non-null pointer is passed as out then the associated object will be
/// assigned an appropriate value
OPENTXS_EXPORT auto DecodeCompactSize(
    ReadView& input,
    ReadView& parsed,
    CompactSize* out) noexcept -> std::optional<std::size_t>;
}  // namespace opentxs::network::blockchain::bitcoin

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
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin
{
class OPENTXS_EXPORT CompactSize
{
public:
    using Bytes = UnallocatedVector<std::byte>;

    // Returns the number of bytes SUBSEQUENT to the marker byte
    // Possible output values are: 0, 2, 4, 8
    static auto CalculateSize(const std::byte first) noexcept -> std::uint64_t;

    auto Encode() const noexcept -> Bytes;
    auto Encode(Writer&& destination) const noexcept -> bool;
    // Number of bytes the CompactSize will occupy
    auto Size() const noexcept -> std::size_t;
    // Number of bytes the CompactSize and associated data will occupy
    auto Total() const noexcept -> std::size_t;
    // Number of bytes encoded by the CompactSize
    auto Value() const noexcept -> std::uint64_t;

    // Marker byte should be omitted
    // Valid inputs are 0, 2, 4, or 8 bytes
    auto Decode(const Bytes& bytes) noexcept -> bool;
    auto Decode(ReadView bytes) noexcept -> bool;

    CompactSize() noexcept;
    explicit CompactSize(std::uint64_t value) noexcept;
    // Marker byte should be omitted
    // Valid inputs are 1, 2, 4, or 8 bytes
    // Throws std::invalid_argument for invalid input
    CompactSize(const Bytes& bytes) noexcept(false);
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

/// Decode operations
///
/// The expected use is to start with a ByteIterator (input) to a range of
/// encoded data with a known size (total). expectedSize is incremented with the
/// size of the next element expected to be present and is continually checked
/// against total prior to attempting to read.
///
/// Prior to calling any of the following Decode functions input should be
/// positioned to a byte expected to contain the beginning of a CompactSize and
/// expectedSize should be set to the total number of bytes in the range which
/// have already been processed, plus one byte.
///
/// If a Decode function returns true then input will be advanced to the byte
/// following the CompactSize and expectedSize will be incremented by the same
/// number of bytes that input was advanced, and the new expectedSize has been
/// successfully verified to not exceed total.
///
/// If a Decode function returns false it means the byte referenced by input
/// designates a size the CompactSize should occupy which exceeds the number of
/// bytes available in the input range.

using Byte = const std::byte;
using ByteIterator = Byte*;

/// Decodes a CompactSize and returns the output as a std::size_t
auto DecodeSize(
    ByteIterator& input,
    std::size_t& expectedSize,
    const std::size_t total,
    std::size_t& output) noexcept -> bool;
/// Decodes a CompactSize and returns the output as a CompactSize
auto DecodeSize(
    ByteIterator& input,
    std::size_t& expectedSize,
    const std::size_t total,
    CompactSize& output) noexcept -> bool;
/// Decodes a compact size and returns the output as a std::size_t, also reports
/// the number of additional bytes used to encode the CompactSize.
auto DecodeSize(
    ByteIterator& input,
    std::size_t& expectedSize,
    const std::size_t total,
    std::size_t& output,
    std::size_t& csExtraBytes) noexcept -> bool;

/// Decodes a serialized CompactSize
///
/// If decoding is successful the input view will be modified to exclude the
/// bytes which have been read
OPENTXS_EXPORT auto DecodeCompactSize(ReadView& input) noexcept
    -> std::optional<std::size_t>;
/// Decodes a serialized CompactSize
///
/// If decoding is successful the input view will be modified to exclude the
/// bytes which have been read and parsed will be set to the excluded bytes
OPENTXS_EXPORT auto DecodeCompactSize(
    ReadView& input,
    ReadView& parsed) noexcept -> std::optional<std::size_t>;
}  // namespace opentxs::network::blockchain::bitcoin

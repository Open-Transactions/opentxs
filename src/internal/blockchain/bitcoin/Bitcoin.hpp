// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#pragma once

#include <boost/endian/buffers.hpp>
#include <boost/endian/conversion.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <tuple>

#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
namespace internal
{
class Input;
}  // namespace internal
}  // namespace block
}  // namespace bitcoin
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace be = boost::endian;

namespace opentxs::blockchain
{
static constexpr auto standard_hash_size_ = 32_uz;
}  // namespace opentxs::blockchain

namespace opentxs::blockchain::bitcoin
{
using Byte = const std::byte;
using ByteIterator = Byte*;
using CompactSize = network::blockchain::bitcoin::CompactSize;

/// input: gets incremented to the byte past the segwit flag byte if transaction
/// is segwit
///
/// expectedSize: gets incremented by 2 if transaction is segwit
auto HasSegwit(
    ByteIterator& input,
    std::size_t& expectedSize,
    const std::size_t size) noexcept(false) -> std::optional<std::byte>;
/// If the segwit marker is detected the input view will be modified to exclude
/// the first two bytes
auto HasSegwit(ReadView& input) noexcept -> std::optional<std::byte>;

struct EncodedOutpoint {
    std::array<std::byte, standard_hash_size_> txid_{};
    be::little_uint32_buf_t index_{};
};

struct EncodedInput {
    EncodedOutpoint outpoint_{};
    CompactSize cs_{};
    Space script_{};
    be::little_uint32_buf_t sequence_{};

    auto size() const noexcept -> std::size_t;
};

struct EncodedOutput {
    be::little_uint64_buf_t value_{};
    CompactSize cs_{};
    Space script_{};

    auto size() const noexcept -> std::size_t;
};

struct EncodedWitnessItem {
    CompactSize cs_{};
    Space item_{};

    auto size() const noexcept -> std::size_t;
};

struct EncodedInputWitness {
    CompactSize cs_{};
    UnallocatedVector<EncodedWitnessItem> items_{};

    auto size() const noexcept -> std::size_t;
};

struct EncodedTransaction {
    be::little_int32_buf_t version_{};
    std::optional<std::byte> segwit_flag_{};
    CompactSize input_count_{};
    UnallocatedVector<EncodedInput> inputs_{};
    CompactSize output_count_{};
    UnallocatedVector<EncodedOutput> outputs_{};
    UnallocatedVector<EncodedInputWitness> witnesses_{};
    be::little_uint32_buf_t lock_time_{};
    Space wtxid_{};
    Space txid_{};

    static auto DefaultVersion(const blockchain::Type chain) noexcept
        -> std::uint32_t;

    auto CalculateIDs(
        const api::Crypto& crypto,
        const blockchain::Type chain) noexcept -> bool;
    auto CalculateIDs(
        const api::Crypto& crypto,
        const blockchain::Type chain,
        ReadView bytes) noexcept -> bool;
    static auto Deserialize(
        const api::Crypto& crypto,
        const blockchain::Type chain,
        const ReadView bytes) noexcept(false) -> EncodedTransaction;

    auto wtxid_preimage() const noexcept -> Space;
    auto txid_preimage() const noexcept -> Space;
    auto txid_size() const noexcept -> std::size_t;
    auto size() const noexcept -> std::size_t;
};

enum class SigOption : std::uint8_t {
    All,
    None,
    Single,
};

struct SigHash {
    std::byte flags_{0x01};
    std::array<std::byte, 3> forkid_{};

    auto AnyoneCanPay() const noexcept -> bool;
    auto begin() const noexcept -> const std::byte*;
    auto end() const noexcept -> const std::byte*;
    auto ForkID() const noexcept -> ReadView;
    auto Type() const noexcept -> SigOption;

    SigHash(
        const blockchain::Type chain,
        const SigOption flag = SigOption::All,
        const bool anyoneCanPay = false) noexcept;
};

struct Bip143Hashes {
    using Hash = std::array<std::byte, standard_hash_size_>;

    Hash outpoints_{};
    Hash sequences_{};
    Hash outputs_{};

    auto Outpoints(const SigHash type) const noexcept -> const Hash&;
    auto Outputs(const SigHash type, const Hash* single) const noexcept
        -> const Hash&;
    auto Preimage(
        const std::size_t index,
        const std::size_t total,
        const be::little_int32_buf_t& version,
        const be::little_uint32_buf_t& locktime,
        const SigHash& sigHash,
        const blockchain::bitcoin::block::internal::Input& input) const
        noexcept(false) -> Space;
    auto Sequences(const SigHash type) const noexcept -> const Hash&;

private:
    static auto blank() noexcept -> const Hash&;
    static auto get_single(
        const std::size_t index,
        const std::size_t total,
        const SigHash& sigHash) noexcept -> std::unique_ptr<Hash>;
};
}  // namespace opentxs::blockchain::bitcoin

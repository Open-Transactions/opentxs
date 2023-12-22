// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/endian/buffers.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>

#include "internal/blockchain/protocol/bitcoin/bitcoincash/token/Types.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/alloc/Allocated.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/core/ByteArray.hpp"
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
namespace block
{
class Hash;
}  // namespace block

namespace protocol
{
namespace bitcoin
{
namespace base
{
namespace block
{
class Input;
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace be = boost::endian;

namespace opentxs::blockchain
{
static constexpr auto standard_hash_size_ = 32_uz;
}  // namespace opentxs::blockchain

namespace opentxs::blockchain::protocol::bitcoin::base
{
using blockchain::block::Hash;
using blockchain::block::TransactionHash;
using network::blockchain::bitcoin::CompactSize;

struct EncodedOutpoint {
    std::array<std::byte, standard_hash_size_> txid_;
    be::little_uint32_buf_t index_;

    EncodedOutpoint() noexcept
        : txid_()
        , index_()
    {
    }
    EncodedOutpoint(const EncodedOutpoint&) noexcept = default;
    EncodedOutpoint(EncodedOutpoint&& rhs) noexcept
        : txid_(std::move(rhs.txid_))
        , index_(std::move(rhs.index_))
    {
    }
    auto operator=(const EncodedOutpoint&) noexcept
        -> EncodedOutpoint& = default;
    auto operator=(EncodedOutpoint&&) noexcept -> EncodedOutpoint& = default;
};

struct EncodedInput final : opentxs::pmr::Allocated {
    EncodedOutpoint outpoint_;
    CompactSize cs_;
    ByteArray script_;
    be::little_uint32_buf_t sequence_;

    auto size() const noexcept -> std::size_t;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    EncodedInput(allocator_type alloc = {}) noexcept
        : Allocated(alloc)
        , outpoint_()
        , cs_()
        , script_(alloc)
        , sequence_()
    {
    }
    EncodedInput(const EncodedInput& rhs, allocator_type alloc = {}) noexcept
        : Allocated(alloc)
        , outpoint_(rhs.outpoint_)
        , cs_(rhs.cs_)
        , script_(rhs.script_, alloc)
        , sequence_(rhs.sequence_)
    {
    }
    EncodedInput(EncodedInput&& rhs) noexcept
        : EncodedInput(std::move(rhs), rhs.get_allocator())
    {
    }
    EncodedInput(EncodedInput&& rhs, allocator_type alloc) noexcept
        : Allocated(alloc)
        , outpoint_(std::move(rhs.outpoint_))
        , cs_(std::move(rhs.cs_))
        , script_(std::move(rhs.script_), alloc)
        , sequence_(std::move(rhs.sequence_))
    {
    }
    auto operator=(const EncodedInput& rhs) noexcept -> EncodedInput&
    {
        if (this != std::addressof(rhs)) {
            outpoint_ = rhs.outpoint_;
            cs_ = rhs.cs_;
            script_ = rhs.script_;
            sequence_ = rhs.sequence_;
        }

        return *this;
    }
    auto operator=(EncodedInput&& rhs) noexcept -> EncodedInput&
    {
        if (get_allocator() == rhs.get_allocator()) {
            using std::swap;
            swap(outpoint_, rhs.outpoint_);
            swap(cs_, rhs.cs_);
            swap(script_, rhs.script_);
            swap(sequence_, rhs.sequence_);

            return *this;
        } else {

            // NOLINTNEXTLINE(misc-unconventional-assign-operator)
            return operator=(rhs);
        }
    }

    ~EncodedInput() final = default;
};

struct EncodedOutput final : opentxs::pmr::Allocated {
    be::little_uint64_buf_t value_;
    CompactSize cs_;
    std::optional<protocol::bitcoin::bitcoincash::token::cashtoken::Value>
        cashtoken_;
    ByteArray script_;

    auto size() const noexcept -> std::size_t;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    EncodedOutput(allocator_type alloc = {}) noexcept
        : Allocated(alloc)
        , value_()
        , cs_()
        , cashtoken_()
        , script_(alloc)
    {
    }
    EncodedOutput(const EncodedOutput& rhs, allocator_type alloc = {}) noexcept
        : Allocated(alloc)
        , value_(rhs.value_)
        , cs_(rhs.cs_)
        , cashtoken_(rhs.cashtoken_)
        , script_(rhs.script_, alloc)
    {
    }
    EncodedOutput(EncodedOutput&& rhs) noexcept
        : EncodedOutput(std::move(rhs), rhs.get_allocator())
    {
    }
    EncodedOutput(EncodedOutput&& rhs, allocator_type alloc) noexcept
        : Allocated(alloc)
        , value_(std::move(rhs.value_))
        , cs_(std::move(rhs.cs_))
        , cashtoken_(std::move(rhs.cashtoken_))
        , script_(std::move(rhs.script_), alloc)
    {
    }
    auto operator=(const EncodedOutput& rhs) noexcept -> EncodedOutput&
    {
        if (this != std::addressof(rhs)) {
            value_ = rhs.value_;
            cs_ = rhs.cs_;
            cashtoken_ = rhs.cashtoken_;
            script_ = rhs.script_;
        }

        return *this;
    }
    auto operator=(EncodedOutput&& rhs) noexcept -> EncodedOutput&
    {
        if (get_allocator() == rhs.get_allocator()) {
            using std::swap;
            swap(value_, rhs.value_);
            swap(cs_, rhs.cs_);
            swap(cashtoken_, rhs.cashtoken_);
            swap(script_, rhs.script_);

            return *this;
        } else {

            // NOLINTNEXTLINE(misc-unconventional-assign-operator)
            return operator=(rhs);
        }
    }

    ~EncodedOutput() final = default;
};

struct EncodedWitnessItem final : opentxs::pmr::Allocated {
    CompactSize cs_;
    ByteArray item_;

    auto size() const noexcept -> std::size_t;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    EncodedWitnessItem(allocator_type alloc = {}) noexcept
        : Allocated(alloc)
        , cs_()
        , item_(alloc)
    {
    }
    EncodedWitnessItem(
        const EncodedWitnessItem& rhs,
        allocator_type alloc = {}) noexcept
        : Allocated(alloc)
        , cs_(rhs.cs_)
        , item_(rhs.item_, alloc)
    {
    }
    EncodedWitnessItem(EncodedWitnessItem&& rhs) noexcept
        : EncodedWitnessItem(std::move(rhs), rhs.get_allocator())
    {
    }
    EncodedWitnessItem(EncodedWitnessItem&& rhs, allocator_type alloc) noexcept
        : Allocated(alloc)
        , cs_(std::move(rhs.cs_))
        , item_(std::move(rhs.item_), alloc)
    {
    }
    auto operator=(const EncodedWitnessItem& rhs) noexcept
        -> EncodedWitnessItem&
    {
        if (this != std::addressof(rhs)) {
            cs_ = rhs.cs_;
            item_ = rhs.item_;
        }

        return *this;
    }
    auto operator=(EncodedWitnessItem&& rhs) noexcept -> EncodedWitnessItem&
    {
        if (get_allocator() == rhs.get_allocator()) {
            using std::swap;
            swap(cs_, rhs.cs_);
            swap(item_, rhs.item_);

            return *this;
        } else {

            // NOLINTNEXTLINE(misc-unconventional-assign-operator)
            return operator=(rhs);
        }
    }

    ~EncodedWitnessItem() final = default;
};

struct EncodedInputWitness final : opentxs::pmr::Allocated {
    CompactSize cs_;
    Vector<EncodedWitnessItem> items_;

    auto size() const noexcept -> std::size_t;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    EncodedInputWitness(allocator_type alloc = {}) noexcept
        : Allocated(alloc)
        , cs_()
        , items_(alloc)
    {
    }
    EncodedInputWitness(
        const EncodedInputWitness& rhs,
        allocator_type alloc = {}) noexcept
        : Allocated(alloc)
        , cs_(rhs.cs_)
        , items_(rhs.items_, alloc)
    {
    }
    EncodedInputWitness(EncodedInputWitness&& rhs) noexcept
        : EncodedInputWitness(std::move(rhs), rhs.get_allocator())
    {
    }
    EncodedInputWitness(
        EncodedInputWitness&& rhs,
        allocator_type alloc) noexcept
        : Allocated(alloc)
        , cs_(std::move(rhs.cs_))
        , items_(std::move(rhs.items_), alloc)
    {
    }
    auto operator=(const EncodedInputWitness& rhs) noexcept
        -> EncodedInputWitness&
    {
        if (this != std::addressof(rhs)) {
            cs_ = rhs.cs_;
            items_ = rhs.items_;
        }

        return *this;
    }
    auto operator=(EncodedInputWitness&& rhs) noexcept -> EncodedInputWitness&
    {
        if (get_allocator() == rhs.get_allocator()) {
            using std::swap;
            swap(cs_, rhs.cs_);
            swap(items_, rhs.items_);

            return *this;
        } else {

            // NOLINTNEXTLINE(misc-unconventional-assign-operator)
            return operator=(rhs);
        }
    }

    ~EncodedInputWitness() final = default;
};

struct EncodedTransaction final : opentxs::pmr::Allocated {
    be::little_int32_buf_t version_;
    std::optional<std::byte> segwit_flag_;
    CompactSize input_count_;
    Vector<EncodedInput> inputs_;
    CompactSize output_count_;
    Vector<EncodedOutput> outputs_;
    Vector<EncodedInputWitness> witnesses_;
    be::little_uint32_buf_t lock_time_;
    CompactSize dip_2_bytes_;
    std::optional<ByteArray> dip_2_;
    TransactionHash wtxid_;
    TransactionHash txid_;

    static auto DefaultVersion(const blockchain::Type chain) noexcept
        -> std::uint32_t;

    auto CalculateIDs(
        const api::Crypto& crypto,
        const blockchain::Type chain,
        const bool isGeneration) noexcept -> bool;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    EncodedTransaction(allocator_type alloc = {}) noexcept
        : Allocated(alloc)
        , version_()
        , segwit_flag_()
        , input_count_()
        , inputs_(alloc)
        , output_count_()
        , outputs_(alloc)
        , witnesses_(alloc)
        , lock_time_()
        , dip_2_bytes_()
        , dip_2_()
        , wtxid_()
        , txid_()
    {
    }
    EncodedTransaction(
        const EncodedTransaction& rhs,
        allocator_type alloc = {}) noexcept
        : Allocated(alloc)
        , version_(rhs.version_)
        , segwit_flag_(rhs.segwit_flag_)
        , input_count_(rhs.input_count_)
        , inputs_(rhs.inputs_, alloc)
        , output_count_(rhs.output_count_)
        , outputs_(rhs.outputs_, alloc)
        , witnesses_(rhs.witnesses_, alloc)
        , lock_time_(rhs.lock_time_)
        , dip_2_bytes_(rhs.dip_2_bytes_)
        , dip_2_([&]() -> decltype(dip_2_) {
            if (rhs.dip_2_.has_value()) {

                return ByteArray{*rhs.dip_2_, alloc};
            } else {

                return std::nullopt;
            }
        }())
        , wtxid_(rhs.wtxid_)
        , txid_(rhs.txid_)
    {
    }
    EncodedTransaction(EncodedTransaction&& rhs) noexcept
        : EncodedTransaction(std::move(rhs), rhs.get_allocator())
    {
    }
    EncodedTransaction(EncodedTransaction&& rhs, allocator_type alloc) noexcept
        : Allocated(alloc)
        , version_(std::move(rhs.version_))
        , segwit_flag_(std::move(rhs.segwit_flag_))
        , input_count_(std::move(rhs.input_count_))
        , inputs_(std::move(rhs.inputs_), alloc)
        , output_count_(std::move(rhs.output_count_))
        , outputs_(std::move(rhs.outputs_), alloc)
        , witnesses_(std::move(rhs.witnesses_), alloc)
        , lock_time_(std::move(rhs.lock_time_))
        , dip_2_bytes_(std::move(rhs.dip_2_bytes_))
        , dip_2_([&]() -> decltype(dip_2_) {
            if (rhs.dip_2_.has_value()) {

                return ByteArray{std::move(*rhs.dip_2_), alloc};
            } else {

                return std::nullopt;
            }
        }())
        , wtxid_(std::move(rhs.wtxid_))
        , txid_(std::move(rhs.txid_))
    {
    }
    auto operator=(const EncodedTransaction& rhs) noexcept
        -> EncodedTransaction&
    {
        if (this != std::addressof(rhs)) {
            version_ = rhs.version_;
            segwit_flag_ = rhs.segwit_flag_;
            input_count_ = rhs.input_count_;
            inputs_ = rhs.inputs_;
            output_count_ = rhs.output_count_;
            outputs_ = rhs.outputs_;
            witnesses_ = rhs.witnesses_;
            lock_time_ = rhs.lock_time_;
            dip_2_bytes_ = rhs.dip_2_bytes_;
            dip_2_ = [&]() -> decltype(dip_2_) {
                if (rhs.dip_2_.has_value()) {

                    return ByteArray{*rhs.dip_2_, get_allocator()};
                } else {

                    return std::nullopt;
                }
            }();
            wtxid_ = rhs.wtxid_;
            txid_ = rhs.txid_;
        }

        return *this;
    }
    auto operator=(EncodedTransaction&& rhs) noexcept -> EncodedTransaction&
    {
        if (get_allocator() == rhs.get_allocator()) {
            using std::swap;
            swap(version_, rhs.version_);
            swap(segwit_flag_, rhs.segwit_flag_);
            swap(input_count_, rhs.input_count_);
            swap(inputs_, rhs.inputs_);
            swap(output_count_, rhs.output_count_);
            swap(outputs_, rhs.outputs_);
            swap(witnesses_, rhs.witnesses_);
            swap(lock_time_, rhs.lock_time_);
            swap(dip_2_bytes_, rhs.dip_2_bytes_);
            swap(dip_2_, rhs.dip_2_);
            swap(wtxid_, rhs.wtxid_);
            swap(txid_, rhs.txid_);

            return *this;
        } else {

            // NOLINTNEXTLINE(misc-unconventional-assign-operator)
            return operator=(rhs);
        }
    }

    ~EncodedTransaction() final = default;

private:
    struct Preimages {
        ByteArray legacy_{};
        ByteArray segwit_{};
    };

    auto dip_2_size() const noexcept -> std::size_t;
    auto preimages() const noexcept(false) -> Preimages;
    auto legacy_size() const noexcept -> std::size_t;
    auto segwit_size() const noexcept -> std::size_t;
};

enum class SigOption : std::uint8_t {
    All,
    None,
    Single,
};  // IWYU pragma: export

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
        const blockchain::protocol::bitcoin::base::block::Input& input) const
        noexcept(false) -> ByteArray;
    auto Sequences(const SigHash type) const noexcept -> const Hash&;

private:
    static auto blank() noexcept -> const Hash&;
    static auto get_single(
        const std::size_t index,
        const std::size_t total,
        const SigHash& sigHash) noexcept -> std::unique_ptr<Hash>;
};
}  // namespace opentxs::blockchain::protocol::bitcoin::base

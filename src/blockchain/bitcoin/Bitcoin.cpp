// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "internal/blockchain/bitcoin/Bitcoin.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <numeric>
#include <stdexcept>

#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/core/Amount.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Log.hpp"

namespace bb = opentxs::blockchain::bitcoin;

namespace opentxs::blockchain::bitcoin
{
const auto cb = [](const auto lhs, const auto& in) -> std::size_t {
    return lhs + in.size();
};

auto HasSegwit(
    ByteIterator& input,
    std::size_t& expectedSize,
    const std::size_t size) noexcept(false) -> std::optional<std::byte>
{
    auto output = std::optional<std::byte>{};

    OT_ASSERT(false == output.has_value());

    if (size < (expectedSize + 2)) {
        throw std::runtime_error("Unable to check for segwit marker bytes");
    }

    if (nullptr == input) {
        throw std::runtime_error("Invalid input iterator");
    }

    static const auto blank = std::byte{0x00};

    if (blank != *input) {
        LogInsane()(__func__)(": No marker byte").Flush();

        return output;
    }

    auto flag = *(input + 1);

    if (blank == flag) {
        LogInsane()(__func__)(": No flag byte").Flush();

        return output;
    }

    output = flag;
    std::advance(input, 2);
    expectedSize += 2;

    OT_ASSERT(output.has_value());

    return output;
}

auto Bip143Hashes::blank() noexcept -> const Hash&
{
    static const auto output = Hash{};

    return output;
}

auto Bip143Hashes::get_single(
    const std::size_t index,
    const std::size_t total,
    const SigHash& sigHash) noexcept -> std::unique_ptr<Hash>
{
    if (bitcoin::SigOption::Single != sigHash.Type()) { return nullptr; }

    if (index >= total) { return nullptr; }

    // TODO not implemented yet

    return nullptr;
}

auto Bip143Hashes::Outpoints(const SigHash sig) const noexcept -> const Hash&
{
    if (sig.AnyoneCanPay()) { return blank(); }

    return outpoints_;
}

auto Bip143Hashes::Outputs(const SigHash sig, const Hash* single) const noexcept
    -> const Hash&
{
    if (SigOption::All == sig.Type()) {

        return outputs_;
    } else if ((SigOption::Single == sig.Type()) && (nullptr != single)) {

        return *single;
    } else {

        return blank();
    }
}

auto Bip143Hashes::Preimage(
    const std::size_t index,
    const std::size_t total,
    const be::little_int32_buf_t& version,
    const be::little_uint32_buf_t& locktime,
    const SigHash& sigHash,
    const block::bitcoin::internal::Input& input) const noexcept(false) -> Space
{
    const auto& outpoints = Outpoints(sigHash);
    const auto& sequences = Sequences(sigHash);
    const auto& outpoint = input.PreviousOutput();
    const auto pScript = input.Spends().SigningSubscript();

    OT_ASSERT(pScript);

    const auto& script = *pScript;
    const auto scriptBytes = script.CalculateSize();
    const auto cs = blockchain::bitcoin::CompactSize{scriptBytes};
    const auto& output = input.Spends();
    const auto value =
        be::little_int64_buf_t{output.Value().Internal().ExtractInt64()};
    const auto sequence = be::little_uint32_buf_t{input.Sequence()};
    const auto single = get_single(index, total, sigHash);
    const auto& outputs = Outputs(sigHash, single.get());
    // clang-format off
    auto preimage = space(
        sizeof(version) +
        sizeof(outpoints) +
        sizeof(sequences) +
        sizeof(outpoint) +
        cs.Total() +
        sizeof(value) +
        sizeof(sequence) +
        sizeof(outputs) +
        sizeof(locktime) +
        sizeof(sigHash)
    );
    // clang-format on
    auto it = preimage.data();
    std::memcpy(it, &version, sizeof(version));
    std::advance(it, sizeof(version));
    std::memcpy(it, outpoints.data(), outpoints.size());
    std::advance(it, outpoints.size());
    std::memcpy(it, sequences.data(), sequences.size());
    std::advance(it, sequences.size());
    std::memcpy(it, &outpoint, sizeof(outpoint));
    std::advance(it, sizeof(outpoint));

    if (false == cs.Encode(preallocated(cs.Size(), it))) {
        throw std::runtime_error{"CompactSize encoding failure"};
    }

    std::advance(it, cs.Size());

    if (false == script.Serialize(preallocated(scriptBytes, it))) {
        throw std::runtime_error{"Script encoding failure"};
    }

    std::advance(it, scriptBytes);
    std::memcpy(it, &value, sizeof(value));
    std::advance(it, sizeof(value));
    std::memcpy(it, &sequence, sizeof(sequence));
    std::advance(it, sizeof(sequence));
    std::memcpy(it, outputs.data(), outputs.size());
    std::advance(it, outputs.size());
    std::memcpy(it, &locktime, sizeof(locktime));
    std::advance(it, sizeof(locktime));
    std::memcpy(it, &sigHash, sizeof(sigHash));

    return preimage;
}

auto Bip143Hashes::Sequences(const SigHash sig) const noexcept -> const Hash&
{
    if (sig.AnyoneCanPay() || (SigOption::All != sig.Type())) {
        return blank();
    }

    return sequences_;
}

auto EncodedInput::size() const noexcept -> std::size_t
{
    return sizeof(outpoint_) + cs_.Total() + sizeof(sequence_);
}

auto EncodedInputWitness::size() const noexcept -> std::size_t
{
    return cs_.Size() +
           std::accumulate(std::begin(items_), std::end(items_), 0u, cb);
}

auto EncodedOutput::size() const noexcept -> std::size_t
{
    return sizeof(value_) + cs_.Total();
}

auto EncodedTransaction::CalculateIDs(
    const api::Session& api,
    const blockchain::Type chain,
    ReadView bytes) noexcept -> bool
{
    auto output = TransactionHash(api, chain, bytes, writer(wtxid_));

    if (false == output) {
        LogError()(OT_PRETTY_CLASS())("Failed to calculate wtxid").Flush();

        return false;
    }

    if (segwit_flag_.has_value()) {
        const auto preimage = txid_preimage();
        output = TransactionHash(api, chain, reader(preimage), writer(txid_));
    } else {
        txid_ = wtxid_;
    }

    if (false == output) {
        LogError()(OT_PRETTY_CLASS())("Failed to calculate txid").Flush();

        return false;
    }

    return output;
}

auto EncodedTransaction::CalculateIDs(
    const api::Session& api,
    const blockchain::Type chain) noexcept -> bool
{
    const auto preimage = wtxid_preimage();

    return CalculateIDs(api, chain, reader(preimage));
}

auto EncodedTransaction::DefaultVersion(const blockchain::Type) noexcept
    -> std::uint32_t
{
    return 1;
}

auto EncodedTransaction::Deserialize(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView in) noexcept(false) -> EncodedTransaction
{
    if ((nullptr == in.data()) || (0 == in.size())) {
        throw std::runtime_error("Invalid bytes");
    }

    auto output = EncodedTransaction{};
    auto& [version, segwit, inCount, inputs, outCount, outputs, witnesses, locktime, wtxid, txid] =
        output;
    auto it = reinterpret_cast<ByteIterator>(in.data());
    const auto start{it};
    auto expectedSize = sizeof(version);

    if (in.size() < expectedSize) {
        throw std::runtime_error("Partial transaction (version)");
    }

    std::memcpy(static_cast<void*>(&version), it, sizeof(version));
    std::advance(it, sizeof(version));
    LogTrace()(OT_PRETTY_STATIC(EncodedTransaction))("Tx version: ")(
        version.value())
        .Flush();
    segwit = HasSegwit(it, expectedSize, in.size());

    if (segwit.has_value()) {
        LogTrace()(OT_PRETTY_STATIC(EncodedTransaction))(
            "Found segwit transaction flag: ")(
            std::to_integer<std::uint8_t>(segwit.value()))
            .Flush();
    } else {
        LogTrace()(OT_PRETTY_STATIC(EncodedTransaction))(
            "Found non-segwit transaction")
            .Flush();
    }

    expectedSize += 1;

    if (in.size() < expectedSize) {
        throw std::runtime_error("Partial transaction (txin count)");
    }

    if (false == network::blockchain::bitcoin::DecodeSize(
                     it, expectedSize, in.size(), inCount)) {
        throw std::runtime_error("Failed to decode txin count");
    }

    LogTrace()(OT_PRETTY_STATIC(EncodedTransaction))("Tx input count: ")(
        inCount.Value())
        .Flush();

    while (inputs.size() < inCount.Value()) {
        auto& input = inputs.emplace_back();
        auto& [outpoint, scriptBytes, script, sequence] = input;
        expectedSize += sizeof(outpoint);

        if (in.size() < expectedSize) {
            throw std::runtime_error("Partial input (outpoint)");
        }

        std::memcpy(static_cast<void*>(&outpoint), it, sizeof(outpoint));
        std::advance(it, sizeof(outpoint));
        expectedSize += 1;

        if (in.size() < expectedSize) {
            throw std::runtime_error("Partial input (script size)");
        }

        if (false == network::blockchain::bitcoin::DecodeSize(
                         it, expectedSize, in.size(), scriptBytes)) {
            throw std::runtime_error("Failed to decode input script bytes");
        }

        LogTrace()(OT_PRETTY_STATIC(EncodedTransaction))(
            "input script bytes: ")(scriptBytes.Value())
            .Flush();
        expectedSize += scriptBytes.Value();

        if (in.size() < expectedSize) {
            throw std::runtime_error("Partial input (script)");
        }

        script.assign(it, it + scriptBytes.Value());
        std::advance(it, scriptBytes.Value());
        expectedSize += sizeof(sequence);

        if (in.size() < expectedSize) {
            throw std::runtime_error("Partial input (sequence)");
        }

        std::memcpy(static_cast<void*>(&sequence), it, sizeof(sequence));
        std::advance(it, sizeof(sequence));
        LogTrace()(OT_PRETTY_STATIC(EncodedTransaction))("sequence: ")(
            sequence.value())
            .Flush();
    }

    expectedSize += 1;

    if (in.size() < expectedSize) {
        throw std::runtime_error("Partial transaction (txout count)");
    }

    if (false == network::blockchain::bitcoin::DecodeSize(
                     it, expectedSize, in.size(), outCount)) {
        throw std::runtime_error("Failed to decode txout count");
    }

    LogTrace()(OT_PRETTY_STATIC(EncodedTransaction))("Tx output count: ")(
        outCount.Value())
        .Flush();

    while (outputs.size() < outCount.Value()) {
        auto& output = outputs.emplace_back();
        auto& [value, scriptBytes, script] = output;
        expectedSize += sizeof(value);

        if (in.size() < expectedSize) {
            throw std::runtime_error("Partial output (value)");
        }

        std::memcpy(static_cast<void*>(&value), it, sizeof(value));
        std::advance(it, sizeof(value));
        LogTrace()(OT_PRETTY_STATIC(EncodedTransaction))("value: ")(
            value.value())
            .Flush();
        expectedSize += 1;

        if (in.size() < expectedSize) {
            throw std::runtime_error("Partial output (script size)");
        }

        if (false == network::blockchain::bitcoin::DecodeSize(
                         it, expectedSize, in.size(), scriptBytes)) {
            throw std::runtime_error("Failed to decode output script bytes");
        }

        LogTrace()(OT_PRETTY_STATIC(EncodedTransaction))(
            "output script bytes: ")(scriptBytes.Value())
            .Flush();
        expectedSize += scriptBytes.Value();

        if (in.size() < expectedSize) {
            throw std::runtime_error("Partial output (script)");
        }

        script.assign(it, it + scriptBytes.Value());
        std::advance(it, scriptBytes.Value());
    }

    if (segwit.has_value()) {
        for (auto i{0u}; i < inputs.size(); ++i) {
            auto& [witnessCount, witnessItems] = witnesses.emplace_back();

            if (false == network::blockchain::bitcoin::DecodeSize(
                             it, expectedSize, in.size(), witnessCount)) {
                throw std::runtime_error("Failed to witness item count");
            }

            LogTrace()(OT_PRETTY_STATIC(EncodedTransaction))("witness ")(
                i)(" contains ")(witnessCount.Value())(" pushes")
                .Flush();

            for (auto w{0u}; w < witnessCount.Value(); ++w) {
                auto& [witnessBytes, push] = witnessItems.emplace_back();

                if (false == network::blockchain::bitcoin::DecodeSize(
                                 it, expectedSize, in.size(), witnessBytes)) {
                    throw std::runtime_error("Failed to witness item bytes");
                }

                LogTrace()(OT_PRETTY_STATIC(EncodedTransaction))("push ")(
                    w)(" bytes: ")(witnessBytes.Value())
                    .Flush();
                expectedSize += witnessBytes.Value();

                if (in.size() < expectedSize) {
                    throw std::runtime_error("Partial witness item");
                }

                push.assign(it, it + witnessBytes.Value());
                std::advance(it, witnessBytes.Value());
            }
        }
    }

    if ((0 != witnesses.size()) && (witnesses.size() != inputs.size())) {
        throw std::runtime_error("Invalid witnesses count");
    }

    expectedSize += sizeof(locktime);

    if (in.size() < expectedSize) {
        throw std::runtime_error("Partial transaction (lock time)");
    }

    std::memcpy(static_cast<void*>(&locktime), it, sizeof(locktime));
    std::advance(it, sizeof(locktime));
    const auto txBytes = static_cast<std::size_t>(std::distance(start, it));
    const auto view = ReadView{in.data(), txBytes};
    LogTrace()(OT_PRETTY_STATIC(EncodedTransaction))("lock time: ")(
        locktime.value())
        .Flush();

    if (false == output.CalculateIDs(api, chain, view)) {
        throw std::runtime_error("Failed to calculate txid / wtxid");
    }

    OT_ASSERT(txBytes == output.size());

    return output;
}

auto EncodedTransaction::wtxid_preimage() const noexcept -> Space
{
    auto output = space(size());
    auto it = reinterpret_cast<std::byte*>(output.data());
    std::memcpy(it, static_cast<const void*>(&version_), sizeof(version_));
    std::advance(it, sizeof(version_));

    if (segwit_flag_.has_value()) {
        static const auto marker = std::byte{0x0};
        std::memcpy(it, static_cast<const void*>(&marker), sizeof(marker));
        std::advance(it, sizeof(marker));
        const auto& segwit = segwit_flag_.value();
        std::memcpy(it, static_cast<const void*>(&segwit), sizeof(segwit));
        std::advance(it, sizeof(segwit));
    }

    {
        const auto bytes = input_count_.Encode();
        std::memcpy(it, bytes.data(), bytes.size());
        std::advance(it, bytes.size());
    }

    for (const auto& [outpoint, cs, script, sequence] : inputs_) {
        const auto bytes = cs.Encode();
        std::memcpy(it, static_cast<const void*>(&outpoint), sizeof(outpoint));
        std::advance(it, sizeof(outpoint));
        std::memcpy(it, bytes.data(), bytes.size());
        std::advance(it, bytes.size());

        if (0u < script.size()) {
            std::memcpy(it, script.data(), script.size());
            std::advance(it, script.size());
        }

        std::memcpy(it, static_cast<const void*>(&sequence), sizeof(sequence));
        std::advance(it, sizeof(sequence));
    }

    {
        const auto bytes = output_count_.Encode();
        std::memcpy(it, bytes.data(), bytes.size());
        std::advance(it, bytes.size());
    }

    for (const auto& [value, cs, script] : outputs_) {
        const auto bytes = cs.Encode();
        std::memcpy(it, static_cast<const void*>(&value), sizeof(value));
        std::advance(it, sizeof(value));
        std::memcpy(it, bytes.data(), bytes.size());
        std::advance(it, bytes.size());

        if (0u < script.size()) {
            std::memcpy(it, script.data(), script.size());
            std::advance(it, script.size());
        }
    }

    if (segwit_flag_.has_value()) {
        for (const auto& input : witnesses_) {
            {
                const auto bytes = input.cs_.Encode();
                std::memcpy(it, bytes.data(), bytes.size());
                std::advance(it, bytes.size());
            }

            for (const auto& item : input.items_) {
                {
                    const auto bytes = item.cs_.Encode();
                    std::memcpy(it, bytes.data(), bytes.size());
                    std::advance(it, bytes.size());
                }

                if (0u < item.item_.size()) {
                    std::memcpy(it, item.item_.data(), item.item_.size());
                    std::advance(it, item.item_.size());
                }
            }
        }
    }

    std::memcpy(it, static_cast<const void*>(&lock_time_), sizeof(lock_time_));
    std::advance(it, sizeof(lock_time_));

    return output;
}

auto EncodedTransaction::txid_preimage() const noexcept -> Space
{
    auto output = space(txid_size());
    auto it = reinterpret_cast<std::byte*>(output.data());
    std::memcpy(it, static_cast<const void*>(&version_), sizeof(version_));
    std::advance(it, sizeof(version_));

    {
        const auto bytes = input_count_.Encode();
        std::memcpy(it, bytes.data(), bytes.size());
        std::advance(it, bytes.size());
    }

    for (const auto& [outpoint, cs, script, sequence] : inputs_) {
        const auto bytes = cs.Encode();
        std::memcpy(it, static_cast<const void*>(&outpoint), sizeof(outpoint));
        std::advance(it, sizeof(outpoint));
        std::memcpy(it, bytes.data(), bytes.size());
        std::advance(it, bytes.size());

        if (0u < script.size()) {
            std::memcpy(it, script.data(), script.size());
            std::advance(it, script.size());
        }

        std::memcpy(it, static_cast<const void*>(&sequence), sizeof(sequence));
        std::advance(it, sizeof(sequence));
    }

    {
        const auto bytes = output_count_.Encode();
        std::memcpy(it, bytes.data(), bytes.size());
        std::advance(it, bytes.size());
    }

    for (const auto& [value, cs, script] : outputs_) {
        const auto bytes = cs.Encode();
        std::memcpy(it, static_cast<const void*>(&value), sizeof(value));
        std::advance(it, sizeof(value));
        std::memcpy(it, bytes.data(), bytes.size());
        std::advance(it, bytes.size());

        if (0u < script.size()) {
            std::memcpy(it, script.data(), script.size());
        }

        std::advance(it, script.size());
    }

    std::memcpy(it, static_cast<const void*>(&lock_time_), sizeof(lock_time_));
    std::advance(it, sizeof(lock_time_));

    return output;
}

auto EncodedTransaction::txid_size() const noexcept -> std::size_t
{
    return sizeof(version_) + input_count_.Size() +
           std::accumulate(std::begin(inputs_), std::end(inputs_), 0u, cb) +
           output_count_.Size() +
           std::accumulate(std::begin(outputs_), std::end(outputs_), 0u, cb) +
           sizeof(lock_time_);
}

auto EncodedTransaction::size() const noexcept -> std::size_t
{
    return txid_size() +
           (segwit_flag_.has_value()
                ? std::accumulate(
                      std::begin(witnesses_), std::end(witnesses_), 2u, cb)
                : std::size_t{0});
}

auto EncodedWitnessItem::size() const noexcept -> std::size_t
{
    return cs_.Total();
}

constexpr auto All = std::byte{0x01};
constexpr auto None = std::byte{0x02};
constexpr auto Single = std::byte{0x03};
constexpr auto Fork_ID = std::byte{0x40};
constexpr auto Anyone_Can_Pay = std::byte{0x08};
constexpr auto test_anyone_can_pay(const std::byte& rhs) noexcept -> bool
{
    return (rhs & Anyone_Can_Pay) == Anyone_Can_Pay;
}
constexpr auto test_none(const std::byte& rhs) noexcept -> bool
{
    return (rhs & Single) == None;
}
constexpr auto test_single(const std::byte& rhs) noexcept -> bool
{
    return (rhs & Single) == Single;
}
constexpr auto test_all(const std::byte& rhs) noexcept -> bool
{
    return ((rhs & Single) | All) == All;
}

SigHash::SigHash(
    const blockchain::Type chain,
    const SigOption flag,
    const bool anyoneCanPay) noexcept
    : flags_(All)
    , forkid_()
{
    static_assert(sizeof(std::uint32_t) == sizeof(SigHash));

    static_assert(false == test_anyone_can_pay(std::byte{0x00}));
    static_assert(false == test_anyone_can_pay(std::byte{0x01}));
    static_assert(false == test_anyone_can_pay(std::byte{0x02}));
    static_assert(false == test_anyone_can_pay(std::byte{0x03}));
    static_assert(test_anyone_can_pay(std::byte{0x08}));
    static_assert(test_anyone_can_pay(std::byte{0x09}));
    static_assert(test_anyone_can_pay(std::byte{0x0a}));
    static_assert(test_anyone_can_pay(std::byte{0x0b}));

    static_assert(false == test_none(std::byte{0x00}));
    static_assert(false == test_none(std::byte{0x01}));
    static_assert(test_none(std::byte{0x02}));
    static_assert(false == test_none(std::byte{0x03}));
    static_assert(false == test_none(std::byte{0x08}));
    static_assert(false == test_none(std::byte{0x09}));
    static_assert(test_none(std::byte{0x0a}));
    static_assert(false == test_none(std::byte{0x0b}));

    static_assert(false == test_single(std::byte{0x00}));
    static_assert(false == test_single(std::byte{0x01}));
    static_assert(false == test_single(std::byte{0x02}));
    static_assert(test_single(std::byte{0x03}));
    static_assert(false == test_single(std::byte{0x08}));
    static_assert(false == test_single(std::byte{0x09}));
    static_assert(false == test_single(std::byte{0x0a}));
    static_assert(test_single(std::byte{0x0b}));

    static_assert(test_all(std::byte{0x00}));
    static_assert(test_all(std::byte{0x01}));
    static_assert(false == test_all(std::byte{0x02}));
    static_assert(false == test_all(std::byte{0x03}));
    static_assert(test_all(std::byte{0x08}));
    static_assert(test_all(std::byte{0x09}));
    static_assert(false == test_all(std::byte{0x0a}));
    static_assert(false == test_all(std::byte{0x0b}));

    switch (flag) {
        case SigOption::Single: {
            flags_ = Single;
        } break;
        case SigOption::None: {
            flags_ = None;
        } break;
        default: {
        }
    }

    if (anyoneCanPay) { flags_ |= Anyone_Can_Pay; }

    switch (chain) {
        case blockchain::Type::BitcoinCash:
        case blockchain::Type::BitcoinCash_testnet3:
        case blockchain::Type::BitcoinSV:
        case blockchain::Type::BitcoinSV_testnet3:
        case blockchain::Type::eCash:
        case blockchain::Type::eCash_testnet3: {
            flags_ |= Fork_ID;
            break;
        }
        case opentxs::blockchain::Type::Unknown:
        case opentxs::blockchain::Type::Bitcoin:
        case opentxs::blockchain::Type::Bitcoin_testnet3:
        case opentxs::blockchain::Type::Ethereum_frontier:
        case opentxs::blockchain::Type::Ethereum_ropsten:
        case opentxs::blockchain::Type::Litecoin:
        case opentxs::blockchain::Type::Litecoin_testnet4:
        case opentxs::blockchain::Type::PKT:
        case opentxs::blockchain::Type::PKT_testnet:
        case opentxs::blockchain::Type::UnitTest:
        default: {
        }
    }
}

auto SigHash::AnyoneCanPay() const noexcept -> bool
{
    return test_anyone_can_pay(flags_);
}

auto SigHash::begin() const noexcept -> const std::byte*
{
    return reinterpret_cast<const std::byte*>(this);
}

auto SigHash::end() const noexcept -> const std::byte*
{
    return begin() + sizeof(*this);
}

auto SigHash::ForkID() const noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(forkid_.data()), forkid_.size()};
}

auto SigHash::Type() const noexcept -> SigOption
{
    if (test_single(flags_)) {

        return SigOption::Single;
    } else if (test_none(flags_)) {

        return SigOption::None;
    }

    return SigOption::All;
}
}  // namespace opentxs::blockchain::bitcoin

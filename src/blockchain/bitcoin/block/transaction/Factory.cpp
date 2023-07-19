// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::ContactItemType

#include "internal/blockchain/bitcoin/block/Factory.hpp"  // IWYU pragma: associated

#include <BlockchainTransaction.pb.h>
#include <BlockchainTransactionInput.pb.h>
#include <BlockchainTransactionOutput.pb.h>
#include <ContactEnums.pb.h>
#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <array>
#include <cstring>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/block/transaction/Imp.hpp"
#include "blockchain/bitcoin/block/transaction/TransactionPrivate.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/block/Parser.hpp"
#include "internal/blockchain/token/Types.hpp"
#include "internal/core/Amount.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/blockchain/bitcoin/block/Position.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BitcoinTransaction(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const Time& time,
    const boost::endian::little_int32_buf_t& version,
    const boost::endian::little_uint32_buf_t lockTime,
    bool segwit,
    Vector<blockchain::bitcoin::block::Input> inputs,
    Vector<blockchain::bitcoin::block::Output> outputs,
    alloc::Strategy alloc) noexcept
    -> blockchain::bitcoin::block::TransactionPrivate*
{
    using ReturnType = blockchain::bitcoin::block::implementation::Transaction;
    using BlankType = blockchain::bitcoin::block::TransactionPrivate;

    try {
        using Encoded = blockchain::bitcoin::EncodedTransaction;

        auto raw = Encoded{};
        raw.version_ = version;
        raw.segwit_flag_ = segwit ? std::byte{0x01} : std::byte{0x00};
        raw.input_count_ = inputs.size();
        auto isGeneration{false};

        for (const auto& input : inputs) {
            raw.inputs_.emplace_back();
            auto& txin = *raw.inputs_.rbegin();
            const auto& outpoint = input.PreviousOutput();

            static_assert(
                sizeof(txin.outpoint_.txid_) == sizeof(outpoint.txid_));
            static_assert(
                sizeof(txin.outpoint_.index_) == sizeof(outpoint.index_));

            std::memcpy(
                txin.outpoint_.txid_.data(),
                outpoint.txid_.data(),
                outpoint.txid_.size());
            std::memcpy(
                std::addressof(txin.outpoint_.index_),
                outpoint.index_.data(),
                outpoint.index_.size());

            if (auto coinbase = input.Coinbase(); 0 < coinbase.size()) {
                txin.script_.Assign(coinbase);
                isGeneration = true;
            } else {
                input.Script().Serialize(txin.script_.WriteInto());
            }

            txin.cs_ = txin.script_.size();
            txin.sequence_ = input.Sequence();
        }

        raw.output_count_ = outputs.size();

        for (const auto& output : outputs) {
            raw.outputs_.emplace_back();
            auto& txout = *raw.outputs_.rbegin();
            try {
                output.Value().Internal().SerializeBitcoin(
                    preallocated(sizeof(txout.value_), &txout.value_));
            } catch (const std::exception& e) {
                LogError()("opentxs::factory::")(__func__)(": ")(e.what())
                    .Flush();

                return {};
            }
            output.Script().Serialize(txout.script_.WriteInto());
            txout.cs_ = txout.script_.size();
        }

        raw.lock_time_ = lockTime;
        raw.CalculateIDs(crypto, chain, isGeneration);

        return pmr::construct<ReturnType>(
            alloc.result_,
            ReturnType::default_version_,
            false,
            raw.version_.value(),
            raw.segwit_flag_.value(),
            raw.lock_time_.value(),
            raw.txid_,
            raw.wtxid_,
            time,
            UnallocatedCString{},
            std::move(inputs),
            std::move(outputs),
            std::move(raw.dip_2_),
            [&] {
                auto chains = Set<blockchain::Type>{alloc.result_};
                chains.emplace(chain);

                return chains;
            }(),
            blockchain::block::Position{},
            std::nullopt);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}

auto BitcoinTransaction(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const std::size_t position,
    const Time& time,
    ReadView native,
    alloc::Strategy alloc) noexcept -> blockchain::bitcoin::block::Transaction
{
    using blockchain::block::Parser;
    auto out = blockchain::bitcoin::block::Transaction{alloc.result_};

    if (Parser::Transaction(
            crypto, chain, position, time, native, out, alloc)) {

        return out;
    } else {
        LogError()("opentxs::factory::")(__func__)(": failed to parse ")(
            print(chain))(" transaction")
            .Flush();

        return {};
    }
}

auto BitcoinTransaction(
    const blockchain::Type chain,
    const std::size_t position,
    const Time& time,
    blockchain::bitcoin::EncodedTransaction&& parsed,
    alloc::Strategy alloc) noexcept
    -> blockchain::bitcoin::block::TransactionPrivate*
{
    using ReturnType = blockchain::bitcoin::block::implementation::Transaction;
    using BlankType = blockchain::bitcoin::block::TransactionPrivate;

    try {

        return pmr::construct<ReturnType>(
            alloc.result_,
            ReturnType::default_version_,
            0_uz == position,
            parsed.version_.value(),
            parsed.segwit_flag_.value_or(std::byte{0x0}),
            parsed.lock_time_.value(),
            parsed.txid_,
            parsed.wtxid_,
            time,
            UnallocatedCString{},
            [&] {
                const auto& inputs = parsed.inputs_;
                const auto inputCount = inputs.size();
                auto o =
                    Vector<blockchain::bitcoin::block::Input>{alloc.result_};
                o.reserve(inputCount);
                o.clear();

                for (auto i = 0_uz; i < inputCount; ++i) {
                    const auto& input = inputs.at(i);
                    const auto& op = input.outpoint_;
                    const auto& seq = input.sequence_;
                    const auto& witness = parsed.witnesses_;
                    const auto witnessCount = witness.size();
                    auto instantiatedWitness =
                        Vector<blockchain::bitcoin::block::WitnessItem>{
                            alloc.result_};
                    instantiatedWitness.reserve(witnessCount);
                    instantiatedWitness.clear();

                    if (0_uz < witnessCount) {
                        auto& encodedWitness = parsed.witnesses_.at(i);

                        for (auto& item : encodedWitness.items_) {
                            instantiatedWitness.emplace_back(
                                std::move(item.item_));
                        }
                    }

                    o.emplace_back(factory::BitcoinTransactionInput(
                        chain,
                        ReadView{
                            reinterpret_cast<const char*>(&op), sizeof(op)},
                        input.cs_,
                        input.script_.Bytes(),
                        ReadView{
                            reinterpret_cast<const char*>(&seq), sizeof(seq)},
                        (0_uz == position) && (0_uz == i),
                        instantiatedWitness,
                        alloc));
                }

                return o;
            }(),
            [&] {
                const auto& outputs = parsed.outputs_;
                const auto outputCount = outputs.size();
                auto o =
                    Vector<blockchain::bitcoin::block::Output>{alloc.result_};
                o.reserve(outputCount);
                o.clear();

                for (auto i = 0_uz; i < outputCount; ++i) {
                    const auto& output = outputs[i];
                    o.emplace_back(factory::BitcoinTransactionOutput(
                        chain,
                        static_cast<std::uint32_t>(i),
                        opentxs::Amount{output.value_.value()},
                        output.cs_,
                        output.script_.Bytes(),
                        output.cashtoken_,
                        alloc));
                }

                return o;
            }(),
            std::move(parsed.dip_2_),
            [&] {
                auto chains = Set<blockchain::Type>{alloc.result_};
                chains.emplace(chain);

                return chains;
            }(),
            blockchain::block::Position{},
            [&]() -> std::optional<std::size_t> {
                if (std::numeric_limits<std::size_t>::max() == position) {

                    return std::nullopt;
                } else {

                    return position;
                }
            }());
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}

auto BitcoinTransaction(
    const api::crypto::Blockchain& crypto,
    const api::Factory& factory,
    const proto::BlockchainTransaction& in,
    alloc::Strategy alloc) noexcept
    -> blockchain::bitcoin::block::TransactionPrivate*
{
    using ReturnType = blockchain::bitcoin::block::implementation::Transaction;
    using BlankType = blockchain::bitcoin::block::TransactionPrivate;

    try {
        auto chains = Set<blockchain::Type>{alloc.result_};
        std::transform(
            std::begin(in.chain()),
            std::end(in.chain()),
            std::inserter(chains, chains.end()),
            [](const auto type) -> auto {
                return UnitToBlockchain(ClaimToUnit(
                    translate(static_cast<proto::ContactItemType>(type))));
            });

        if (chains.empty()) { throw std::runtime_error{"invalid chains"}; }

        const auto& chain = *chains.cbegin();
        auto inputs = Vector<blockchain::bitcoin::block::Input>{alloc.result_};

        {
            const auto count = in.input().size();
            inputs.reserve(count);
            inputs.clear();
            auto map = Map<std::uint32_t, blockchain::bitcoin::block::Input>{
                alloc.result_};
            map.clear();

            for (const auto& input : in.input()) {
                const auto index = input.index();
                map.emplace(
                    index,
                    factory::BitcoinTransactionInput(
                        crypto,
                        factory,
                        chain,
                        input,
                        (0u == index) && in.is_generation(),
                        alloc));
            }

            std::transform(
                std::begin(map),
                std::end(map),
                std::back_inserter(inputs),
                [](auto& i) -> auto { return std::move(i.second); });
        }

        auto outputs =
            Vector<blockchain::bitcoin::block::Output>{alloc.result_};

        {
            const auto count = in.output().size();
            outputs.reserve(count);
            outputs.clear();
            auto map = Map<std::uint32_t, blockchain::bitcoin::block::Output>{
                alloc.result_};
            map.clear();

            for (const auto& output : in.output()) {
                const auto index = output.index();
                map.emplace(
                    index,
                    factory::BitcoinTransactionOutput(
                        crypto, factory, chain, output, alloc));
            }

            std::transform(
                std::begin(map),
                std::end(map),
                std::back_inserter(outputs),
                [](auto& i) -> auto { return std::move(i.second); });
        }

        return pmr::construct<ReturnType>(
            alloc.result_,
            in.version(),
            in.is_generation(),
            static_cast<std::int32_t>(in.txversion()),
            std::byte{static_cast<std::uint8_t>(in.segwit_flag())},
            in.locktime(),
            blockchain::block::TransactionHash{in.txid()},
            blockchain::block::TransactionHash{in.wtxid()},
            convert_stime(in.time()),
            in.memo(),
            inputs,
            outputs,
            [&]() -> std::optional<ByteArray> {
                if (in.is_dip_2()) {

                    return ByteArray{in.dip_2_extra_bytes()};
                } else {

                    return std::nullopt;
                }
            }(),
            std::move(chains),
            blockchain::block::Position{
                [&]() -> blockchain::block::Height {
                    if (in.has_mined_block()) {

                        return in.mined_height();
                    } else {

                        return -1;
                    }
                }(),
                [&] {
                    auto hashes = blockchain::block::Hash();

                    if (in.has_mined_block()) {
                        const auto rc = hashes.Assign(in.mined_block());

                        if (false == rc) {
                            throw std::runtime_error{"invalid mined_block"};
                        }
                    }

                    return hashes;
                }()},
            std::nullopt);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}

auto BitcoinTransaction(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const blockchain::block::Height height,
    std::span<blockchain::OutputBuilder> scripts,
    ReadView coinbase,
    std::int32_t version,
    alloc::Strategy alloc) noexcept
    -> blockchain::bitcoin::block::TransactionPrivate*
{
    static const auto outpoint = blockchain::block::Outpoint{};

    const auto serializedVersion = boost::endian::little_int32_buf_t{version};
    const auto locktime = boost::endian::little_uint32_buf_t{0};
    const auto sequence = boost::endian::little_uint32_buf_t{0xffffffff};
    const auto cb = [&] {
        const auto bip34 =
            blockchain::bitcoin::block::internal::EncodeBip34(height);
        auto output = space(bip34.size() + coinbase.size());
        auto* it = output.data();
        std::memcpy(it, bip34.data(), bip34.size());
        std::advance(it, bip34.size());
        std::memcpy(it, coinbase.data(), coinbase.size());
        output.resize(std::min(output.size(), 100_uz));

        return output;
    }();
    const auto cs = blockchain::bitcoin::CompactSize{cb.size()};
    auto inputs = Vector<blockchain::bitcoin::block::Input>{alloc.result_};
    inputs.reserve(1_uz);
    inputs.clear();
    inputs.emplace_back(factory::BitcoinTransactionInput(
        chain,
        outpoint.Bytes(),
        cs,
        reader(cb),
        ReadView{reinterpret_cast<const char*>(&sequence), sizeof(sequence)},
        true,
        {},
        alloc));
    auto outputs = Vector<blockchain::bitcoin::block::Output>{alloc.result_};
    outputs.reserve(scripts.size());
    outputs.clear();
    auto index{-1};
    using enum blockchain::bitcoin::block::script::Position;

    for (auto& [amount, script, keys] : scripts) {
        if (false == script.IsValid()) {

            return blockchain::bitcoin::block::TransactionPrivate::Blank(
                alloc.result_);
        }

        auto bytes = Space{};
        script.Serialize(writer(bytes));
        outputs.emplace_back(factory::BitcoinTransactionOutput(
            chain,
            static_cast<std::uint32_t>(++index),
            opentxs::Amount{amount},
            factory::BitcoinScript(
                chain, reader(bytes), Output, true, false, alloc),
            std::nullopt,  // TODO cashtoken
            std::move(keys),
            alloc));
    }

    return factory::BitcoinTransaction(
        crypto,
        chain,
        Clock::now(),
        serializedVersion,
        locktime,
        false,  // TODO segwit
        inputs,
        outputs,
        alloc);
}
}  // namespace opentxs::factory

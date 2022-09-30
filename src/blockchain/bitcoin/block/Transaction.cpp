// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "blockchain/bitcoin/block/Transaction.hpp"  // IWYU pragma: associated

#include <BlockchainTransaction.pb.h>
#include <BlockchainTransactionInput.pb.h>
#include <BlockchainTransactionOutput.pb.h>
#include <ContactEnums.pb.h>
#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <cstring>
#include <iterator>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <type_traits>  // IWYU pragma: keep
#include <utility>

#include "Proto.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Input.hpp"   // IWYU pragma: keep
#include "internal/blockchain/bitcoin/block/Output.hpp"  // IWYU pragma: keep
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/block/Block.hpp"  // IWYU pragma: keep
#include "internal/core/Amount.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/blockchain/bitcoin/block/Outputs.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Algorithm.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Container.hpp"

namespace be = boost::endian;

namespace opentxs::factory
{
auto BitcoinTransaction(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const Time& time,
    const boost::endian::little_int32_buf_t& version,
    const boost::endian::little_uint32_buf_t lockTime,
    bool segwit,
    std::unique_ptr<blockchain::bitcoin::block::internal::Inputs> inputs,
    std::unique_ptr<blockchain::bitcoin::block::internal::Outputs>
        outputs) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Transaction>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Transaction;

    OT_ASSERT(inputs);
    OT_ASSERT(outputs);

    using Encoded = blockchain::bitcoin::EncodedTransaction;

    auto raw = Encoded{};
    raw.version_ = version;
    raw.segwit_flag_ = segwit ? std::byte{0x01} : std::byte{0x00};
    raw.input_count_ = inputs->size();

    for (const auto& input : *inputs) {
        raw.inputs_.emplace_back();
        auto& out = *raw.inputs_.rbegin();
        const auto& outpoint = input.PreviousOutput();

        static_assert(sizeof(out.outpoint_) == sizeof(outpoint));

        std::memcpy(
            &out.outpoint_,
            static_cast<const void*>(&outpoint),
            sizeof(outpoint));

        if (auto coinbase = input.Coinbase(); 0 < coinbase.size()) {
            std::swap(out.script_, coinbase);
        } else {
            input.Script().Serialize(writer(out.script_));
        }

        out.cs_ = out.script_.size();
        out.sequence_ = input.Sequence();
    }

    raw.output_count_ = outputs->size();

    for (const auto& output : *outputs) {
        raw.outputs_.emplace_back();
        auto& out = *raw.outputs_.rbegin();
        try {
            output.Value().Internal().SerializeBitcoin(
                preallocated(sizeof(out.value_), &out.value_));
        } catch (const std::exception& e) {
            LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

            return {};
        }
        output.Script().Serialize(writer(out.script_));
        out.cs_ = out.script_.size();
    }

    raw.lock_time_ = lockTime;
    raw.CalculateIDs(crypto, chain);

    try {
        return std::make_unique<ReturnType>(
            ReturnType::default_version_,
            false,
            raw.version_.value(),
            raw.segwit_flag_.value(),
            raw.lock_time_.value(),
            ByteArray{reader(raw.txid_)},
            ByteArray{reader(raw.wtxid_)},
            time,
            UnallocatedCString{},
            std::move(inputs),
            std::move(outputs),
            UnallocatedVector<blockchain::Type>{chain},
            blockchain::block::Position{});
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinTransaction(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const std::size_t position,
    const Time& time,
    ReadView native) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Transaction>
{
    try {
        return BitcoinTransaction(
            chain,
            position,
            time,
            blockchain::bitcoin::EncodedTransaction::Deserialize(
                crypto, chain, native));
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinTransaction(
    const blockchain::Type chain,
    const std::size_t position,
    const Time& time,
    blockchain::bitcoin::EncodedTransaction&& parsed) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Transaction>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Transaction;

    try {
        auto inputBytes = 0_uz;
        auto instantiatedInputs = UnallocatedVector<
            std::unique_ptr<blockchain::bitcoin::block::internal::Input>>{};
        {
            auto counter = int{0};
            const auto& inputs = parsed.inputs_;
            instantiatedInputs.reserve(inputs.size());

            for (auto i{0u}; i < inputs.size(); ++i) {
                const auto& input = inputs.at(i);
                const auto& op = input.outpoint_;
                const auto& seq = input.sequence_;
                auto witness = UnallocatedVector<Space>{};

                if (0 < parsed.witnesses_.size()) {
                    const auto& encodedWitness = parsed.witnesses_.at(i);

                    for (const auto& [cs, bytes] : encodedWitness.items_) {
                        witness.emplace_back(bytes);
                    }
                }

                instantiatedInputs.emplace_back(
                    factory::BitcoinTransactionInput(
                        chain,
                        ReadView{
                            reinterpret_cast<const char*>(&op), sizeof(op)},
                        input.cs_,
                        reader(input.script_),
                        ReadView{
                            reinterpret_cast<const char*>(&seq), sizeof(seq)},
                        (0 == position) && (0 == counter),
                        std::move(witness)));
                ++counter;
                inputBytes += input.size();
            }

            const auto cs = blockchain::bitcoin::CompactSize{inputs.size()};
            inputBytes += cs.Size();
            instantiatedInputs.shrink_to_fit();
        }

        auto outputBytes = 0_uz;
        auto instantiatedOutputs = UnallocatedVector<
            std::unique_ptr<blockchain::bitcoin::block::internal::Output>>{};
        {
            instantiatedOutputs.reserve(parsed.outputs_.size());
            auto counter = std::uint32_t{0};

            for (const auto& output : parsed.outputs_) {
                instantiatedOutputs.emplace_back(
                    factory::BitcoinTransactionOutput(
                        chain,
                        counter++,
                        opentxs::Amount{output.value_.value()},
                        output.cs_,
                        reader(output.script_)));
                outputBytes += output.size();
            }

            const auto cs =
                blockchain::bitcoin::CompactSize{parsed.outputs_.size()};
            outputBytes += cs.Size();
            instantiatedOutputs.shrink_to_fit();
        }

        auto outputs =
            std::unique_ptr<const blockchain::bitcoin::block::Outputs>{};

        return std::make_unique<ReturnType>(
            ReturnType::default_version_,
            0 == position,
            parsed.version_.value(),
            parsed.segwit_flag_.value_or(std::byte{0x0}),
            parsed.lock_time_.value(),
            ByteArray{reader(parsed.txid_)},
            ByteArray{reader(parsed.wtxid_)},
            time,
            UnallocatedCString{},
            factory::BitcoinTransactionInputs(
                std::move(instantiatedInputs), inputBytes),
            factory::BitcoinTransactionOutputs(
                std::move(instantiatedOutputs), outputBytes),
            UnallocatedVector<blockchain::Type>{chain},
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

        return {};
    }
}

auto BitcoinTransaction(
    const api::crypto::Blockchain& crypto,
    const api::Factory& factory,
    const proto::BlockchainTransaction& in) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Transaction>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Transaction;
    auto chains = UnallocatedVector<blockchain::Type>{};
    std::transform(
        std::begin(in.chain()),
        std::end(in.chain()),
        std::back_inserter(chains),
        [](const auto type) -> auto{
            return UnitToBlockchain(ClaimToUnit(
                translate(static_cast<proto::ContactItemType>(type))));
        });

    if (0 == chains.size()) {
        LogError()("opentxs::factory::")(__func__)(": Invalid chains").Flush();

        return {};
    }

    const auto& chain = chains.at(0);

    try {
        auto inputs = UnallocatedVector<
            std::unique_ptr<blockchain::bitcoin::block::internal::Input>>{};

        {
            auto map = UnallocatedMap<
                std::uint32_t,
                std::unique_ptr<blockchain::bitcoin::block::internal::Input>>{};

            for (const auto& input : in.input()) {
                const auto index = input.index();
                map.emplace(
                    index,
                    factory::BitcoinTransactionInput(
                        crypto,
                        factory,
                        chain,
                        input,
                        (0u == index) && in.is_generation()));
            }

            std::transform(
                std::begin(map), std::end(map), std::back_inserter(inputs), [
                ](auto& in) -> auto{ return std::move(in.second); });
        }

        auto outputs = UnallocatedVector<
            std::unique_ptr<blockchain::bitcoin::block::internal::Output>>{};

        {
            auto map = UnallocatedMap<
                std::uint32_t,
                std::unique_ptr<
                    blockchain::bitcoin::block::internal::Output>>{};

            for (const auto& output : in.output()) {
                const auto index = output.index();
                map.emplace(
                    index,
                    factory::BitcoinTransactionOutput(
                        crypto, factory, chain, output));
            }

            std::transform(
                std::begin(map), std::end(map), std::back_inserter(outputs), [
                ](auto& in) -> auto{ return std::move(in.second); });
        }

        return std::make_unique<ReturnType>(
            in.version(),
            in.is_generation(),
            static_cast<std::int32_t>(in.txversion()),
            std::byte{static_cast<std::uint8_t>(in.segwit_flag())},
            in.locktime(),
            ByteArray{(in.txid())},
            ByteArray{(in.wtxid())},
            convert_stime(in.time()),
            in.memo(),
            factory::BitcoinTransactionInputs(std::move(inputs)),
            factory::BitcoinTransactionOutputs(std::move(outputs)),
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
                    auto out = blockchain::block::Hash();

                    if (in.has_mined_block()) {
                        const auto rc = out.Assign(in.mined_block());

                        if (false == rc) {
                            throw std::runtime_error{"invalid mined_block"};
                        }
                    }

                    return out;
                }()});
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinTransaction(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const blockchain::block::Height height,
    UnallocatedVector<blockchain::OutputBuilder>&& scripts,
    const UnallocatedCString& coinbase,
    const std::int32_t version) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Transaction>
{
    static const auto outpoint = opentxs::blockchain::block::Outpoint{};

    const auto serializedVersion = boost::endian::little_int32_buf_t{version};
    const auto locktime = boost::endian::little_uint32_buf_t{0};
    const auto sequence = boost::endian::little_uint32_buf_t{0xffffffff};
    const auto cb = [&] {
        const auto bip34 =
            opentxs::blockchain::bitcoin::block::internal::EncodeBip34(height);
        auto output = space(bip34.size() + coinbase.size());
        auto* it = output.data();
        std::memcpy(it, bip34.data(), bip34.size());
        std::advance(it, bip34.size());
        std::memcpy(it, coinbase.data(), coinbase.size());
        output.resize(std::min(output.size(), 100_uz));

        return output;
    }();
    const auto cs = opentxs::blockchain::bitcoin::CompactSize{cb.size()};
    auto inputs = UnallocatedVector<std::unique_ptr<
        opentxs::blockchain::bitcoin::block::internal::Input>>{};
    inputs.emplace_back(factory::BitcoinTransactionInput(
        chain,
        outpoint.Bytes(),
        cs,
        reader(cb),
        ReadView{reinterpret_cast<const char*>(&sequence), sizeof(sequence)},
        true,
        {}));
    auto outputs = UnallocatedVector<std::unique_ptr<
        opentxs::blockchain::bitcoin::block::internal::Output>>{};
    auto index{-1};
    using Position = opentxs::blockchain::bitcoin::block::Script::Position;

    for (auto& [amount, pScript, keys] : scripts) {
        if (false == bool(pScript)) { return {}; }

        const auto& script = *pScript;
        auto bytes = Space{};
        script.Serialize(writer(bytes));
        outputs.emplace_back(factory::BitcoinTransactionOutput(
            chain,
            static_cast<std::uint32_t>(++index),
            opentxs::Amount{amount},
            factory::BitcoinScript(chain, reader(bytes), Position::Output),
            std::move(keys)));
    }

    return factory::BitcoinTransaction(
        crypto,
        chain,
        Clock::now(),
        serializedVersion,
        locktime,
        false,  // TODO segwit
        factory::BitcoinTransactionInputs(std::move(inputs)),
        factory::BitcoinTransactionOutputs(std::move(outputs)));
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::bitcoin::block::implementation
{
const VersionNumber Transaction::default_version_{1};

Transaction::Transaction(
    const VersionNumber serializeVersion,
    const bool isGeneration,
    const std::int32_t version,
    const std::byte segwit,
    const std::uint32_t lockTime,
    const blockchain::block::pTxid&& txid,
    const blockchain::block::pTxid&& wtxid,
    const Time& time,
    const UnallocatedCString& memo,
    std::unique_ptr<internal::Inputs> inputs,
    std::unique_ptr<internal::Outputs> outputs,
    UnallocatedVector<blockchain::Type>&& chains,
    blockchain::block::Position&& minedPosition,
    std::optional<std::size_t>&& position) noexcept(false)
    : position_(std::move(position))
    , serialize_version_(serializeVersion)
    , is_generation_(isGeneration)
    , version_(version)
    , segwit_flag_(segwit)
    , lock_time_(lockTime)
    , txid_(std::move(txid))
    , wtxid_(std::move(wtxid))
    , time_(time)
    , inputs_(std::move(inputs))
    , outputs_(std::move(outputs))
    , cache_(memo, std::move(chains), std::move(minedPosition))
{
    if (false == bool(inputs_)) { throw std::runtime_error("invalid inputs"); }

    if (false == bool(outputs_)) {
        throw std::runtime_error("invalid outputs");
    }
}

Transaction::Transaction(const Transaction& rhs) noexcept
    : position_(rhs.position_)
    , serialize_version_(rhs.serialize_version_)
    , is_generation_(rhs.is_generation_)
    , version_(rhs.version_)
    , segwit_flag_(rhs.segwit_flag_)
    , lock_time_(rhs.lock_time_)
    , txid_(rhs.txid_)
    , wtxid_(rhs.wtxid_)
    , time_(rhs.time_)
    , inputs_(rhs.inputs_->clone())
    , outputs_(rhs.outputs_->clone())
    , cache_(rhs.cache_)
{
}

auto Transaction::AssociatedLocalNyms(const api::crypto::Blockchain& crypto)
    const noexcept -> UnallocatedVector<identifier::Nym>
{
    auto output = UnallocatedVector<identifier::Nym>{};
    inputs_->AssociatedLocalNyms(crypto, output);
    outputs_->AssociatedLocalNyms(crypto, output);
    dedup(output);

    return output;
}

auto Transaction::AssociatedRemoteContacts(
    const api::session::Client& api,
    const identifier::Nym& nym) const noexcept
    -> UnallocatedVector<identifier::Generic>
{
    auto output = UnallocatedVector<identifier::Generic>{};
    inputs_->AssociatedRemoteContacts(api, output);
    outputs_->AssociatedRemoteContacts(api, output);
    dedup(output);
    const auto mask = api.Contacts().ContactID(nym);
    output.erase(std::remove(output.begin(), output.end(), mask), output.end());

    return output;
}

auto Transaction::base_size() const noexcept -> std::size_t
{
    static constexpr auto fixed = sizeof(version_) + sizeof(lock_time_);

    return fixed + inputs_->CalculateSize(false) + outputs_->CalculateSize();
}

auto Transaction::calculate_size(const bool normalize) const noexcept
    -> std::size_t
{
    return cache_.size(normalize, [&] {
        const bool isSegwit =
            (false == normalize) && (std::byte{0x00} != segwit_flag_);

        return sizeof(version_) + inputs_->CalculateSize(normalize) +
               outputs_->CalculateSize() +
               (isSegwit ? calculate_witness_size() : 0_uz) +
               sizeof(lock_time_);
    });
}

auto Transaction::calculate_witness_size(const Space& in) noexcept
    -> std::size_t
{
    return blockchain::bitcoin::CompactSize{in.size()}.Total();
}

auto Transaction::calculate_witness_size(
    const UnallocatedVector<Space>& in) noexcept -> std::size_t
{
    const auto cs = blockchain::bitcoin::CompactSize{in.size()};

    return std::accumulate(
        std::begin(in),
        std::end(in),
        cs.Size(),
        [](const auto& previous, const auto& input) -> std::size_t {
            return previous + calculate_witness_size(input);
        });
}

auto Transaction::calculate_witness_size() const noexcept -> std::size_t
{
    return std::accumulate(
        std::begin(*inputs_),
        std::end(*inputs_),
        2_uz,  // NOTE: marker byte and segwit flag byte
        [](const std::size_t previous, const auto& input) -> std::size_t {
            return previous + calculate_witness_size(input.Witness());
        });
}

auto Transaction::IDNormalized(const api::Factory& factory) const noexcept
    -> const identifier::Generic&
{
    return cache_.normalized([&] {
        auto preimage = Space{};
        const auto serialized = serialize(writer(preimage), true);

        OT_ASSERT(serialized);

        return factory.IdentifierFromPreimage(
            reader(preimage), identifier::Algorithm::sha256);
    });
}

auto Transaction::ExtractElements(const cfilter::Type style) const noexcept
    -> Vector<Vector<std::byte>>
{
    auto output = inputs_->ExtractElements(style);
    LogTrace()(OT_PRETTY_CLASS())("extracted ")(output.size())(
        " input elements")
        .Flush();
    auto temp = outputs_->ExtractElements(style);
    LogTrace()(OT_PRETTY_CLASS())("extracted ")(temp.size())(" output elements")
        .Flush();
    output.insert(
        output.end(),
        std::make_move_iterator(temp.begin()),
        std::make_move_iterator(temp.end()));

    if (cfilter::Type::ES == style) {
        const auto* data = static_cast<const std::byte*>(txid_.data());
        output.emplace_back(data, data + txid_.size());
    }

    LogTrace()(OT_PRETTY_CLASS())("extracted ")(output.size())(
        " total elements")
        .Flush();
    std::sort(output.begin(), output.end());

    return output;
}

auto Transaction::FindMatches(
    const api::Session& api,
    const cfilter::Type style,
    const blockchain::block::Patterns& txos,
    const blockchain::block::ParsedPatterns& elements,
    const Log& log) const noexcept -> blockchain::block::Matches
{
    log(OT_PRETTY_CLASS())("processing transaction ").asHex(ID()).Flush();
    auto output = inputs_->FindMatches(api, txid_, style, txos, elements, log);
    auto& [inputs, outputs] = output;
    auto temp = outputs_->FindMatches(api, txid_, style, elements, log);
    inputs.insert(
        inputs.end(),
        std::make_move_iterator(temp.first.begin()),
        std::make_move_iterator(temp.first.end()));
    outputs.insert(
        outputs.end(),
        std::make_move_iterator(temp.second.begin()),
        std::make_move_iterator(temp.second.end()));

    return output;
}

auto Transaction::GetPatterns(const api::Session& api) const noexcept
    -> UnallocatedVector<PatternID>
{
    auto output = inputs_->GetPatterns(api);
    const auto oPatterns = outputs_->GetPatterns(api);
    output.reserve(output.size() + oPatterns.size());
    output.insert(output.end(), oPatterns.begin(), oPatterns.end());
    dedup(output);

    return output;
}

auto Transaction::GetPreimageBTC(
    const std::size_t index,
    const blockchain::bitcoin::SigHash& hashType) const noexcept -> Space
{
    if (SigHash::All != hashType.Type()) {
        // TODO
        LogError()(OT_PRETTY_CLASS())("Mode not supported").Flush();

        return {};
    }

    auto copy = Transaction{*this};
    copy.cache_.reset_size();

    if (false == copy.inputs_->ReplaceScript(index)) {
        LogError()(OT_PRETTY_CLASS())("Failed to initialize input script")
            .Flush();

        return {};
    }

    if (hashType.AnyoneCanPay() && (!copy.inputs_->AnyoneCanPay(index))) {
        LogError()(OT_PRETTY_CLASS())("Failed to apply AnyoneCanPay flag")
            .Flush();

        return {};
    }

    auto output = Space{};
    copy.Serialize(writer(output));

    return output;
}

auto Transaction::Keys() const noexcept -> UnallocatedVector<crypto::Key>
{
    auto out = inputs_->Keys();
    auto keys = outputs_->Keys();
    std::move(keys.begin(), keys.end(), std::back_inserter(out));
    dedup(out);

    return out;
}

auto Transaction::Memo(const api::crypto::Blockchain& crypto) const noexcept
    -> UnallocatedCString
{
    if (auto memo = cache_.memo(); false == memo.empty()) { return memo; }

    for (const auto& output : *outputs_) {
        auto note = output.Note(crypto);

        if (false == note.empty()) { return note; }
    }

    return {};
}

auto Transaction::MergeMetadata(
    const api::crypto::Blockchain& crypto,
    const blockchain::Type chain,
    const internal::Transaction& rhs,
    const Log& log) noexcept -> void
{
    log(OT_PRETTY_CLASS())("merging transaction ").asHex(ID()).Flush();

    if (txid_ != rhs.ID()) {
        LogError()(OT_PRETTY_CLASS())("Wrong transaction").Flush();

        return;
    }

    if (false == inputs_->MergeMetadata(rhs.Inputs().Internal(), log)) {
        LogError()(OT_PRETTY_CLASS())("Failed to merge inputs").Flush();

        return;
    }

    if (false == outputs_->MergeMetadata(rhs.Outputs().Internal(), log)) {
        LogError()(OT_PRETTY_CLASS())("Failed to merge outputs").Flush();

        return;
    }

    cache_.merge(crypto, rhs, log);
}

auto Transaction::NetBalanceChange(
    const api::crypto::Blockchain& crypto,
    const identifier::Nym& nym) const noexcept -> opentxs::Amount
{
    const auto& log = LogTrace();
    log(OT_PRETTY_CLASS())("parsing transaction ").asHex(ID()).Flush();
    const auto spent = inputs_->NetBalanceChange(crypto, nym, log);
    const auto created = outputs_->NetBalanceChange(crypto, nym, log);
    const auto total = spent + created;
    log(OT_PRETTY_CLASS())
        .asHex(ID())(" total input contribution is ")(
            spent)(" and total output contribution is ")(
            created)(" for a net balance change of ")(total)
        .Flush();

    return total;
}

auto Transaction::Print() const noexcept -> UnallocatedCString
{
    auto out = std::stringstream{};
    out << "  version: " << std::to_string(version_) << '\n';
    auto count{0};
    const auto inputs = Inputs().size();
    const auto outputs = Outputs().size();

    for (const auto& input : Inputs()) {
        out << "  input " << std::to_string(++count);
        out << " of " << std::to_string(inputs) << '\n';
        out << input.Print();
    }

    count = 0;

    for (const auto& output : Outputs()) {
        out << "  output " << std::to_string(++count);
        out << " of " << std::to_string(outputs) << '\n';
        out << output.Print();
    }

    out << "  locktime: " << std::to_string(lock_time_) << '\n';

    return out.str();
}

auto Transaction::serialize(
    const AllocateOutput destination,
    const bool normalize) const noexcept -> std::optional<std::size_t>
{
    if (!destination) {
        LogError()(OT_PRETTY_CLASS())("Invalid output allocator").Flush();

        return std::nullopt;
    }

    const auto size = calculate_size(normalize);
    auto output = destination(size);

    if (false == output.valid(size)) {
        LogError()(OT_PRETTY_CLASS())("Failed to allocate output bytes")
            .Flush();

        return std::nullopt;
    }

    const auto version = be::little_int32_buf_t{version_};
    const auto lockTime = be::little_uint32_buf_t{lock_time_};
    auto remaining{output.size()};
    auto* it = static_cast<std::byte*>(output.data());

    if (remaining < sizeof(version)) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed to serialize version. Need at least ")(sizeof(version))(
            " bytes but only have ")(remaining)
            .Flush();

        return std::nullopt;
    }

    std::memcpy(static_cast<void*>(it), &version, sizeof(version));
    std::advance(it, sizeof(version));
    remaining -= sizeof(version);
    const bool isSegwit =
        (false == normalize) && (std::byte{0x00} != segwit_flag_);

    if (isSegwit) {
        if (remaining < sizeof(std::byte)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed to serialize marker byte. Need at least ")(
                sizeof(std::byte))(" bytes but only have ")(remaining)
                .Flush();

            return std::nullopt;
        }

        *it = std::byte{0x00};
        std::advance(it, sizeof(std::byte));
        remaining -= sizeof(std::byte);

        if (remaining < sizeof(segwit_flag_)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed to serialize segwit flag. Need at least ")(
                sizeof(segwit_flag_))(" bytes but only have ")(remaining)
                .Flush();

            return std::nullopt;
        }

        *it = segwit_flag_;
        std::advance(it, sizeof(segwit_flag_));
        remaining -= sizeof(segwit_flag_);
    }

    const auto inputs =
        normalize ? inputs_->SerializeNormalized(preallocated(remaining, it))
                  : inputs_->Serialize(preallocated(remaining, it));

    if (inputs.has_value()) {
        std::advance(it, inputs.value());
        remaining -= inputs.value();
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize inputs").Flush();

        return std::nullopt;
    }

    const auto outputs = outputs_->Serialize(preallocated(remaining, it));

    if (outputs.has_value()) {
        std::advance(it, outputs.value());
        remaining -= outputs.value();
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize outputs").Flush();

        return std::nullopt;
    }

    if (isSegwit) {
        for (const auto& input : *inputs_) {
            const auto& witness = input.Witness();
            const auto pushCount =
                blockchain::bitcoin::CompactSize{witness.size()};

            if (false == pushCount.Encode(preallocated(remaining, it))) {
                LogError()(OT_PRETTY_CLASS())("Failed to serialize push count")
                    .Flush();

                return std::nullopt;
            }

            std::advance(it, pushCount.Size());
            remaining -= pushCount.Size();

            for (const auto& push : witness) {
                const auto pushSize =
                    blockchain::bitcoin::CompactSize{push.size()};

                if (false == pushSize.Encode(preallocated(remaining, it))) {
                    LogError()(OT_PRETTY_CLASS())(
                        "Failed to serialize push size")
                        .Flush();

                    return std::nullopt;
                }

                std::advance(it, pushSize.Size());
                remaining -= pushSize.Size();

                if (remaining < push.size()) {
                    LogError()(OT_PRETTY_CLASS())(
                        "Failed to serialize witness push. Need at least ")(
                        push.size())(" bytes but only have ")(remaining)
                        .Flush();

                    return std::nullopt;
                }

                if (0u < push.size()) {
                    std::memcpy(it, push.data(), push.size());
                }

                std::advance(it, push.size());
                remaining -= push.size();
            }
        }
    }

    if (remaining != sizeof(lockTime)) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed to serialize lock time. Need exactly ")(sizeof(lockTime))(
            " bytes but have ")(remaining)
            .Flush();

        return std::nullopt;
    }

    std::memcpy(static_cast<void*>(it), &lockTime, sizeof(lockTime));
    std::advance(it, sizeof(lockTime));
    // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
    remaining -= sizeof(lockTime);

    return size;
}

auto Transaction::Serialize(const AllocateOutput destination) const noexcept
    -> std::optional<std::size_t>
{
    return serialize(destination, false);
}

auto Transaction::Serialize(const api::Session& api) const noexcept
    -> std::optional<SerializeType>
{
    auto output = SerializeType{};
    output.set_version(std::max(default_version_, serialize_version_));

    for (const auto chain : cache_.chains()) {
        output.add_chain(translate(UnitToClaim(BlockchainToUnit(chain))));
    }

    output.set_txid(UnallocatedCString{txid_.Bytes()});
    output.set_txversion(version_);
    output.set_locktime(lock_time_);

    if (false == Serialize(writer(*output.mutable_serialized())).has_value()) {
        return {};
    }

    if (false == inputs_->Serialize(api, output)) { return {}; }

    if (false == outputs_->Serialize(api, output)) { return {}; }

    // TODO optional uint32 confirmations = 9;
    // TODO optional string blockhash = 10;
    // TODO optional uint32 blockindex = 11;
    // TODO optional uint64 fee = 12;
    output.set_time(Clock::to_time_t(time_));
    // TODO repeated string conflicts = 14;
    output.set_memo(cache_.memo());
    output.set_segwit_flag(std::to_integer<std::uint32_t>(segwit_flag_));
    output.set_wtxid(UnallocatedCString{wtxid_.Bytes()});
    output.set_is_generation(is_generation_);
    const auto& [height, hash] = cache_.position();

    if ((0 <= height) && (false == hash.IsNull())) {
        output.set_mined_height(height);
        output.set_mined_block(UnallocatedCString{hash.Bytes()});
    }

    return output;
}

auto Transaction::SetKeyData(const blockchain::block::KeyData& data) noexcept
    -> void
{
    inputs_->SetKeyData(data);
    outputs_->SetKeyData(data);
}

auto Transaction::vBytes(blockchain::Type chain) const noexcept -> std::size_t
{
    const auto& data = params::get(chain);
    const auto total = CalculateSize();

    if (data.SupportsSegwit()) {
        const auto& scale = data.SegwitScaleFactor();

        OT_ASSERT(0 < scale);

        const auto weight = (base_size() * (scale - 1u)) + total;
        static constexpr auto ceil = [](const auto a, const auto b) {
            return (a + (b - 1u)) / b;
        };

        return ceil(weight, scale);
    } else {

        return CalculateSize();
    }
}
}  // namespace opentxs::blockchain::bitcoin::block::implementation

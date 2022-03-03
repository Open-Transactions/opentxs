// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "1_Internal.hpp"                            // IWYU pragma: associated
#include "blockchain/block/bitcoin/Transaction.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <cstring>
#include <iterator>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "Proto.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/block/Block.hpp"  // IWYU pragma: keep
#include "internal/blockchain/node/Node.hpp"
#include "internal/core/Amount.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Algorithm.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/BlockchainTransaction.pb.h"
#include "serialization/protobuf/BlockchainTransactionInput.pb.h"
#include "serialization/protobuf/BlockchainTransactionOutput.pb.h"
#include "serialization/protobuf/ContactEnums.pb.h"
#include "util/Container.hpp"

namespace be = boost::endian;

namespace opentxs::factory
{
auto BitcoinTransaction(
    const api::Session& api,
    const blockchain::Type chain,
    const Time& time,
    const boost::endian::little_int32_buf_t& version,
    const boost::endian::little_uint32_buf_t lockTime,
    bool segwit,
    std::unique_ptr<blockchain::block::bitcoin::internal::Inputs> inputs,
    std::unique_ptr<blockchain::block::bitcoin::internal::Outputs>
        outputs) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Transaction>
{
    using ReturnType = blockchain::block::bitcoin::implementation::Transaction;

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
    raw.CalculateIDs(api, chain);

    try {
        return std::make_unique<ReturnType>(
            api,
            ReturnType::default_version_,
            false,
            raw.version_.value(),
            raw.segwit_flag_.value(),
            raw.lock_time_.value(),
            api.Factory().Data(reader(raw.txid_)),
            api.Factory().Data(reader(raw.wtxid_)),
            time,
            UnallocatedCString{},
            std::move(inputs),
            std::move(outputs),
            UnallocatedVector<blockchain::Type>{chain},
            make_blank<blockchain::block::Position>::value(api));
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinTransaction(
    const api::Session& api,
    const blockchain::Type chain,
    const std::size_t position,
    const Time& time,
    blockchain::bitcoin::EncodedTransaction&& parsed) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Transaction>
{
    using ReturnType = blockchain::block::bitcoin::implementation::Transaction;

    try {
        auto inputBytes = std::size_t{};
        auto instantiatedInputs = UnallocatedVector<
            std::unique_ptr<blockchain::block::bitcoin::internal::Input>>{};
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
                        api,
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

        auto outputBytes = std::size_t{};
        auto instantiatedOutputs = UnallocatedVector<
            std::unique_ptr<blockchain::block::bitcoin::internal::Output>>{};
        {
            instantiatedOutputs.reserve(parsed.outputs_.size());
            auto counter = std::uint32_t{0};

            for (const auto& output : parsed.outputs_) {
                instantiatedOutputs.emplace_back(
                    factory::BitcoinTransactionOutput(
                        api,
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
            std::unique_ptr<const blockchain::block::bitcoin::Outputs>{};

        return std::make_unique<ReturnType>(
            api,
            ReturnType::default_version_,
            0 == position,
            parsed.version_.value(),
            parsed.segwit_flag_.value_or(std::byte{0x0}),
            parsed.lock_time_.value(),
            api.Factory().Data(parsed.txid_),
            api.Factory().Data(parsed.wtxid_),
            time,
            UnallocatedCString{},
            factory::BitcoinTransactionInputs(
                std::move(instantiatedInputs), inputBytes),
            factory::BitcoinTransactionOutputs(
                std::move(instantiatedOutputs), outputBytes),
            UnallocatedVector<blockchain::Type>{chain},
            make_blank<blockchain::block::Position>::value(api),
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
    const api::Session& api,
    const proto::BlockchainTransaction& in) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Transaction>
{
    using ReturnType = blockchain::block::bitcoin::implementation::Transaction;
    auto chains = UnallocatedVector<blockchain::Type>{};
    std::transform(
        std::begin(in.chain()),
        std::end(in.chain()),
        std::back_inserter(chains),
        [](const auto type) -> auto {
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
            std::unique_ptr<blockchain::block::bitcoin::internal::Input>>{};

        {
            auto map = UnallocatedMap<
                std::uint32_t,
                std::unique_ptr<blockchain::block::bitcoin::internal::Input>>{};

            for (const auto& input : in.input()) {
                const auto index = input.index();
                map.emplace(
                    index,
                    factory::BitcoinTransactionInput(
                        api,
                        chain,
                        input,
                        (0u == index) && in.is_generation()));
            }

            std::transform(
                std::begin(map), std::end(map), std::back_inserter(inputs), [
                ](auto& in) -> auto { return std::move(in.second); });
        }

        auto outputs = UnallocatedVector<
            std::unique_ptr<blockchain::block::bitcoin::internal::Output>>{};

        {
            auto map = UnallocatedMap<
                std::uint32_t,
                std::unique_ptr<
                    blockchain::block::bitcoin::internal::Output>>{};

            for (const auto& output : in.output()) {
                const auto index = output.index();
                map.emplace(
                    index,
                    factory::BitcoinTransactionOutput(api, chain, output));
            }

            std::transform(
                std::begin(map), std::end(map), std::back_inserter(outputs), [
                ](auto& in) -> auto { return std::move(in.second); });
        }

        return std::make_unique<ReturnType>(
            api,
            in.version(),
            in.is_generation(),
            static_cast<std::int32_t>(in.txversion()),
            std::byte{static_cast<std::uint8_t>(in.segwit_flag())},
            in.locktime(),
            api.Factory().Data(in.txid(), StringStyle::Raw),
            api.Factory().Data(in.wtxid(), StringStyle::Raw),
            Clock::from_time_t(in.time()),
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
                    auto out = api.Factory().Data();

                    if (in.has_mined_block()) { out->Assign(in.mined_block()); }

                    return out;
                }()});
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::block::bitcoin::implementation
{
const VersionNumber Transaction::default_version_{1};

Transaction::Transaction(
    const api::Session& api,
    const VersionNumber serializeVersion,
    const bool isGeneration,
    const std::int32_t version,
    const std::byte segwit,
    const std::uint32_t lockTime,
    const pTxid&& txid,
    const pTxid&& wtxid,
    const Time& time,
    const UnallocatedCString& memo,
    std::unique_ptr<internal::Inputs> inputs,
    std::unique_ptr<internal::Outputs> outputs,
    UnallocatedVector<blockchain::Type>&& chains,
    block::Position&& minedPosition,
    std::optional<std::size_t>&& position) noexcept(false)
    : api_(api)
    , position_(std::move(position))
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
    : api_(rhs.api_)
    , position_(rhs.position_)
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

auto Transaction::AssociatedLocalNyms() const noexcept
    -> UnallocatedVector<OTNymID>
{
    auto output = UnallocatedVector<OTNymID>{};
    inputs_->AssociatedLocalNyms(output);
    outputs_->AssociatedLocalNyms(output);
    dedup(output);

    return output;
}

auto Transaction::AssociatedRemoteContacts(
    const api::session::Contacts& contacts,
    const identifier::Nym& nym) const noexcept
    -> UnallocatedVector<OTIdentifier>
{
    auto output = UnallocatedVector<OTIdentifier>{};
    inputs_->AssociatedRemoteContacts(output);
    outputs_->AssociatedRemoteContacts(output);
    dedup(output);
    const auto mask = contacts.ContactID(nym);
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
               (isSegwit ? calculate_witness_size() : std::size_t{0}) +
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
        std::size_t{2},  // NOTE: marker byte and segwit flag byte
        [=](const std::size_t previous, const auto& input) -> std::size_t {
            return previous + calculate_witness_size(input.Witness());
        });
}

auto Transaction::IDNormalized() const noexcept -> const Identifier&
{
    return cache_.normalized([&] {
        auto preimage = Space{};
        const auto serialized = serialize(writer(preimage), true);

        OT_ASSERT(serialized);

        auto output = api_.Factory().Identifier();
        output->CalculateDigest(
            reader(preimage), identifier::Algorithm::sha256);

        return output;
    });
}

auto Transaction::ExtractElements(const filter::Type style) const noexcept
    -> UnallocatedVector<Space>
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

    if (filter::Type::ES == style) {
        const auto* data = static_cast<const std::byte*>(txid_->data());
        output.emplace_back(data, data + txid_->size());
    }

    LogTrace()(OT_PRETTY_CLASS())("extracted ")(output.size())(
        " total elements")
        .Flush();
    std::sort(output.begin(), output.end());

    return output;
}

auto Transaction::FindMatches(
    const filter::Type style,
    const Patterns& txos,
    const ParsedPatterns& elements) const noexcept -> Matches
{
    LogTrace()(OT_PRETTY_CLASS())("Verifying ")(
        elements.data_.size() + txos.size())(" potential matches in ")(
        inputs_->size())(" inputs for transaction ")(txid_->asHex())
        .Flush();
    auto output = inputs_->FindMatches(txid_->Bytes(), style, txos, elements);
    auto& [inputs, outputs] = output;
    LogTrace()(OT_PRETTY_CLASS())("Verifying ")(
        elements.data_.size() + txos.size())(" potential matches in ")(
        inputs_->size())(" output for transaction ")(txid_->asHex())
        .Flush();
    auto temp = outputs_->FindMatches(txid_->Bytes(), style, elements);
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

auto Transaction::GetPatterns() const noexcept -> UnallocatedVector<PatternID>
{
    auto output = inputs_->GetPatterns();
    const auto oPatterns = outputs_->GetPatterns();
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

auto Transaction::Memo() const noexcept -> UnallocatedCString
{
    if (auto memo = cache_.memo(); false == memo.empty()) { return memo; }

    for (const auto& output : *outputs_) {
        auto note = output.Note();

        if (false == note.empty()) { return note; }
    }

    return {};
}

auto Transaction::MergeMetadata(
    const blockchain::Type chain,
    const internal::Transaction& rhs) noexcept -> void
{
    if (txid_ != rhs.ID()) {
        LogError()(OT_PRETTY_CLASS())("Wrong transaction").Flush();

        return;
    }

    if (false == inputs_->MergeMetadata(rhs.Inputs().Internal())) {
        LogError()(OT_PRETTY_CLASS())("Failed to merge inputs").Flush();

        return;
    }

    if (false == outputs_->MergeMetadata(rhs.Outputs().Internal())) {
        LogError()(OT_PRETTY_CLASS())("Failed to merge outputs").Flush();

        return;
    }

    cache_.merge(rhs);
}

auto Transaction::NetBalanceChange(const identifier::Nym& nym) const noexcept
    -> opentxs::Amount
{
    return inputs_->NetBalanceChange(nym) + outputs_->NetBalanceChange(nym);
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
    auto it = static_cast<std::byte*>(output.data());

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
    remaining -= sizeof(lockTime);

    return size;
}

auto Transaction::Serialize(const AllocateOutput destination) const noexcept
    -> std::optional<std::size_t>
{
    return serialize(destination, false);
}

auto Transaction::Serialize() const noexcept -> std::optional<SerializeType>
{
    auto output = SerializeType{};
    output.set_version(std::max(default_version_, serialize_version_));

    for (const auto chain : cache_.chains()) {
        output.add_chain(translate(UnitToClaim(BlockchainToUnit(chain))));
    }

    output.set_txid(txid_->str());
    output.set_txversion(version_);
    output.set_locktime(lock_time_);

    if (false == Serialize(writer(*output.mutable_serialized())).has_value()) {
        return {};
    }

    if (false == inputs_->Serialize(output)) { return {}; }

    if (false == outputs_->Serialize(output)) { return {}; }

    // TODO optional uint32 confirmations = 9;
    // TODO optional string blockhash = 10;
    // TODO optional uint32 blockindex = 11;
    // TODO optional uint64 fee = 12;
    output.set_time(Clock::to_time_t(time_));
    // TODO repeated string conflicts = 14;
    output.set_memo(cache_.memo());
    output.set_segwit_flag(std::to_integer<std::uint32_t>(segwit_flag_));
    output.set_wtxid(wtxid_->str());
    output.set_is_generation(is_generation_);
    const auto& [height, hash] = cache_.position();

    if ((0 <= height) && (false == hash->empty())) {
        output.set_mined_height(height);
        output.set_mined_block(hash->str());
    }

    return std::move(output);
}

auto Transaction::SetKeyData(const KeyData& data) noexcept -> void
{
    inputs_->SetKeyData(data);
    outputs_->SetKeyData(data);
}

auto Transaction::vBytes(blockchain::Type chain) const noexcept -> std::size_t
{
    const auto& data = params::Data::Chains().at(chain);
    const auto total = CalculateSize();

    if (data.segwit_) {
        const auto& scale = data.segwit_scale_factor_;

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
}  // namespace opentxs::blockchain::block::bitcoin::implementation

// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Internal.hpp"

#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"

#include "api/Core.hpp"
#include "blockchain/bitcoin/CompactSize.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"

#include <boost/endian/buffers.hpp>

#include <algorithm>

#include "Transaction.hpp"

namespace be = boost::endian;

#define OT_METHOD                                                              \
    "opentxs::blockchain::block::bitcoin::implementation::Transaction::"

namespace opentxs
{
using ReturnType = blockchain::block::bitcoin::implementation::Transaction;

auto Factory::BitcoinTransaction(
    const api::internal::Core& api,
    const blockchain::Type chain,
    const bool isGeneration,
    const ReadView txid,
    blockchain::bitcoin::EncodedTransaction&& parsed) noexcept
    -> std::shared_ptr<blockchain::block::bitcoin::Transaction>
{
    try {
        auto inputBytes = std::size_t{};
        auto instantiatedInputs =
            std::vector<std::unique_ptr<blockchain::block::bitcoin::Input>>{};
        {
            auto counter = int{0};
            instantiatedInputs.reserve(parsed.inputs_.size());

            for (const auto& input : parsed.inputs_) {
                const auto& op = input.outpoint_;
                const auto& seq = input.sequence_;
                instantiatedInputs.emplace_back(
                    Factory::BitcoinTransactionInput(
                        ReadView{reinterpret_cast<const char*>(&op),
                                 sizeof(op)},
                        input.cs_,
                        reader(input.script_),
                        ReadView{reinterpret_cast<const char*>(&seq),
                                 sizeof(seq)},
                        isGeneration && (0 == counter)));
                ++counter;
                inputBytes += input.size();
            }

            const auto cs =
                blockchain::bitcoin::CompactSize{parsed.inputs_.size()};
            inputBytes += cs.Size();
            instantiatedInputs.shrink_to_fit();
        }

        auto outputBytes = std::size_t{};
        auto instantiatedOutputs =
            std::vector<std::unique_ptr<blockchain::block::bitcoin::Output>>{};
        {
            instantiatedOutputs.reserve(parsed.outputs_.size());

            for (const auto& output : parsed.outputs_) {
                instantiatedOutputs.emplace_back(
                    Factory::BitcoinTransactionOutput(
                        output.value_.value(),
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
            chain,
            parsed.version_.value(),
            parsed.lock_time_.value(),
            api.Factory().Data(txid),
            opentxs::Factory::BitcoinTransactionInputs(
                std::move(instantiatedInputs), inputBytes),
            opentxs::Factory::BitcoinTransactionOutputs(
                std::move(instantiatedOutputs), outputBytes));
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": ")(e.what()).Flush();

        return {};
    }
}

auto Factory::BitcoinTransaction(
    const api::internal::Core& api,
    const bool isGeneration,
    const proto::BlockchainTransaction& in) noexcept
    -> std::shared_ptr<blockchain::block::bitcoin::Transaction>
{
    try {
        auto inputs =
            std::vector<std::unique_ptr<blockchain::block::bitcoin::Input>>{};

        {
            auto map = std::map<
                std::uint32_t,
                std::unique_ptr<blockchain::block::bitcoin::Input>>{};

            for (const auto& input : in.input()) {
                const auto index = input.index();
                map.emplace(
                    index,
                    Factory::BitcoinTransactionInput(
                        input, (0u == index) && isGeneration));
            }

            std::transform(
                std::begin(map), std::end(map), std::back_inserter(inputs), [
                ](auto& in) -> auto { return std::move(in.second); });
        }

        auto outputs =
            std::vector<std::unique_ptr<blockchain::block::bitcoin::Output>>{};

        {
            auto map = std::map<
                std::uint32_t,
                std::unique_ptr<blockchain::block::bitcoin::Output>>{};

            for (const auto& output : in.output()) {
                const auto index = output.index();
                map.emplace(index, Factory::BitcoinTransactionOutput(output));
            }

            std::transform(
                std::begin(map), std::end(map), std::back_inserter(outputs), [
                ](auto& in) -> auto { return std::move(in.second); });
        }

        return std::make_unique<ReturnType>(
            api,
            in.version(),
            Translate(in.chain()),
            static_cast<std::int32_t>(in.txversion()),
            in.locktime(),
            api.Factory().Data(in.txid(), StringStyle::Raw),
            opentxs::Factory::BitcoinTransactionInputs(std::move(inputs)),
            opentxs::Factory::BitcoinTransactionOutputs(std::move(outputs)));
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs

namespace opentxs::blockchain::block::bitcoin::implementation
{
const VersionNumber Transaction::default_version_{1};

Transaction::Transaction(
    const api::internal::Core& api,
    const VersionNumber serializeVersion,
    const blockchain::Type chain,
    const std::int32_t version,
    const std::uint32_t lockTime,
    const pTxid&& txid,
    std::unique_ptr<const bitcoin::Inputs> inputs,
    std::unique_ptr<const bitcoin::Outputs> outputs) noexcept(false)
    : api_(api)
    , chain_(chain)
    , serialize_version_(serializeVersion)
    , version_(version)
    , lock_time_(lockTime)
    , txid_(std::move(txid))
    , inputs_(std::move(inputs))
    , outputs_(std::move(outputs))
    , normalized_id_()
    , size_()
    , normalized_size_()
{
    if (false == bool(inputs_)) { throw std::runtime_error("invalid inputs"); }

    if (false == bool(outputs_)) {
        throw std::runtime_error("invalid outputs");
    }
}

auto Transaction::calculate_size(const bool normalize) const noexcept
    -> std::size_t
{
    auto& output = normalize ? normalized_size_ : size_;

    if (false == output.has_value()) {
        output = sizeof(version_) + inputs_->CalculateSize(normalize) +
                 outputs_->CalculateSize() + sizeof(lock_time_);
    }

    return output.value();
}

auto Transaction::IDNormalized() const noexcept -> const Identifier&
{
    if (false == normalized_id_.has_value()) {
        auto preimage = Space{};
        const auto serialized = serialize(writer(preimage), true);

        OT_ASSERT(serialized);

        normalized_id_ = api_.Factory().Identifier();
        auto& output = normalized_id_.value().get();
        output.CalculateDigest(reader(preimage), ID::sha256);
    }

    return normalized_id_.value();
}

auto Transaction::serialize(
    const AllocateOutput destination,
    const bool normalize) const noexcept -> std::optional<std::size_t>
{
    if (!destination) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid output allocator")
            .Flush();
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
        return {};
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
    }

    const auto size = calculate_size(normalize);
    auto output = destination(size);

    if (false == output.valid(size)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to allocate output bytes")
            .Flush();
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
        return {};
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
    }

    const auto version = be::little_int32_buf_t{version_};
    const auto lockTime = be::little_uint32_buf_t{lock_time_};
    auto remaining{output.size()};
    auto it = static_cast<std::byte*>(output.data());
    std::memcpy(static_cast<void*>(it), &version, sizeof(version));
    std::advance(it, sizeof(version));
    remaining -= sizeof(version);
    const auto inputs =
        normalize ? inputs_->SerializeNormalized(preallocated(remaining, it))
                  : inputs_->Serialize(preallocated(remaining, it));

    if (inputs.has_value()) {
        std::advance(it, inputs.value());
        remaining -= inputs.value();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to serialize inputs")
            .Flush();
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
        return {};
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
    }

    const auto outputs = outputs_->Serialize(preallocated(remaining, it));

    if (outputs.has_value()) {
        std::advance(it, outputs.value());
        remaining -= outputs.value();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to serialize outputs")
            .Flush();
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
        return {};
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
    }

    std::memcpy(static_cast<void*>(it), &lockTime, sizeof(lockTime));

    return size;
}

auto Transaction::Serialize(const AllocateOutput destination) const noexcept
    -> std::optional<std::size_t>
{
    return serialize(destination, false);
}

auto Transaction::Serialize() const noexcept
    -> std::optional<proto::BlockchainTransaction>
{
    auto output = proto::BlockchainTransaction{};
    output.set_version(std::max(default_version_, serialize_version_));
    output.set_chain(Translate(chain_));
    output.set_txid(txid_->str());
    output.set_txversion(version_);
    output.set_locktime(lock_time_);

    if (false == Serialize(writer(*output.mutable_serialized()))) { return {}; }

    if (false == inputs_->Serialize(output)) { return {}; }

    if (false == outputs_->Serialize(output)) { return {}; }

    // TODO optional uint32 confirmations = 9;
    // TODO optional string blockhash = 10;
    // TODO optional uint32 blockindex = 11;
    // TODO optional uint64 fee = 12;
    // TODO optional int64 time = 13;
    // TODO repeated string conflicts = 14;
    // TODO optional string memo = 15;

    return std::move(output);
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation

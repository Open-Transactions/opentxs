// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/block/bitcoin/Inputs.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <utility>

#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "serialization/protobuf/BlockchainTransaction.pb.h"
#include "util/Container.hpp"

namespace opentxs::factory
{
auto BitcoinTransactionInputs(
    UnallocatedVector<
        std::unique_ptr<blockchain::block::bitcoin::internal::Input>>&& inputs,
    std::optional<std::size_t> size) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Inputs>
{
    using ReturnType = blockchain::block::bitcoin::implementation::Inputs;

    try {

        return std::make_unique<ReturnType>(std::move(inputs), size);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::block::bitcoin::implementation
{
Inputs::Inputs(InputList&& inputs, std::optional<std::size_t> size) noexcept(
    false)
    : inputs_(std::move(inputs))
    , cache_()
{
    for (const auto& input : inputs_) {
        if (false == bool(input)) { throw std::runtime_error("invalid input"); }
    }
}

Inputs::Inputs(const Inputs& rhs) noexcept
    : inputs_(clone(rhs.inputs_))
    , cache_(rhs.cache_)
{
}

auto Inputs::AnyoneCanPay(const std::size_t index) noexcept -> bool
{
    auto& inputs = const_cast<InputList&>(inputs_);

    try {
        auto replace = InputList{};
        replace.emplace_back(inputs.at(index).release());
        inputs.swap(replace);
        cache_.reset_size();

        return true;
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Invalid index").Flush();

        return false;
    }
}

auto Inputs::AssociatedLocalNyms(
    UnallocatedVector<OTNymID>& output) const noexcept -> void
{
    std::for_each(
        std::begin(inputs_), std::end(inputs_), [&](const auto& item) {
            item->AssociatedLocalNyms(output);
        });
}

auto Inputs::AssociatedRemoteContacts(
    UnallocatedVector<OTIdentifier>& output) const noexcept -> void
{
    std::for_each(
        std::begin(inputs_), std::end(inputs_), [&](const auto& item) {
            item->AssociatedRemoteContacts(output);
        });
}

auto Inputs::AssociatePreviousOutput(
    const std::size_t index,
    const internal::Output& output) noexcept -> bool
{
    try {

        return inputs_.at(index)->AssociatePreviousOutput(output);
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Invalid index").Flush();

        return false;
    }
}

auto Inputs::CalculateSize(const bool normalized) const noexcept -> std::size_t
{
    return cache_.size(normalized, [&] {
        const auto cs = blockchain::bitcoin::CompactSize(size());

        return std::accumulate(
            cbegin(),
            cend(),
            cs.Size(),
            [=](const std::size_t& lhs, const auto& rhs) -> std::size_t {
                return lhs + rhs.Internal().CalculateSize(normalized);
            });
    });
}

auto Inputs::clone(const InputList& rhs) noexcept -> InputList
{
    auto output = InputList{};
    std::transform(
        std::begin(rhs),
        std::end(rhs),
        std::back_inserter(output),
        [](const auto& in) { return in->clone(); });

    return output;
}

auto Inputs::ExtractElements(const cfilter::Type style) const noexcept
    -> UnallocatedVector<Space>
{
    auto output = UnallocatedVector<Space>{};
    LogTrace()(OT_PRETTY_CLASS())("processing ")(size())(" inputs").Flush();

    for (const auto& txin : *this) {
        auto temp = txin.Internal().ExtractElements(style);
        output.insert(
            output.end(),
            std::make_move_iterator(temp.begin()),
            std::make_move_iterator(temp.end()));
    }

    LogTrace()(OT_PRETTY_CLASS())("extracted ")(output.size())(" elements")
        .Flush();
    std::sort(output.begin(), output.end());

    return output;
}

auto Inputs::FindMatches(
    const ReadView txid,
    const cfilter::Type type,
    const Patterns& txos,
    const ParsedPatterns& patterns) const noexcept -> Matches
{
    auto output = Matches{};
    auto& [inputs, outputs] = output;
    auto index{-1};

    for (const auto& txin : *this) {
        auto temp = txin.Internal().FindMatches(txid, type, txos, patterns);
        LogTrace()(OT_PRETTY_CLASS())("Verified ")(
            temp.second.size() +
            temp.first.size())(" matches in input ")(++index)
            .Flush();
        inputs.insert(
            inputs.end(),
            std::make_move_iterator(temp.first.begin()),
            std::make_move_iterator(temp.first.end()));
        outputs.insert(
            outputs.end(),
            std::make_move_iterator(temp.second.begin()),
            std::make_move_iterator(temp.second.end()));
    }

    return output;
}

auto Inputs::GetPatterns() const noexcept -> UnallocatedVector<PatternID>
{
    auto output = UnallocatedVector<PatternID>{};
    std::for_each(
        std::begin(inputs_), std::end(inputs_), [&](const auto& txin) {
            const auto patterns = txin->GetPatterns();
            output.insert(output.end(), patterns.begin(), patterns.end());
        });

    dedup(output);

    return output;
}

auto Inputs::Keys() const noexcept -> UnallocatedVector<crypto::Key>
{
    auto out = UnallocatedVector<crypto::Key>{};

    for (const auto& input : *this) {
        auto keys = input.Keys();
        std::move(keys.begin(), keys.end(), std::back_inserter(out));
        dedup(out);
    }

    return out;
}

auto Inputs::MergeMetadata(const internal::Inputs& rhs) noexcept -> bool
{
    const auto count = size();

    if (count != rhs.size()) {
        LogError()(OT_PRETTY_CLASS())("Wrong number of inputs").Flush();

        return false;
    }

    for (auto i = std::size_t{0}; i < count; ++i) {
        auto& l = *inputs_.at(i);
        auto& r = rhs.at(i).Internal();

        if (false == l.MergeMetadata(r)) {
            LogError()(OT_PRETTY_CLASS())("Failed to merge input ")(i).Flush();

            return false;
        }
    }

    return true;
}

auto Inputs::NetBalanceChange(const identifier::Nym& nym) const noexcept
    -> opentxs::Amount
{
    return std::accumulate(
        std::begin(inputs_),
        std::end(inputs_),
        opentxs::Amount{0},
        [&](const auto prev, const auto& input) -> auto {
            return prev + input->NetBalanceChange(nym);
        });
}

auto Inputs::ReplaceScript(const std::size_t index) noexcept -> bool
{
    try {
        cache_.reset_size();

        return inputs_.at(index)->ReplaceScript();
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Invalid index").Flush();

        return false;
    }
}

auto Inputs::serialize(const AllocateOutput destination, const bool normalize)
    const noexcept -> std::optional<std::size_t>
{
    if (!destination) {
        LogError()(OT_PRETTY_CLASS())("Invalid output allocator").Flush();

        return std::nullopt;
    }

    const auto size = CalculateSize(normalize);
    auto output = destination(size);

    if (false == output.valid(size)) {
        LogError()(OT_PRETTY_CLASS())("Failed to allocate output bytes")
            .Flush();

        return std::nullopt;
    }

    auto remaining{output.size()};
    const auto cs = blockchain::bitcoin::CompactSize(this->size()).Encode();
    auto it = static_cast<std::byte*>(output.data());
    std::memcpy(static_cast<void*>(it), cs.data(), cs.size());
    std::advance(it, cs.size());
    remaining -= cs.size();

    for (const auto& row : inputs_) {
        OT_ASSERT(row);

        const auto bytes =
            normalize ? row->SerializeNormalized(preallocated(remaining, it))
                      : row->Serialize(preallocated(remaining, it));

        if (false == bytes.has_value()) {
            LogError()(OT_PRETTY_CLASS())("Failed to serialize input").Flush();

            return std::nullopt;
        }

        std::advance(it, bytes.value());
        remaining -= bytes.value();
    }

    return size;
}

auto Inputs::Serialize(const AllocateOutput destination) const noexcept
    -> std::optional<std::size_t>
{
    return serialize(destination, false);
}

auto Inputs::Serialize(proto::BlockchainTransaction& destination) const noexcept
    -> bool
{
    auto index = std::uint32_t{0};

    for (const auto& input : inputs_) {
        OT_ASSERT(input);

        auto& out = *destination.add_input();

        if (false == input->Serialize(index, out)) { return false; }

        ++index;
    }

    return true;
}

auto Inputs::SerializeNormalized(const AllocateOutput destination)
    const noexcept -> std::optional<std::size_t>
{
    return serialize(destination, true);
}

auto Inputs::SetKeyData(const KeyData& data) noexcept -> void
{
    for (auto& input : inputs_) { input->SetKeyData(data); }
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation

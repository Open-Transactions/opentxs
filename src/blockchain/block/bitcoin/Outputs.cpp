// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "1_Internal.hpp"                        // IWYU pragma: associated
#include "blockchain/block/bitcoin/Outputs.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <utility>

#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "serialization/protobuf/BlockchainTransaction.pb.h"
#include "util/Container.hpp"

namespace opentxs::factory
{
auto BitcoinTransactionOutputs(
    UnallocatedVector<std::unique_ptr<
        blockchain::block::bitcoin::internal::Output>>&& outputs,
    std::optional<std::size_t> size) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Outputs>
{
    using ReturnType = blockchain::block::bitcoin::implementation::Outputs;

    try {

        return std::make_unique<ReturnType>(std::move(outputs), size);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::block::bitcoin::implementation
{
Outputs::Outputs(
    OutputList&& outputs,
    std::optional<std::size_t> size) noexcept(false)
    : outputs_(std::move(outputs))
    , cache_()
{
    for (const auto& output : outputs_) {
        if (false == bool(output)) {
            throw std::runtime_error("invalid output");
        }
    }
}

Outputs::Outputs(const Outputs& rhs) noexcept
    : outputs_(clone(rhs.outputs_))
    , cache_(rhs.cache_)
{
}

auto Outputs::AssociatedLocalNyms(
    UnallocatedVector<OTNymID>& output) const noexcept -> void
{
    std::for_each(
        std::begin(outputs_), std::end(outputs_), [&](const auto& item) {
            item->AssociatedLocalNyms(output);
        });
}

auto Outputs::AssociatedRemoteContacts(
    UnallocatedVector<OTIdentifier>& output) const noexcept -> void
{
    std::for_each(
        std::begin(outputs_), std::end(outputs_), [&](const auto& item) {
            item->AssociatedRemoteContacts(output);
        });
}

auto Outputs::CalculateSize() const noexcept -> std::size_t
{
    return cache_.size([&] {
        const auto cs = blockchain::bitcoin::CompactSize(size());

        return std::accumulate(
            cbegin(),
            cend(),
            cs.Size(),
            [](const std::size_t& lhs, const auto& rhs) -> std::size_t {
                return lhs + rhs.Internal().CalculateSize();
            });
    });
}

auto Outputs::clone(const OutputList& rhs) noexcept -> OutputList
{
    auto output = OutputList{};
    std::transform(
        std::begin(rhs),
        std::end(rhs),
        std::back_inserter(output),
        [](const auto& in) { return in->clone(); });

    return output;
}

auto Outputs::ExtractElements(const filter::Type style) const noexcept
    -> UnallocatedVector<Space>
{
    auto output = UnallocatedVector<Space>{};
    LogTrace()(OT_PRETTY_CLASS())("processing ")(size())(" outputs").Flush();

    for (const auto& txout : *this) {
        auto temp = txout.Internal().ExtractElements(style);
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

auto Outputs::FindMatches(
    const ReadView txid,
    const filter::Type type,
    const ParsedPatterns& patterns) const noexcept -> Matches
{
    auto output = Matches{};
    auto index{-1};

    for (const auto& txout : *this) {
        auto temp = txout.Internal().FindMatches(txid, type, patterns);
        LogTrace()(OT_PRETTY_CLASS())("Verified ")(temp.second.size())(
            " matches in output ")(++index)
            .Flush();
        output.second.insert(
            output.second.end(),
            std::make_move_iterator(temp.second.begin()),
            std::make_move_iterator(temp.second.end()));
    }

    return output;
}

auto Outputs::ForTestingOnlyAddKey(
    const std::size_t index,
    const blockchain::crypto::Key& key) noexcept -> bool
{
    try {
        outputs_.at(index)->ForTestingOnlyAddKey(key);

        return true;
    } catch (...) {

        return false;
    }
}

auto Outputs::GetPatterns() const noexcept -> UnallocatedVector<PatternID>
{
    auto output = UnallocatedVector<PatternID>{};
    std::for_each(
        std::begin(outputs_), std::end(outputs_), [&](const auto& txout) {
            const auto patterns = txout->GetPatterns();
            output.insert(output.end(), patterns.begin(), patterns.end());
        });

    dedup(output);

    return output;
}

auto Outputs::Keys() const noexcept -> UnallocatedVector<crypto::Key>
{
    auto out = UnallocatedVector<crypto::Key>{};

    for (const auto& output : *this) {
        auto keys = output.Keys();
        std::move(keys.begin(), keys.end(), std::back_inserter(out));
        dedup(out);
    }

    return out;
}

auto Outputs::MergeMetadata(const internal::Outputs& rhs) noexcept -> bool
{
    const auto count = size();

    if (count != rhs.size()) {
        LogError()(OT_PRETTY_CLASS())("Wrong number of outputs").Flush();

        return false;
    }

    for (auto i = std::size_t{0}; i < count; ++i) {
        auto& l = *outputs_.at(i);
        auto& r = rhs.at(i).Internal();

        if (false == l.MergeMetadata(r)) {
            LogError()(OT_PRETTY_CLASS())("Failed to merge output ")(i).Flush();

            return false;
        }
    }

    return true;
}

auto Outputs::NetBalanceChange(const identifier::Nym& nym) const noexcept
    -> opentxs::Amount
{
    return std::accumulate(
        std::begin(outputs_),
        std::end(outputs_),
        opentxs::Amount{0},
        [&](const auto prev, const auto& output) -> auto {
            return prev + output->NetBalanceChange(nym);
        });
}

auto Outputs::Serialize(const AllocateOutput destination) const noexcept
    -> std::optional<std::size_t>
{
    if (!destination) {
        LogError()(OT_PRETTY_CLASS())("Invalid output allocator").Flush();

        return std::nullopt;
    }

    const auto size = CalculateSize();
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

    for (const auto& row : outputs_) {
        OT_ASSERT(row);

        const auto bytes = row->Serialize(preallocated(remaining, it));

        if (false == bytes.has_value()) {
            LogError()(OT_PRETTY_CLASS())("Failed to serialize script").Flush();

            return std::nullopt;
        }

        std::advance(it, bytes.value());
        remaining -= bytes.value();
    }

    return size;
}

auto Outputs::Serialize(
    proto::BlockchainTransaction& destination) const noexcept -> bool
{
    for (const auto& output : outputs_) {
        OT_ASSERT(output);

        auto& out = *destination.add_output();

        if (false == output->Serialize(out)) { return false; }
    }

    return true;
}

auto Outputs::SetKeyData(const KeyData& data) noexcept -> void
{
    for (auto& output : outputs_) { output->SetKeyData(data); }
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation

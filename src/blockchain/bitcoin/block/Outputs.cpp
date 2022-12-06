// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type

#include "blockchain/bitcoin/block/Outputs.hpp"  // IWYU pragma: associated

#include <BlockchainTransaction.pb.h>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Output.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/core/ByteArray.hpp"  // IWYU pragma: keep
#include "opentxs/core/Data.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Container.hpp"

namespace opentxs::factory
{
auto BitcoinTransactionOutputs(
    UnallocatedVector<std::unique_ptr<
        blockchain::bitcoin::block::internal::Output>>&& outputs,
    std::optional<std::size_t> size) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Outputs>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Outputs;

    try {

        return std::make_unique<ReturnType>(std::move(outputs), size);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::bitcoin::block::implementation
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
    const api::crypto::Blockchain& crypto,
    UnallocatedVector<identifier::Nym>& output) const noexcept -> void
{
    std::for_each(
        std::begin(outputs_), std::end(outputs_), [&](const auto& item) {
            item->AssociatedLocalNyms(crypto, output);
        });
}

auto Outputs::AssociatedRemoteContacts(
    const api::session::Client& api,
    UnallocatedVector<identifier::Generic>& output) const noexcept -> void
{
    std::for_each(
        std::begin(outputs_), std::end(outputs_), [&](const auto& item) {
            item->AssociatedRemoteContacts(api, output);
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

auto Outputs::ExtractElements(const cfilter::Type style, Elements& out)
    const noexcept -> void
{
    LogTrace()(OT_PRETTY_CLASS())("processing ")(size())(" outputs").Flush();

    for (const auto& txout : *this) {
        txout.Internal().ExtractElements(style, out);
    }
}

auto Outputs::FindMatches(
    const api::Session& api,
    const Txid& txid,
    const cfilter::Type type,
    const ParsedPatterns& patterns,
    const Log& log,
    Matches& out,
    alloc::Default monotonic) const noexcept -> void
{
    for (const auto& txout : *this) {
        txout.Internal().FindMatches(
            api, txid, type, patterns, log, out, monotonic);
    }
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

auto Outputs::IndexElements(const api::Session& api, ElementHashes& out)
    const noexcept -> void
{
    std::for_each(
        std::begin(outputs_), std::end(outputs_), [&](const auto& txout) {
            txout->IndexElements(api, out);
        });
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

auto Outputs::MergeMetadata(
    const internal::Outputs& rhs,
    const Log& log) noexcept -> bool
{
    const auto count = size();

    if (count != rhs.size()) {
        LogError()(OT_PRETTY_CLASS())("Wrong number of outputs").Flush();

        return false;
    }

    for (auto i = 0_uz; i < count; ++i) {
        auto& l = *outputs_.at(i);
        const auto& r = rhs.at(i).Internal();

        if (false == l.MergeMetadata(r, log)) {
            LogError()(OT_PRETTY_CLASS())("Failed to merge output ")(i).Flush();

            return false;
        }
    }

    return true;
}

auto Outputs::NetBalanceChange(
    const api::crypto::Blockchain& crypto,
    const identifier::Nym& nym,
    const Log& log) const noexcept -> opentxs::Amount
{
    return std::accumulate(
        std::begin(outputs_),
        std::end(outputs_),
        opentxs::Amount{0},
        [&](const auto prev, const auto& output) -> auto{
            return prev + output->NetBalanceChange(crypto, nym, log);
        });
}

auto Outputs::Serialize(Writer&& destination) const noexcept
    -> std::optional<std::size_t>
{
    try {
        const auto size = CalculateSize();
        auto buf = reserve(std::move(destination), size, "outputs");
        serialize_compact_size(this->size(), buf, "output count");

        for (const auto& row : outputs_) {
            OT_ASSERT(row);

            const auto& output = *row;
            const auto expected = output.CalculateSize();
            const auto wrote = output.Serialize(buf.Write(expected));

            if ((false == wrote.has_value()) || (*wrote != expected)) {

                throw std::runtime_error{"failed to serialize output"};
            }
        }

        check_finished(buf);

        return size;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return std::nullopt;
    }
}

auto Outputs::Serialize(
    const api::Session& api,
    proto::BlockchainTransaction& destination) const noexcept -> bool
{
    for (const auto& output : outputs_) {
        OT_ASSERT(output);

        auto& out = *destination.add_output();

        if (false == output->Serialize(api, out)) { return false; }
    }

    return true;
}

auto Outputs::SetKeyData(const KeyData& data) noexcept -> void
{
    for (const auto& output : outputs_) { output->SetKeyData(data); }
}
}  // namespace opentxs::blockchain::bitcoin::block::implementation

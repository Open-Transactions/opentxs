// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "blockchain/bitcoin/block/Inputs.hpp"  // IWYU pragma: associated

#include <BlockchainTransaction.pb.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
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
auto BitcoinTransactionInputs(
    UnallocatedVector<
        std::unique_ptr<blockchain::bitcoin::block::internal::Input>>&& inputs,
    std::optional<std::size_t> size) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Inputs>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Inputs;

    try {

        return std::make_unique<ReturnType>(std::move(inputs), size);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::bitcoin::block::implementation
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
    const api::crypto::Blockchain& crypto,
    UnallocatedVector<identifier::Nym>& output) const noexcept -> void
{
    std::for_each(
        std::begin(inputs_), std::end(inputs_), [&](const auto& item) {
            item->AssociatedLocalNyms(crypto, output);
        });
}

auto Inputs::AssociatedRemoteContacts(
    const api::session::Client& api,
    UnallocatedVector<identifier::Generic>& output) const noexcept -> void
{
    std::for_each(
        std::begin(inputs_), std::end(inputs_), [&](const auto& item) {
            item->AssociatedRemoteContacts(api, output);
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

auto Inputs::ExtractElements(const cfilter::Type style, Elements& out)
    const noexcept -> void
{
    LogTrace()(OT_PRETTY_CLASS())("processing ")(size())(" inputs").Flush();

    for (const auto& txin : *this) {
        txin.Internal().ExtractElements(style, out);
    }
}

auto Inputs::FindMatches(
    const api::Session& api,
    const Txid& txid,
    const cfilter::Type type,
    const Patterns& txos,
    const ParsedPatterns& patterns,
    const Log& log,
    Matches& out,
    alloc::Default monotonic) const noexcept -> void
{
    auto index = 0_uz;

    for (const auto& txin : *this) {
        txin.Internal().FindMatches(
            api, txid, type, txos, patterns, index, log, out, monotonic);
        ++index;
    }
}

auto Inputs::IndexElements(const api::Session& api, ElementHashes& out)
    const noexcept -> void
{
    std::for_each(
        std::begin(inputs_), std::end(inputs_), [&](const auto& txin) {
            txin->IndexElements(api, out);
        });
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

auto Inputs::MergeMetadata(const internal::Inputs& rhs, const Log& log) noexcept
    -> bool
{
    const auto count = size();

    if (count != rhs.size()) {
        LogError()(OT_PRETTY_CLASS())("Wrong number of inputs").Flush();

        return false;
    }

    for (auto i = 0_uz; i < count; ++i) {
        auto& l = *inputs_.at(i);
        const auto& r = rhs.at(i).Internal();

        if (false == l.MergeMetadata(r, i, log)) {
            LogError()(OT_PRETTY_CLASS())("Failed to merge input ")(i).Flush();

            return false;
        }
    }

    return true;
}

auto Inputs::NetBalanceChange(
    const api::crypto::Blockchain& crypto,
    const identifier::Nym& nym,
    const Log& log) const noexcept -> opentxs::Amount
{
    auto index = 0_uz;

    return std::accumulate(
        std::begin(inputs_),
        std::end(inputs_),
        opentxs::Amount{0},
        [&](const auto prev, const auto& input) -> auto{
            return prev + input->NetBalanceChange(crypto, nym, index++, log);
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

auto Inputs::serialize(Writer&& destination, const bool normalize)
    const noexcept -> std::optional<std::size_t>
{
    try {
        const auto size = CalculateSize(normalize);
        auto buf = reserve(std::move(destination), size, "inputs");
        serialize_compact_size(this->size(), buf, "input count");

        for (const auto& row : inputs_) {
            OT_ASSERT(row);

            const auto& input = *row;
            const auto expected = input.CalculateSize(normalize);
            const auto wrote =
                normalize ? input.SerializeNormalized(buf.Write(expected))
                          : input.Serialize(buf.Write(expected));

            if ((false == wrote.has_value()) || (*wrote != expected)) {

                throw std::runtime_error{"failed to serialize input"};
            }
        }

        check_finished(buf);

        return size;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return std::nullopt;
    }
}

auto Inputs::Serialize(Writer&& destination) const noexcept
    -> std::optional<std::size_t>
{
    return serialize(std::move(destination), false);
}

auto Inputs::Serialize(
    const api::Session& api,
    proto::BlockchainTransaction& destination) const noexcept -> bool
{
    auto index = std::uint32_t{0};

    for (const auto& input : inputs_) {
        OT_ASSERT(input);

        auto& out = *destination.add_input();

        if (false == input->Serialize(api, index, out)) { return false; }

        ++index;
    }

    return true;
}

auto Inputs::SerializeNormalized(Writer&& destination) const noexcept
    -> std::optional<std::size_t>
{
    return serialize(std::move(destination), true);
}

auto Inputs::SetKeyData(const KeyData& data) noexcept -> void
{
    for (const auto& input : inputs_) { input->SetKeyData(data); }
}
}  // namespace opentxs::blockchain::bitcoin::block::implementation

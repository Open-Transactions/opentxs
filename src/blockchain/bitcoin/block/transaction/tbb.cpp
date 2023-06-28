// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/block/transaction/Imp.hpp"  // IWYU pragma: associated

#include <iterator>

#include "TBB.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/bitcoin/block/Input.hpp"
#include "internal/blockchain/bitcoin/block/Output.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::bitcoin::block::implementation
{
auto Transaction::calculate_input_sizes(const bool normalize) const noexcept
    -> std::size_t
{
    using Range = tbb::blocked_range<const block::Input*>;
    const auto& data = inputs_;

    return tbb::parallel_reduce(
        Range{data.data(), std::next(data.data(), data.size())},
        0_uz,
        [normalize](const Range& r, std::size_t init) {
            for (const auto& i : r) {
                init += i.Internal().CalculateSize(normalize);
            }

            return init;
        },
        [](std::size_t lhs, std::size_t rhs) { return lhs + rhs; });
}

auto Transaction::calculate_output_sizes() const noexcept -> std::size_t
{
    using Range = tbb::blocked_range<const block::Output*>;
    const auto& data = outputs_;

    return tbb::parallel_reduce(
        Range{data.data(), std::next(data.data(), data.size())},
        0_uz,
        [](const Range& r, std::size_t init) {
            for (const auto& i : r) { init += i.Internal().CalculateSize(); }

            return init;
        },
        [](std::size_t lhs, std::size_t rhs) { return lhs + rhs; });
}

auto Transaction::calculate_witness_sizes() const noexcept -> std::size_t
{
    using Range = tbb::blocked_range<const block::Input*>;
    const auto& data = inputs_;

    return tbb::parallel_reduce(
        Range{data.data(), std::next(data.data(), data.size())},
        0_uz,
        [](const Range& r, std::size_t init) {
            for (const auto& i : r) {
                init += calculate_witness_size(i.Witness());
            }

            return init;
        },
        [](std::size_t lhs, std::size_t rhs) { return lhs + rhs; });
}

auto Transaction::calculate_witness_sizes(
    std::span<const WitnessItem> in) noexcept -> std::size_t
{
    using Range = tbb::blocked_range<std::size_t>;

    return tbb::parallel_reduce(
        Range{0_uz, in.size()},
        0_uz,
        [in](const Range& r, std::size_t init) {
            for (auto i = r.begin(); i != r.end(); ++i) {
                const auto& witness = in[i];
                init += calculate_witness_size(witness);
            }

            return init;
        },
        [](std::size_t lhs, std::size_t rhs) { return lhs + rhs; });
}

auto Transaction::merge_metadata(
    const api::crypto::Blockchain& crypto,
    std::span<const block::Input> rhs,
    const Log& log) noexcept -> void
{
    tbb::parallel_for(
        tbb::blocked_range<std::size_t>{0_uz, inputs_.size()},
        [&, this](const auto& r) {
            for (auto i = r.begin(); i != r.end(); ++i) {
                auto& lTx = inputs_[i].Internal();
                const auto& rTx = rhs[i].Internal();
                lTx.MergeMetadata(crypto.Internal().API(), rTx, i, log);
            }
        });
}

auto Transaction::merge_metadata(
    const api::crypto::Blockchain& crypto,
    std::span<const block::Output> rhs,
    const Log& log) noexcept -> void
{
    tbb::parallel_for(
        tbb::blocked_range<std::size_t>{0_uz, outputs_.size()},
        [&, this](const auto& r) {
            for (auto i = r.begin(); i != r.end(); ++i) {
                auto& lTx = outputs_[i].Internal();
                const auto& rTx = rhs[i].Internal();
                lTx.MergeMetadata(crypto.Internal().API(), rTx, log);
            }
        });
}
}  // namespace opentxs::blockchain::bitcoin::block::implementation

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/block/transaction/Imp.hpp"  // IWYU pragma: associated

#include <algorithm>  // IWYU pragma: keep
#include <execution>
#include <numeric>
#include <ranges>

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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-member-function"
    struct Visitor {
        bool normalize_;

        auto operator()(std::size_t lhs, const block::Input& rhs) const noexcept
            -> std::size_t
        {
            return lhs + rhs.Internal().CalculateSize(normalize_);
        }
        auto operator()(const block::Input& lhs, std::size_t rhs) const noexcept
            -> std::size_t
        {
            return lhs.Internal().CalculateSize(normalize_) + rhs;
        }
        auto operator()(const block::Input& lhs, const block::Input& rhs)
            const noexcept -> std::size_t
        {
            return lhs.Internal().CalculateSize(normalize_) +
                   rhs.Internal().CalculateSize(normalize_);
        }
        auto operator()(std::size_t lhs, std::size_t rhs) const noexcept
            -> std::size_t
        {
            return lhs + rhs;
        }
    };
#pragma GCC diagnostic pop
    using namespace std::execution;

    return std::reduce(
        par, inputs_.begin(), inputs_.end(), 0_uz, Visitor{normalize});
}

auto Transaction::calculate_output_sizes() const noexcept -> std::size_t
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-member-function"
    struct Visitor {
        auto operator()(std::size_t lhs, const block::Output& rhs)
            const noexcept -> std::size_t
        {
            return lhs + rhs.Internal().CalculateSize();
        }
        auto operator()(const block::Output& lhs, std::size_t rhs)
            const noexcept -> std::size_t
        {
            return lhs.Internal().CalculateSize() + rhs;
        }
        auto operator()(const block::Output& lhs, const block::Output& rhs)
            const noexcept -> std::size_t
        {
            return lhs.Internal().CalculateSize() +
                   rhs.Internal().CalculateSize();
        }
        auto operator()(std::size_t lhs, std::size_t rhs) const noexcept
            -> std::size_t
        {
            return lhs + rhs;
        }
    };
#pragma GCC diagnostic pop
    using namespace std::execution;

    return std::reduce(par, outputs_.begin(), outputs_.end(), 0_uz, Visitor{});
}

auto Transaction::calculate_witness_sizes() const noexcept -> std::size_t
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-member-function"
    struct Visitor {
        auto operator()(std::size_t lhs, const block::Input& rhs) const noexcept
            -> std::size_t
        {
            return lhs + calculate_witness_size(rhs.Witness());
        }
        auto operator()(const block::Input& lhs, std::size_t rhs) const noexcept
            -> std::size_t
        {
            return calculate_witness_size(lhs.Witness()) + rhs;
        }
        auto operator()(const block::Input& lhs, const block::Input& rhs)
            const noexcept -> std::size_t
        {
            return calculate_witness_size(lhs.Witness()) +
                   calculate_witness_size(rhs.Witness());
        }
        auto operator()(std::size_t lhs, std::size_t rhs) const noexcept
            -> std::size_t
        {
            return lhs + rhs;
        }
    };
#pragma GCC diagnostic pop
    using namespace std::execution;

    return std::reduce(par, inputs_.begin(), inputs_.end(), 0_uz, Visitor{});
}

auto Transaction::calculate_witness_sizes(
    std::span<const WitnessItem> in) noexcept -> std::size_t
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-member-function"
    struct Visitor {
        auto operator()(std::size_t lhs, const WitnessItem& rhs) const noexcept
            -> std::size_t
        {
            return lhs + calculate_witness_size(rhs);
        }
        auto operator()(const WitnessItem& lhs, std::size_t rhs) const noexcept
            -> std::size_t
        {
            return calculate_witness_size(lhs) + rhs;
        }
        auto operator()(const WitnessItem& lhs, const WitnessItem& rhs)
            const noexcept -> std::size_t
        {
            return calculate_witness_size(lhs) + calculate_witness_size(rhs);
        }
        auto operator()(std::size_t lhs, std::size_t rhs) const noexcept
            -> std::size_t
        {
            return lhs + rhs;
        }
    };
#pragma GCC diagnostic pop
    using namespace std::execution;

    return std::reduce(par, in.begin(), in.end(), 0_uz, Visitor{});
}

auto Transaction::merge_metadata(
    const api::crypto::Blockchain& crypto,
    std::span<const block::Input> rhs,
    const Log& log) noexcept -> void
{
    auto merge = [&, this](auto n) {
        inputs_[n].Internal().MergeMetadata(
            crypto.Internal().API(), rhs[n].Internal(), n, log);
    };
    const auto range = std::views::iota(0_uz, inputs_.size());
    using namespace std::execution;
    std::for_each(par, range.begin(), range.end(), merge);
}

auto Transaction::merge_metadata(
    const api::crypto::Blockchain& crypto,
    std::span<const block::Output> rhs,
    const Log& log) noexcept -> void
{
    auto merge = [&, this](auto n) {
        outputs_[n].Internal().MergeMetadata(
            crypto.Internal().API(), rhs[n].Internal(), log);
    };
    const auto range = std::views::iota(0_uz, outputs_.size());
    using namespace std::execution;
    std::for_each(par, range.begin(), range.end(), merge);
}
}  // namespace opentxs::blockchain::bitcoin::block::implementation

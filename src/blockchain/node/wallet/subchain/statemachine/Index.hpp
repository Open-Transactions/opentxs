// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <optional>
#include <queue>
#include <tuple>
#include <utility>

#include "blockchain/node/wallet/subchain/statemachine/Job.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace crypto
{
class Element;
}  // namespace crypto

namespace node
{
namespace internal
{
struct WalletDatabase;
}  // namespace internal

namespace wallet
{
class Progress;
class Rescan;
class Scan;
class SubchainStateData;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Index : public Job
{
public:
    auto Processed(const ProgressBatch& processed) noexcept -> void;
    auto Reorg(const block::Position& parent) noexcept -> void final;
    auto Run() noexcept -> bool final;

    Index(
        SubchainStateData& parent,
        Scan& scan,
        Rescan& rescan,
        Progress& progress) noexcept;

    ~Index() override = default;

protected:
    Scan& scan_;
    Rescan& rescan_;
    Progress& progress_;
    std::optional<Bip32Index> last_indexed_;

    auto done(
        const block::Position& original,
        const node::internal::WalletDatabase::ElementMap& elements) noexcept
        -> void;
    auto index_element(
        const filter::Type type,
        const blockchain::crypto::Element& input,
        const Bip32Index index,
        node::internal::WalletDatabase::ElementMap& output) const noexcept
        -> void;
    // NOTE this function does not lock the mutex. Use carefully.
    auto ready_for_scan() noexcept -> void;

private:
    static constexpr auto lookback_ = block::Height{240};

    bool first_;
    std::queue<std::pair<block::Position, std::size_t>> queue_;

    auto rescan(
        const Lock& lock,
        const block::Position& pos,
        const std::size_t matches) noexcept -> void;
    auto type() const noexcept -> const char* final { return "index"; }

    virtual auto Do(
        std::optional<Bip32Index> current,
        Bip32Index target) noexcept -> void = 0;
    // NOTE always called when no other threads are running
    virtual auto need_index(const std::optional<Bip32Index>& current)
        const noexcept -> std::optional<Bip32Index> = 0;

    Index() = delete;
    Index(const Index&) = delete;
    Index(Index&&) = delete;
    auto operator=(const Index&) -> Index& = delete;
    auto operator=(Index&&) -> Index& = delete;
};
}  // namespace opentxs::blockchain::node::wallet

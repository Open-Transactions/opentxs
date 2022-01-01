// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Bytes.hpp"
#include "serialization/protobuf/BlockchainTransactionOutput.pb.h"

namespace opentxs
{
namespace proto
{
class BlockchainTransaction;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::blockchain::block::bitcoin::implementation
{
class Outputs final : public internal::Outputs
{
public:
    using OutputList = std::pmr::vector<std::unique_ptr<internal::Output>>;

    auto AssociatedLocalNyms(std::pmr::vector<OTNymID>& output) const noexcept
        -> void final;
    auto AssociatedRemoteContacts(
        std::pmr::vector<OTIdentifier>& output) const noexcept -> void final;
    auto at(const std::size_t position) const noexcept(false)
        -> const value_type& final
    {
        return *outputs_.at(position);
    }
    auto begin() const noexcept -> const_iterator final { return cbegin(); }
    auto CalculateSize() const noexcept -> std::size_t final;
    auto cbegin() const noexcept -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto cend() const noexcept -> const_iterator final
    {
        return const_iterator(this, outputs_.size());
    }
    auto clone() const noexcept -> std::unique_ptr<internal::Outputs> final
    {
        return std::make_unique<Outputs>(*this);
    }
    auto end() const noexcept -> const_iterator final { return cend(); }
    auto ExtractElements(const filter::Type style) const noexcept
        -> std::pmr::vector<Space> final;
    auto FindMatches(
        const ReadView txid,
        const filter::Type type,
        const ParsedPatterns& elements) const noexcept -> Matches final;
    auto GetPatterns() const noexcept -> std::pmr::vector<PatternID> final;
    auto Keys() const noexcept -> std::pmr::vector<crypto::Key> final;
    auto Internal() const noexcept -> const internal::Outputs& final
    {
        return *this;
    }
    auto NetBalanceChange(const identifier::Nym& nym) const noexcept
        -> opentxs::Amount final;
    auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> final;
    auto Serialize(proto::BlockchainTransaction& destination) const noexcept
        -> bool final;
    auto SetKeyData(const KeyData& data) noexcept -> void final;
    auto size() const noexcept -> std::size_t final { return outputs_.size(); }

    auto at(const std::size_t position) noexcept(false) -> value_type& final
    {
        return *outputs_.at(position);
    }
    auto ForTestingOnlyAddKey(
        const std::size_t index,
        const blockchain::crypto::Key& key) noexcept -> bool final;
    auto Internal() noexcept -> internal::Outputs& final { return *this; }
    auto MergeMetadata(const internal::Outputs& rhs) noexcept -> bool final;

    Outputs(
        OutputList&& outputs,
        std::optional<std::size_t> size = {}) noexcept(false);
    Outputs(const Outputs&) noexcept;

    ~Outputs() final = default;

private:
    struct Cache {
        auto reset_size() noexcept -> void
        {
            auto lock = Lock{lock_};
            size_ = std::nullopt;
        }
        template <typename F>
        auto size(F cb) noexcept -> std::size_t
        {
            auto lock = Lock{lock_};

            auto& output = size_;

            if (false == output.has_value()) { output = cb(); }

            return output.value();
        }

        Cache() noexcept = default;
        Cache(const Cache& rhs) noexcept
            : lock_()
            , size_()
        {
            auto lock = Lock{rhs.lock_};
            size_ = rhs.size_;
        }

    private:
        mutable std::mutex lock_{};
        std::optional<std::size_t> size_{};
    };

    const OutputList outputs_;
    mutable Cache cache_;

    static auto clone(const OutputList& rhs) noexcept -> OutputList;

    Outputs() = delete;
    Outputs(Outputs&&) = delete;
    auto operator=(const Outputs&) -> Outputs& = delete;
    auto operator=(Outputs&&) -> Outputs& = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation

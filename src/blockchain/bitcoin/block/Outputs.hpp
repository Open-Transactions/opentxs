// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type

#pragma once

#include <BlockchainTransactionOutput.pb.h>
#include <cstddef>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>

#include "internal/blockchain/bitcoin/block/Output.hpp"
#include "internal/blockchain/bitcoin/block/Outputs.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/blockchain/bitcoin/block/Outputs.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

namespace session
{
class Client;
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

namespace proto
{
class BlockchainTransaction;
}  // namespace proto

class Log;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block::implementation
{
class Outputs final : public internal::Outputs
{
public:
    using OutputList = UnallocatedVector<std::unique_ptr<internal::Output>>;

    auto AssociatedLocalNyms(
        const api::crypto::Blockchain& crypto,
        UnallocatedVector<identifier::Nym>& output) const noexcept
        -> void final;
    auto AssociatedRemoteContacts(
        const api::session::Client& api,
        UnallocatedVector<identifier::Generic>& output) const noexcept
        -> void final;
    auto at(const std::size_t position) const noexcept(false)
        -> const value_type& final
    {
        return *outputs_.at(position);
    }
    auto begin() const noexcept -> const_iterator final { return cbegin(); }
    auto CalculateSize() const noexcept -> std::size_t final;
    auto cbegin() const noexcept -> const_iterator final { return {this, 0}; }
    auto cend() const noexcept -> const_iterator final
    {
        return {this, outputs_.size()};
    }
    auto clone() const noexcept -> std::unique_ptr<internal::Outputs> final
    {
        return std::make_unique<Outputs>(*this);
    }
    auto end() const noexcept -> const_iterator final { return cend(); }
    auto ExtractElements(const cfilter::Type style, Elements& out)
        const noexcept -> void final;
    auto FindMatches(
        const api::Session& api,
        const Txid& txid,
        const cfilter::Type type,
        const ParsedPatterns& elements,
        const Log& log,
        Matches& out,
        alloc::Default monotonic) const noexcept -> void final;
    auto IndexElements(const api::Session& api, ElementHashes& out)
        const noexcept -> void final;
    auto Keys() const noexcept -> UnallocatedVector<crypto::Key> final;
    auto NetBalanceChange(
        const api::crypto::Blockchain& crypto,
        const identifier::Nym& nym,
        const Log& log) const noexcept -> opentxs::Amount final;
    auto Serialize(Writer&& destination) const noexcept
        -> std::optional<std::size_t> final;
    auto Serialize(
        const api::Session& api,
        proto::BlockchainTransaction& destination) const noexcept -> bool final;
    auto SetKeyData(const KeyData& data) noexcept -> void final;
    auto size() const noexcept -> std::size_t final { return outputs_.size(); }

    auto at(const std::size_t position) noexcept(false) -> value_type& final
    {
        return *outputs_.at(position);
    }
    auto ForTestingOnlyAddKey(
        const std::size_t index,
        const blockchain::crypto::Key& key) noexcept -> bool final;
    auto MergeMetadata(const internal::Outputs& rhs, const Log& log) noexcept
        -> bool final;

    Outputs(
        OutputList&& outputs,
        std::optional<std::size_t> size = {}) noexcept(false);
    Outputs() = delete;
    Outputs(const Outputs&) noexcept;
    Outputs(Outputs&&) = delete;
    auto operator=(const Outputs&) -> Outputs& = delete;
    auto operator=(Outputs&&) -> Outputs& = delete;

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
};
}  // namespace opentxs::blockchain::bitcoin::block::implementation

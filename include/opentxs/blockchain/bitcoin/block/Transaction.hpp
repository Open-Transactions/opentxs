// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <optional>
#include <tuple>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

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

namespace blockchain
{
namespace bitcoin
{
namespace block
{
namespace internal
{
class Transaction;
}  // namespace internal

class Inputs;
class Outputs;
}  // namespace block
}  // namespace bitcoin
}  // namespace blockchain

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

namespace proto
{
class BlockchainTransaction;
class BlockchainTransactionOutput;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block
{
class OPENTXS_EXPORT Transaction
{
public:
    virtual auto AssociatedLocalNyms(const api::crypto::Blockchain& crypto)
        const noexcept -> UnallocatedVector<identifier::Nym> = 0;
    virtual auto AssociatedRemoteContacts(
        const api::session::Client& api,
        const identifier::Nym& nym) const noexcept
        -> UnallocatedVector<identifier::Generic> = 0;
    virtual auto BlockPosition() const noexcept
        -> std::optional<std::size_t> = 0;
    virtual auto Chains() const noexcept
        -> UnallocatedVector<blockchain::Type> = 0;
    virtual auto clone() const noexcept -> std::unique_ptr<Transaction> = 0;
    virtual auto ID() const noexcept -> const blockchain::block::Txid& = 0;
    virtual auto Inputs() const noexcept -> const block::Inputs& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Transaction& = 0;
    virtual auto IsGeneration() const noexcept -> bool = 0;
    virtual auto Keys() const noexcept -> UnallocatedVector<crypto::Key> = 0;
    virtual auto Locktime() const noexcept -> std::uint32_t = 0;
    virtual auto Memo(const api::crypto::Blockchain& crypto) const noexcept
        -> UnallocatedCString = 0;
    virtual auto NetBalanceChange(
        const api::crypto::Blockchain& crypto,
        const identifier::Nym& nym) const noexcept -> opentxs::Amount = 0;
    virtual auto Outputs() const noexcept -> const block::Outputs& = 0;
    virtual auto Print() const noexcept -> UnallocatedCString = 0;
    virtual auto SegwitFlag() const noexcept -> std::byte = 0;
    virtual auto Timestamp() const noexcept -> Time = 0;
    virtual auto Version() const noexcept -> std::int32_t = 0;
    virtual auto vBytes(blockchain::Type chain) const noexcept
        -> std::size_t = 0;
    virtual auto WTXID() const noexcept -> const blockchain::block::Txid& = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::Transaction& = 0;

    Transaction(const Transaction&) = delete;
    Transaction(Transaction&&) = delete;
    auto operator=(const Transaction&) -> Transaction& = delete;
    auto operator=(Transaction&&) -> Transaction& = delete;

    virtual ~Transaction() = default;

protected:
    Transaction() noexcept = default;
};
}  // namespace opentxs::blockchain::bitcoin::block

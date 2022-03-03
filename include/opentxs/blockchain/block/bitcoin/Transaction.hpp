// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <optional>
#include <tuple>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Contacts;
}  // namespace session
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
namespace internal
{
struct Transaction;
}  // namespace internal

class Inputs;
class Outputs;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

namespace proto
{
class BlockchainTransaction;
class BlockchainTransactionOutput;
}  // namespace proto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block::bitcoin
{
class OPENTXS_EXPORT Transaction
{
public:
    virtual auto AssociatedLocalNyms() const noexcept
        -> UnallocatedVector<OTNymID> = 0;
    virtual auto AssociatedRemoteContacts(
        const api::session::Contacts& contacts,
        const identifier::Nym& nym) const noexcept
        -> UnallocatedVector<OTIdentifier> = 0;
    virtual auto BlockPosition() const noexcept
        -> std::optional<std::size_t> = 0;
    virtual auto Chains() const noexcept
        -> UnallocatedVector<blockchain::Type> = 0;
    virtual auto clone() const noexcept -> std::unique_ptr<Transaction> = 0;
    virtual auto ID() const noexcept -> const Txid& = 0;
    virtual auto Inputs() const noexcept -> const bitcoin::Inputs& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Transaction& = 0;
    virtual auto IsGeneration() const noexcept -> bool = 0;
    virtual auto Keys() const noexcept -> UnallocatedVector<crypto::Key> = 0;
    virtual auto Locktime() const noexcept -> std::uint32_t = 0;
    virtual auto Memo() const noexcept -> UnallocatedCString = 0;
    virtual auto NetBalanceChange(const identifier::Nym& nym) const noexcept
        -> opentxs::Amount = 0;
    virtual auto Outputs() const noexcept -> const bitcoin::Outputs& = 0;
    virtual auto Print() const noexcept -> UnallocatedCString = 0;
    virtual auto SegwitFlag() const noexcept -> std::byte = 0;
    virtual auto Timestamp() const noexcept -> Time = 0;
    virtual auto Version() const noexcept -> std::int32_t = 0;
    virtual auto vBytes(blockchain::Type chain) const noexcept
        -> std::size_t = 0;
    virtual auto WTXID() const noexcept -> const Txid& = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::Transaction& = 0;

    virtual ~Transaction() = default;

protected:
    Transaction() noexcept = default;

private:
    Transaction(const Transaction&) = delete;
    Transaction(Transaction&&) = delete;
    auto operator=(const Transaction&) -> Transaction& = delete;
    auto operator=(Transaction&&) -> Transaction& = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin

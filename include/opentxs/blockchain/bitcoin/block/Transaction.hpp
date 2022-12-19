// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Input;
class Output;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class TransactionHash;
class TransactionPrivate;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block
{
class OPENTXS_EXPORT Transaction : virtual public blockchain::block::Transaction
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Transaction&;

    auto Inputs() const noexcept -> std::span<const block::Input>;
    auto IsGeneration() const noexcept -> bool;
    auto Locktime() const noexcept -> std::uint32_t;
    auto Outputs() const noexcept -> std::span<const block::Output>;
    auto SegwitFlag() const noexcept -> std::byte;
    auto Timestamp() const noexcept -> Time;
    auto TXID() const noexcept -> const TransactionHash& { return ID(); }
    auto Version() const noexcept -> std::int32_t;
    auto vBytes(blockchain::Type chain) const noexcept -> std::size_t;
    auto WTXID() const noexcept -> const TransactionHash& { return Hash(); }

    OPENTXS_NO_EXPORT Transaction(
        blockchain::block::TransactionPrivate* imp) noexcept;
    Transaction(allocator_type alloc = {}) noexcept;
    Transaction(const Transaction& rhs, allocator_type alloc = {}) noexcept;
    Transaction(Transaction&& rhs) noexcept;
    Transaction(Transaction&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Transaction& rhs) noexcept -> Transaction&;
    auto operator=(Transaction&& rhs) noexcept -> Transaction&;

    ~Transaction() override;
};
}  // namespace opentxs::blockchain::bitcoin::block

// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>

#include "blockchain/block/transaction/TransactionPrivate.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"

namespace opentxs::blockchain::block::implementation
{
class Transaction : virtual public TransactionPrivate
{
public:
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> TransactionPrivate* override;
    auto Hash() const noexcept -> const TransactionHash& final { return hash_; }
    auto ID() const noexcept -> const TransactionHash& final { return id_; }

    [[nodiscard]] auto get_deleter() noexcept -> std::function<void()> override;

    Transaction() = delete;
    Transaction(const Transaction& rhs, allocator_type alloc) noexcept;
    Transaction(const Transaction&) = delete;
    Transaction(Transaction&&) = delete;
    auto operator=(const Transaction&) -> Transaction& = delete;
    auto operator=(Transaction&&) -> Transaction& = delete;

    ~Transaction() override;

protected:
    const TransactionHash id_;
    const TransactionHash hash_;

    Transaction(
        TransactionHash id,
        TransactionHash hash,
        allocator_type alloc) noexcept;
};
}  // namespace opentxs::blockchain::block::implementation

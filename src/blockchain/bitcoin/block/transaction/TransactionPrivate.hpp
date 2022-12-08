// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "blockchain/block/transaction/TransactionPrivate.hpp"

#include <functional>

#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"

namespace opentxs::blockchain::bitcoin::block
{
class TransactionPrivate : virtual public blockchain::block::TransactionPrivate,
                           virtual public internal::Transaction
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> TransactionPrivate*;

    auto asBitcoinPrivate() const noexcept
        -> const bitcoin::block::TransactionPrivate* final
    {
        return this;
    }
    auto asBitcoinPublic() const noexcept
        -> const bitcoin::block::Transaction& final
    {
        return self_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> blockchain::block::TransactionPrivate* override;

    auto asBitcoinPrivate() noexcept
        -> bitcoin::block::TransactionPrivate* final
    {
        return this;
    }
    auto asBitcoinPublic() noexcept -> bitcoin::block::Transaction& final
    {
        return self_;
    }
    [[nodiscard]] auto get_deleter() noexcept -> std::function<void()> override;

    TransactionPrivate(allocator_type alloc) noexcept;
    TransactionPrivate() = delete;
    TransactionPrivate(
        const TransactionPrivate& rhs,
        allocator_type alloc) noexcept;
    TransactionPrivate(const TransactionPrivate&) = delete;
    TransactionPrivate(TransactionPrivate&&) = delete;
    auto operator=(const TransactionPrivate&) -> TransactionPrivate& = delete;
    auto operator=(TransactionPrivate&&) -> TransactionPrivate& = delete;

    ~TransactionPrivate() override;

private:
    block::Transaction self_;
};
}  // namespace opentxs::blockchain::bitcoin::block

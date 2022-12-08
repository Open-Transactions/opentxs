// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>

#include "internal/blockchain/block/Transaction.hpp"
#include "util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Transaction;
class TransactionPrivate;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Transaction;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block
{
class TransactionPrivate : virtual public internal::Transaction,
                           public opentxs::implementation::Allocated
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> TransactionPrivate*;
    static auto Reset(block::Transaction& tx) noexcept -> void;

    virtual auto asBitcoinPrivate() const noexcept
        -> const bitcoin::block::TransactionPrivate*;
    virtual auto asBitcoinPublic() const noexcept
        -> const bitcoin::block::Transaction&;
    [[nodiscard]] virtual auto clone(allocator_type alloc) const noexcept
        -> TransactionPrivate*;

    virtual auto asBitcoinPrivate() noexcept
        -> bitcoin::block::TransactionPrivate*;
    virtual auto asBitcoinPublic() noexcept -> bitcoin::block::Transaction&;
    [[nodiscard]] virtual auto get_deleter() noexcept -> std::function<void()>;

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
};
}  // namespace opentxs::blockchain::block

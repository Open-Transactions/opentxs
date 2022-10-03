// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <optional>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace storage
{
namespace lmdb
{
class DatabasePrivate;
class TransactionPrivate;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::lmdb
{
class Transaction
{
public:
    auto Success() const noexcept -> bool;

    auto Finalize(const std::optional<bool> success = std::nullopt) noexcept
        -> bool;
    auto SetSuccess(bool value) noexcept -> void;

    Transaction(const Transaction&) = delete;
    Transaction(Transaction&& rhs) noexcept;
    auto operator=(const Transaction&) -> Transaction& = delete;
    auto operator=(Transaction&&) -> Transaction& = delete;

    ~Transaction();

private:
    friend DatabasePrivate;
    friend TransactionPrivate;

    std::unique_ptr<TransactionPrivate> imp_;

    Transaction() noexcept;
    Transaction(std::unique_ptr<TransactionPrivate> imp) noexcept;
};
}  // namespace opentxs::storage::lmdb

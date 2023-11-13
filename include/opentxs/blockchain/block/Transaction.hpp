// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <typeindex>
// IWYU pragma: no_include <variant>

#pragma once

#include <compare>
#include <cstddef>
#include <optional>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Allocated.hpp"
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

class Crypto;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace internal
{
class Transaction;
}  // namespace internal

class Transaction;
class TransactionHash;
class TransactionPrivate;
}  // namespace block

namespace protocol
{
namespace bitcoin
{
namespace base
{
namespace block
{
class Transaction;
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::blockchain::block::Transaction> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::blockchain::block::Transaction& data)
        const noexcept -> std::size_t;
};
}  // namespace std

namespace opentxs::blockchain::block
{
OPENTXS_EXPORT auto operator==(const Transaction&, const Transaction&) noexcept
    -> bool;
OPENTXS_EXPORT auto operator<=>(const Transaction&, const Transaction&) noexcept
    -> std::strong_ordering;
OPENTXS_EXPORT auto swap(Transaction&, Transaction&) noexcept -> void;
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::block
{
class OPENTXS_EXPORT Transaction : virtual public opentxs::Allocated
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Transaction&;

    [[nodiscard]] operator bool() const noexcept { return IsValid(); }

    auto asBitcoin() const& noexcept
        -> const protocol::bitcoin::base::block::Transaction&;
    auto AssociatedLocalNyms(
        const api::crypto::Blockchain& crypto,
        allocator_type alloc) const noexcept -> Set<identifier::Nym>;
    auto AssociatedRemoteContacts(
        const api::session::Client& api,
        const identifier::Nym& nym,
        allocator_type alloc) const noexcept -> Set<identifier::Generic>;
    auto BlockPosition() const noexcept -> std::optional<std::size_t>;
    auto Chains(allocator_type alloc) const noexcept -> Set<blockchain::Type>;
    auto get_allocator() const noexcept -> allocator_type final;
    auto Hash() const noexcept -> const TransactionHash&;
    auto ID() const noexcept -> const TransactionHash&;
    OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::Transaction&;
    [[nodiscard]] auto IsValid() const noexcept -> bool;
    auto Keys(allocator_type alloc) const noexcept -> Set<crypto::Key>;
    auto Memo(const api::crypto::Blockchain& crypto) const noexcept
        -> UnallocatedCString;
    auto Memo(const api::crypto::Blockchain& crypto, allocator_type alloc)
        const noexcept -> CString;
    auto NetBalanceChange(
        const api::crypto::Blockchain& crypto,
        const identifier::Nym& nym) const noexcept -> opentxs::Amount;
    auto Print(const api::Crypto& crypto) const noexcept -> UnallocatedCString;
    auto Print(const api::Crypto& crypto, allocator_type alloc) const noexcept
        -> CString;

    auto asBitcoin() & noexcept -> protocol::bitcoin::base::block::Transaction&;
    auto asBitcoin() && noexcept -> protocol::bitcoin::base::block::Transaction;
    auto get_deleter() noexcept -> delete_function final;
    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Transaction&;
    auto swap(Transaction& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Transaction(TransactionPrivate* imp) noexcept;
    Transaction(allocator_type alloc = {}) noexcept;
    Transaction(const Transaction& rhs, allocator_type alloc = {}) noexcept;
    Transaction(Transaction&& rhs) noexcept;
    Transaction(Transaction&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Transaction& rhs) noexcept -> Transaction&;
    auto operator=(Transaction&& rhs) noexcept -> Transaction&;

    ~Transaction() override;

protected:
    friend TransactionPrivate;

    TransactionPrivate* imp_;
};
}  // namespace opentxs::blockchain::block

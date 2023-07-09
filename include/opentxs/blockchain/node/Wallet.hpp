// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <tuple>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Output;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Outpoint;
}  // namespace block

namespace node
{
namespace internal
{
class Wallet;
}  // namespace internal
}  // namespace node
}  // namespace blockchain

namespace identifier
{
class Account;
class Generic;
class Nym;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node
{
class OPENTXS_EXPORT Wallet
{
public:
    virtual auto GetBalance() const noexcept -> Balance = 0;
    virtual auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance = 0;
    virtual auto GetBalance(
        const identifier::Nym& owner,
        const identifier::Account& subaccount) const noexcept -> Balance = 0;
    virtual auto GetBalance(const crypto::Key& key) const noexcept
        -> Balance = 0;
    virtual auto GetOutputs(alloc::Strategy alloc = {}) const noexcept
        -> Vector<UTXO> = 0;
    virtual auto GetOutputs(TxoState type, alloc::Strategy alloc = {})
        const noexcept -> Vector<UTXO> = 0;
    virtual auto GetOutputs(
        const identifier::Nym& owner,
        alloc::Strategy alloc = {}) const noexcept -> Vector<UTXO> = 0;
    virtual auto GetOutputs(
        const identifier::Nym& owner,
        TxoState type,
        alloc::Strategy alloc = {}) const noexcept -> Vector<UTXO> = 0;
    virtual auto GetOutputs(
        const identifier::Nym& owner,
        const identifier::Account& subaccount,
        alloc::Strategy alloc = {}) const noexcept -> Vector<UTXO> = 0;
    virtual auto GetOutputs(
        const identifier::Nym& owner,
        const identifier::Account& subaccount,
        TxoState type,
        alloc::Strategy alloc = {}) const noexcept -> Vector<UTXO> = 0;
    virtual auto GetOutputs(
        const crypto::Key& key,
        TxoState type,
        alloc::Strategy alloc = {}) const noexcept -> Vector<UTXO> = 0;
    virtual auto GetTags(const block::Outpoint& output) const noexcept
        -> UnallocatedSet<TxoTag> = 0;
    virtual auto Height() const noexcept -> block::Height = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Wallet& = 0;
    virtual auto StartRescan() const noexcept -> bool = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::Wallet& = 0;

    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;

    OPENTXS_NO_EXPORT virtual ~Wallet() = default;

protected:
    Wallet() noexcept = default;
};
}  // namespace opentxs::blockchain::node
